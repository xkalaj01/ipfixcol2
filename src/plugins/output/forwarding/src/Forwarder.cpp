/**
 * \file src/plugins/output/forwarding/src/Forwarder.cpp
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Forwarding output (source file)
 * \date 2020
 */

/* Copyright (C) 2020 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <iostream>
#include <bits/unique_ptr.h>
#include <mutex>
#include "Forwarder.h"


Forwarder::Forwarder(ipx_ctx_t* ctx, Config* config)
        : ctx(ctx), config(config)
{
    builder = bldr_create();

    // Link destinations with info
    for (auto& host : config->hosts){

        auto *dest_info = new destination_info;

        dest_info->info = &host;

        // Link with previous destination
        if (!destinations.empty()){
            destinations.back()->next_destination = dest_info;
        }

        destinations.push_back(dest_info);

        IPX_CTX_INFO(ctx,"Destination added - [%s:%s] %s",
                     host.addr.c_str(), host.port.c_str(),
                     host.hostname.c_str());
    }

    //Link last element with first (create circle)
    if (!destinations.empty()){
        destinations.back()->next_destination = destinations.front();
    }

    // Set the pointer to first destination for Round Robin
    rr_last_destination = destinations.front();

    // Create reconnection thread
    reconnection_thread = std::thread(&Forwarder::reconnector, this);
}

Forwarder::~Forwarder() {
    IPX_CTX_INFO(ctx, "Module shutting down",'\0');

    // Terminate reconnection thread
    reconnection_thread_cv.notify_all();
    if (reconnection_thread.joinable()){
        reconnection_thread.join();
    }

    // Clear the destination table
    for (auto dest : destinations){
        for (auto conn: dest->connections){
            sender_destroy(conn->sender);
            conn->odid2seqnum_map.clear();
            delete conn;
        }
        dest->connections.clear();
        delete dest;
    }

    bldr_destroy(builder);
}

void Forwarder::reconnector() {
    std::unique_lock<std::mutex> lock(reconnection_thread_mutex);

    std::chrono::seconds sec(config->options.check_rate);

    // Main loop of thread
    while (reconnection_thread_cv.wait_for(lock, sec) == std::cv_status::timeout){

        while(destinations_lock.test_and_set(std::memory_order_acquire));
        // Iterate over all idle sessions and try to reconnect them all
        for ( auto conn_it = idle_connections.begin(); conn_it != idle_connections.end(); conn_it++){
            connection *conn = *conn_it;
            int succ = sender_connect(conn->sender);
            if (succ == 0){
                // Change the socket status to OK for both TCP and UDP
                conn->status = STATUS_OK;
                idle_connections.erase(conn_it--);
            }

        }

        destinations_lock.clear(std::memory_order_release);
    }
}

void Forwarder::processMsg(ipx_msg_t *msg) {

    ipx_msg_type msg_type = ipx_msg_get_type(msg);

    while(destinations_lock.test_and_set(std::memory_order_acquire));

    // Determine what message is being parsed
    if (msg_type == IPX_MSG_IPFIX){
        ipx_msg_ipfix_t *msg_ipfix = ipx_msg_base2ipfix(msg);
        processIPFIX(msg_ipfix);
    } else if (msg_type == IPX_MSG_SESSION){
        ipx_msg_session_t *msg_session = ipx_msg_base2session(msg);
        processSession(msg_session);
    }

    destinations_lock.clear(std::memory_order_release);
}

void Forwarder::processSession(ipx_msg_session_t *msg_session) {

    ipx_msg_session_event event = ipx_msg_session_get_event(msg_session);
    const struct ipx_session * session = ipx_msg_session_get_session(msg_session);

    if (event == IPX_MSG_SESSION_OPEN){
        // new exporter connection -> add connection to each destination
        for (auto dest : destinations) {
            cfg_host *host = dest->info;
            auto *conn = new connection;
            std::string conn_stat_str;

            conn->input_session = session;
            conn->sender = sender_create(host->addr.c_str(), host->port.c_str(), config->options.proto);
            conn->tmplt_snapshot = nullptr;
            dest->connections.push_back(conn);


            // Try to establish connection
            int conn_succ = sender_connect(conn->sender);
            if (conn_succ == 0 ){
                conn->status = STATUS_OK;
                conn_stat_str = "ESTABLISHED";

            } else {
                conn->status = STATUS_CLOSED;
                idle_connections.push_back(conn);
                conn_stat_str = "NOT ESTABLISHED";

            }

            IPX_CTX_INFO(ctx,"Connection added - :%d\t[%s:%s]\t%s",
                         sender_get_port(conn->sender), host->addr.c_str(),
                         host->port.c_str(), conn_stat_str.c_str());
        }
    } else if (event == IPX_MSG_SESSION_CLOSE){
        // exporter disconnects -> remove senders for each destination
        for (auto dest : destinations) {
            for (auto conn_it = dest->connections.begin(); conn_it != dest->connections.end(); conn_it++){
                connection * conn = *conn_it;

                if (conn->input_session == session){
                    IPX_CTX_INFO(ctx,"Connection removed - :%d\t[%s:%s]\t%s",
                                 sender_get_port(conn->sender),
                                 dest->info->addr.c_str(),
                                 dest->info->port.c_str());

                    conn->input_session = nullptr;
                    sender_destroy(conn->sender);
                    dest->connections.erase(conn_it--);
                    delete conn;
                }
            }
        }
    }
}

void Forwarder::processIPFIX(ipx_msg_ipfix_t *msg_ipfix) {

    uint16_t flowset_id;
    uint16_t set_len;
    fds_ipfix_set_hdr *set_header;
    uint32_t rec_i = 0;

    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr= (const struct fds_ipfix_msg_hdr *)ipx_msg_ipfix_get_packet(msg_ipfix);

    // Get the information for builder
    uint32_t pkt_odid = ntohl(ipfix_msg_hdr->odid);
    uint32_t pkt_exp_time = ntohl(ipfix_msg_hdr->export_time);
    uint8_t *msg_end = (uint8_t *)ipfix_msg_hdr + ntohs(ipfix_msg_hdr->length);
    bldr_start(builder, pkt_odid, pkt_exp_time);

    // Get number of sets
    struct ipx_ipfix_set *sets;
    size_t set_cnt;
    ipx_msg_ipfix_get_sets(msg_ipfix, &sets, &set_cnt);

    // Remember information fom packet header for future use
    curr_msg_snapshot = nullptr;
    curr_msg_odid = pkt_odid;

    // Iteration through all the records
    for (uint32_t i = 0; i < set_cnt; ++i){
        // Get header of the set
        set_header = sets[i].ptr;
        set_len = ntohs(set_header->length);

        // Check a length of the set
        uint8_t *set_end = (uint8_t *)set_header + set_len;
        if(set_end > msg_end){
            throw std::runtime_error("Malformed IPFIX packet!");
        }

        flowset_id = ntohs(set_header->flowset_id);
        if (flowset_id >= FDS_IPFIX_SET_MIN_DSET){
            // Process data set;
            processDataSet(msg_ipfix, set_header, set_end, &rec_i);
        }

    }

    bldr_end(builder, config->options.mtu_size);
}

void Forwarder::processDataSet(ipx_msg_ipfix_t *msg, fds_ipfix_set_hdr *dset, uint8_t *set_end, uint32_t *rec_i) {
    uint32_t rec_cnt = 0;
    uint16_t dset_id = ntohs(dset->flowset_id);
    const uint32_t rec_total = ipx_msg_ipfix_get_drec_cnt(msg);
    struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(msg, *rec_i);

    // Get the snapshot pointer from first record
    curr_msg_snapshot = ipx_msg_ipfix_get_drec(msg, 0)->rec.snap;

    // Counting the records which belongs to the current set
    while ((ipfix_rec != nullptr) && (ipfix_rec->rec.data < set_end) && ((*rec_i) < rec_total)) {
        // Get the next record
        (*rec_i)++;
        rec_cnt++;
        ipfix_rec = ipx_msg_ipfix_get_drec(msg, *rec_i);
    }

    // Add dataset to builder
    const struct fds_ipfix_dset *data_set;
    data_set = (const struct fds_ipfix_dset *) dset;

    bldr_add_dataset(builder, data_set, dset_id, rec_cnt);
}

void Forwarder::forward(ipx_msg_ipfix_t *msg_ipfix) {

    while(destinations_lock.test_and_set(std::memory_order_acquire));

    // Perform selected operational mode by user
    switch (config->options.oper_mode){

        case cfg_options::SEND_MODE_ALL:
            forwardAll(msg_ipfix);
            break;

        case cfg_options::SEND_MODE_ROUND_ROBIN:
            forwardRoundRobin(msg_ipfix);
            break;

        default:
            IPX_CTX_ERROR(ctx, "Unknown operational mode!", '\0');
            return;
    }

    destinations_lock.clear(std::memory_order_release);

}

void Forwarder::forwardAll(ipx_msg_ipfix_t *msg_ipfix) {

    for (auto dest : destinations) {
        // find corresponding output session to the input session
        connection *conn = getOutputConnection(dest, msg_ipfix);

        if ((conn != nullptr) && (conn->status != STATUS_CLOSED)){

            // Check if the host has the latest templates
            if (!hasLatestTemplates(conn)){
                updateTemplates(msg_ipfix, conn);
            }

            // Send the data out
            conn->status = packetSender(builder, conn, true);

            switch(conn->status){
                case STATUS_OK:
                    IPX_CTX_DEBUG(ctx, "Message forwarded to -> [%s:%s] %s",
                                 dest->info->addr.c_str(),
                                 dest->info->port.c_str(),
                                 dest->info->hostname.c_str());
                    break;

                case STATUS_CLOSED:
                    IPX_CTX_WARNING(ctx, "Failed to forward message to -> [%s:%s] %s",
                            dest->info->addr.c_str(), dest->info->port.c_str(),
                            dest->info->hostname.c_str());

                    idle_connections.push_back(conn);
                    conn->tmplt_snapshot = nullptr;
                    break;
                default:
                    // do nothing
                    break;
            }
        }
    }
}

void Forwarder::forwardRoundRobin(ipx_msg_ipfix_t *msg_ipfix) {
    assert(rr_last_destination != nullptr);

    // Find the next destination to send packet to
    destination_info *dest_to_send = rr_last_destination->next_destination;
    connection *conn = nullptr;

    // Loop through all the destinations and try to forward the data
    while(true) {

        // find next destination to send data to
        while (true) {
            conn = getOutputConnection(dest_to_send, msg_ipfix);
            if (conn && conn->status != STATUS_CLOSED) {
                // we found it!
                break;
            }

            // Prevent infinite loop
            if (dest_to_send->next_destination == rr_last_destination->next_destination) {
                // we haven't found it :(
                IPX_CTX_ERROR(ctx, "All destinations are unreachable! Dropping message", '\0');
                return;
            }

            dest_to_send = dest_to_send->next_destination;
        }

        // If we found connection for next destination, send him the data
        if (conn != nullptr) {
            // Check for relevant templates
            if (!hasLatestTemplates(conn)) {
                updateTemplates(msg_ipfix, conn);
            }

            // Send the data out
            conn->status = packetSender(builder, conn, true);

            switch (conn->status) {
                case STATUS_OK:
                    IPX_CTX_DEBUG(ctx, "Message forwarded to -> [%s:%s] %s",
                                    dest_to_send->info->addr.c_str(),
                                    dest_to_send->info->port.c_str(),
                                    dest_to_send->info->hostname.c_str());

                    rr_last_destination = dest_to_send;
                    return;

                case STATUS_CLOSED:
                    IPX_CTX_INFO(ctx, "Failed to forward message to -> [%s:%s] %s",
                                    dest_to_send->info->addr.c_str(),
                                    dest_to_send->info->port.c_str(),
                                    dest_to_send->info->hostname.c_str());

                    // Mark as closed and move to next destination
                    idle_connections.push_back(conn);
                    conn->tmplt_snapshot = nullptr;
                    break;

                default:
                    // do nothing
                    break;
            }

        }
    }
}

connection *Forwarder::getOutputConnection(destination_info *dest, ipx_msg_ipfix_t *msg_ipfix){
    // First we get input session to identify output connection
    ipx_msg_ctx * msg_ctx = ipx_msg_ipfix_get_ctx(msg_ipfix);

    // Go through all the connections associated with destination
    for (auto conn : dest->connections) {
        if (conn->input_session == msg_ctx->session){
            return conn;
        }
    }
    return nullptr;
}

SEND_STATUS Forwarder::packetSender(fwd_bldr_t *builder, struct connection *conn, bool req_flag) {
    int pkt_cnt = bldr_pkts_cnt(builder);
    enum SEND_STATUS stat;

    struct iovec *pkt_parts;
    size_t size;
    size_t rec_cnt;
    uint32_t seq_num;

    // find sequence number mapping
    if (conn->odid2seqnum_map.find(curr_msg_odid) == conn->odid2seqnum_map.end()){
        conn->odid2seqnum_map[curr_msg_odid] = 0;
    }

    for (int i = 0; i < pkt_cnt; i++){

        seq_num = conn->odid2seqnum_map[curr_msg_odid];

        // Prepare the packet
        if (bldr_pkts_iovec(builder, seq_num, i, &pkt_parts, &size,
                            &rec_cnt)) {
            // Internal Error
            return STATUS_INVALID;
        }

        // Send the packet
        stat = sender_send_parts(conn->sender, pkt_parts, size,
                                 MODE_NON_BLOCKING, req_flag);

        if (stat != STATUS_OK) {
            return stat;
        }

        conn->odid2seqnum_map[curr_msg_odid] += rec_cnt;
        req_flag = true; // Remaining packets are always required
    }
    return STATUS_OK;

}

/**
 * Callback function for looping over all the templates in snapshot
 *
 * @param tmplt  Current template
 * @param data   Pointer to template builder
 * @return true  Continue iteration
 */
