//
// Created by jan on 16.4.20.
//

#ifndef IPFIXCOL2_FORWARDER_H
#define IPFIXCOL2_FORWARDER_H

#include <thread>
#include <condition_variable>
#include "Config.hpp"
#include "packet.h"
#include "sender.h"

struct connection_info{
    cfg_host*       info;
    fwd_sender_t*   conn;
};

class Forwarder {
public:
    Forwarder(Config* config);
    ~Forwarder();

    

private:
    ipx_ctx_t*      ctx;
    Config*         config;
    fwd_bldr_t*     pkt_builder;
    fwd_sender_t*   sender;

    std::vector<connection_info*>   connections_active;
    std::vector<connection_info*>   connections_idle;
    std::atomic_flag                connections_queue_lock;

    void connection_checking();
    std::thread             checking_thread;
    std::mutex              checking_thread_mutex;
    std::condition_variable checking_thread_cv;


};


#endif //IPFIXCOL2_FORWARDER_H
