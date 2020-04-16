//
// Created by jan on 16.4.20.
//

#include <iostream>
#include <bits/unique_ptr.h>
#include <mutex>
#include "Forwarder.h"

Forwarder::Forwarder(Config* config) {
    this->config = config;
    pkt_builder = bldr_create();
    int proto = (config->options.proto == config->options.SEND_PROTO_TCP) ? IPPROTO_TCP: IPPROTO_UDP;

    // Create senders ad link them with info
    for (auto& info : config->hosts){
        std::unique_ptr<connection_info> conn(new connection_info);

        try {
            conn->info = &info;
            conn->conn = sender_create(info.addr.c_str(), info.port.c_str(), proto);
            // add the connection to idle list
            if (conn->conn){
                connections_idle.push_back(conn.release());

            } else {
                throw std::runtime_error("Error while initializing sender for : "+info.hostname);
            }

        } catch (std::runtime_error &ex){
            throw ex;
        }
    }

    // Create checking thread
    checking_thread = std::thread(&Forwarder::connection_checking, this);
}

void Forwarder::connection_checking() {
    std::unique_lock<std::mutex> lock(checking_thread_mutex);

    std::chrono::seconds sec(config->options.check_rate);

    // Main loop of thread
    while (checking_thread_cv.wait_for(lock, sec) == std::cv_status::timeout){

        while(connections_queue_lock.test_and_set(std::memory_order_acquire));

        // Iterate over all idle sessions and try to reconnect them all
        for ( auto it = connections_idle.begin(); it != connections_idle.end(); it++){
            int succ = sender_connect((*it)->conn);
            if (succ == 0){
                connections_active.push_back(*it);
                connections_idle.erase(it--);
            }

        }

        connections_queue_lock.clear(std::memory_order_release);
    }
}