static bool
tmplt_add_cb(const struct fds_template *tmplt, void *data)
{
    auto *builder = (fwd_bldr_t*)data;

    bldr_add_template(builder, tmplt->raw.data, tmplt->raw.length, tmplt->id, tmplt->type);

    return true;
}

SEND_STATUS Forwarder::updateTemplates(ipx_msg_ipfix_t *msg_ipfix, connection *conn) {
    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr= (const struct fds_ipfix_msg_hdr *)ipx_msg_ipfix_get_packet(msg_ipfix);
    SEND_STATUS succ = STATUS_INVALID;

    if (curr_msg_snapshot!= nullptr){
        // Get the information for builder
        uint32_t pkt_odid = ntohl(ipfix_msg_hdr->odid);
        uint32_t pkt_exp_time = ntohl(ipfix_msg_hdr->export_time);

        // Start builder
        fwd_bldr_t *builder_tmplt = bldr_create();
        bldr_start(builder_tmplt, pkt_odid, pkt_exp_time);

        // Iterate over all templates in snapshot and add them to builder
        fds_tsnapshot_for(curr_msg_snapshot,&tmplt_add_cb, (void*)builder_tmplt);

        // End builder and send data
        bldr_end(builder_tmplt, config->options.mtu_size);
        succ = packetSender(builder_tmplt, conn, true);
        bldr_destroy(builder_tmplt);

        // Update the snapshot pointer for connection
        conn->tmplt_snapshot = curr_msg_snapshot;

    }
    return succ;
}

bool Forwarder::hasLatestTemplates(connection *conn) {
    if (conn->tmplt_snapshot != curr_msg_snapshot && curr_msg_snapshot!= nullptr){
        return false;
    }
    return true;
}

