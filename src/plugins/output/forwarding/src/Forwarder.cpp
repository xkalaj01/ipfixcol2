//
// Created by jan on 16.4.20.
//

#include <iostream>
#include <bits/unique_ptr.h>
#include <mutex>
#include "Forwarder.h"
#include "../../../../core/message_base.h"
#include "../../../../core/message_ipfix.h"

Forwarder::Forwarder(Config* config) {

    this->config = config;
    builder_all = bldr_create();
    builder_tmplt = bldr_create();

    // Link destinations with info
    for (auto& info : config->hosts){
        std::unique_ptr<destination_info> dest_info(new destination_info);

        dest_info->info = &info;
        destinations.push_back(dest_info.release());
    }

    // Create reconnection thread
    reconnection_thread = std::thread(&Forwarder::reconnector, this);
}

Forwarder::~Forwarder() {
    // Terminate reconnection thread
    reconnection_thread_cv.notify_all();

    if (reconnection_thread.joinable()){
        reconnection_thread.join();
    }
}

void Forwarder::reconnector() {
    std::unique_lock<std::mutex> lock(reconnection_thread_mutex);

    std::chrono::seconds sec(config->options.check_rate);

    // Main loop of thread
    while (reconnection_thread_cv.wait_for(lock, sec) == std::cv_status::timeout){

        while(destinations_lock.test_and_set(std::memory_order_acquire));
        printf("Trying to reconnect ");
        // Iterate over all idle sessions and try to reconnect them all
        for ( auto conn_it = idle_connections.begin(); conn_it != idle_connections.end(); conn_it++){
            connection *conn = *conn_it;
            int succ = sender_connect(conn->sender);
            if (succ == 0){
                conn->status = STATUS_OK;
                idle_connections.erase(conn_it--);
                printf("-Reconnection successfull!\n");
            }

        }

        destinations_lock.clear(std::memory_order_release);
    }
}

void Forwarder::processMsg(ipx_msg_t *msg, const fds_iemgr_t *iemgr) {

    ipx_msg_type msg_type = ipx_msg_get_type(msg);


    if (msg_type == IPX_MSG_IPFIX){
        ipx_msg_ipfix_t *msg_ipfix = ipx_msg_base2ipfix(msg);
        processIPFIX(msg_ipfix);
    } else if (msg_type == IPX_MSG_SESSION){
        ipx_msg_session_t *msg_session = ipx_msg_base2session(msg);
        processSession(msg_session);
    }

}

void Forwarder::processSession(ipx_msg_session_t *msg_session) {

    ipx_msg_session_event event = ipx_msg_session_get_event(msg_session);
    const struct ipx_session * session = ipx_msg_session_get_session(msg_session);

    if (event == IPX_MSG_SESSION_OPEN){
        // new exporter connection -> add connection to each destination
        for (auto dest_it = destinations.begin(); dest_it != destinations.end(); dest_it++){
            destination_info *dest = *dest_it;
            cfg_host *host = dest->info;
            connection *conn = new connection;

            conn->input_session = session;
            conn->sender = sender_create(host->addr.c_str(), host->port.c_str(), config->options.proto);
            dest->connections.push_back(conn);
            printf("Adding connection\n");

            // Try to establish connection
            int conn_succ = sender_connect(conn->sender);
            if (conn_succ == 0 ){
                conn->status = STATUS_OK;
                printf("-connection active\n");

            } else {
                conn->status = STATUS_CLOSED;
                idle_connections.push_back(conn);
                printf("-connection idle\n");

            }
        }
    } else if (event == IPX_MSG_SESSION_CLOSE){
        // exporter disconnects -> remove senders for each destination
        for (auto dest_it = destinations.begin(); dest_it != destinations.end(); dest_it++){
            destination_info *dest = *dest_it;

            for (auto conn_it = dest->connections.begin(); conn_it != dest->connections.end(); conn_it++){
                connection * conn = *conn_it;

                if (conn->input_session == session){
                    sender_destroy(conn->sender);
                    conn->input_session = NULL;
                    dest->connections.erase(conn_it--);
                    printf("Removing connection\n");

                }
            }
        }
    }
}

void Forwarder::processIPFIX(ipx_msg_ipfix_t *msg_ipfix) {

    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr= (const struct fds_ipfix_msg_hdr *)ipx_msg_ipfix_get_packet(msg_ipfix);

    uint32_t pkt_odid = ntohl(ipfix_msg_hdr->odid);
    uint32_t pkt_exp_time = ntohl(ipfix_msg_hdr->export_time);
    uint8_t *msg_end = (uint8_t *)ipfix_msg_hdr + ntohs(ipfix_msg_hdr->length);

    bldr_start(builder_all, pkt_odid, pkt_exp_time);
    bldr_start(builder_tmplt, pkt_odid, pkt_exp_time);

    bool any_templates = false;

    // Get number of sets
    struct ipx_ipfix_set *sets;
    size_t set_cnt;
    ipx_msg_ipfix_get_sets(msg_ipfix, &sets, &set_cnt);

    uint16_t flowset_id;
    uint16_t set_len;
    fds_ipfix_set_hdr *set_header;

    // Iteration through all the sets
    for (uint32_t i = 0; i < set_cnt; ++i){
        // Get header of the set
        set_header = sets[i].ptr;
        set_len = ntohs(set_header->length);

        // Check a length of the set
        uint8_t *set_end = (uint8_t *)set_header + set_len;
        if(set_end > msg_end){
            throw std::runtime_error("Malformed IPFIX packet!");
        }

        // Get the type of the set and add it to the builder
        flowset_id = ntohs(set_header->flowset_id);
        if (flowset_id == FDS_TYPE_TEMPLATE || flowset_id == FDS_TYPE_TEMPLATE_OPTS){
            // Process template set;
            any_templates = true;
            processTemplateSet(set_header);
        } else {
            // Process data set;
            processDataSet(msg_ipfix, set_header);
        }

    }

    bldr_end(builder_all, config->options.mtu_size);
    bldr_end(builder_tmplt, config->options.mtu_size);
}

void Forwarder::processTemplateSet(fds_ipfix_set_hdr *tset) {

    uint16_t tset_length = ntohs(tset->length);
    uint16_t tset_id = ntohs(tset->flowset_id);
    enum fds_template_type type;
    void *ptr;
    struct fds_tset_iter tset_iter;
    fds_tset_iter_init(&tset_iter, tset);

    switch (tset_id){
        case FDS_IPFIX_SET_TMPLT:
            type = FDS_TYPE_TEMPLATE;
            ptr = tset_iter.ptr.trec;
            break;
        case FDS_IPFIX_SET_OPTS_TMPLT:
            type = FDS_TYPE_TEMPLATE_OPTS;
            ptr = tset_iter.ptr.opts_trec;
            break;
        default:
            return;
    }

    // Iteration through all templates in the set
    while (fds_tset_iter_next(&tset_iter) == FDS_OK){

        // Filling the template structure with data from raw packet
        uint16_t tmplt_size = tset_iter.size;
        struct fds_template *tmplt;
        if (fds_template_parse(type, ptr, &tmplt_size, &tmplt) != FDS_OK){
            throw std::runtime_error("Template parsing error");
        }

        bldr_add_template(builder_all, (uint8_t*)tset, tset_length, tmplt->id, tset_id);
        bldr_add_template(builder_tmplt, (uint8_t*)tset, tset_length, tmplt->id, tset_id);

    }
}

void Forwarder::processDataSet(ipx_msg_ipfix_t *msg, fds_ipfix_set_hdr *dset) {

    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);
    uint16_t tset_id = ntohs(dset->flowset_id);

    const struct fds_ipfix_dset *data_set;
    data_set = (const struct fds_ipfix_dset *) dset;

    bldr_add_dataset(builder_all, data_set, tset_id, rec_cnt);
}

void Forwarder::forward(ipx_msg_t *msg) {

    switch (config->options.oper_mode){
        case cfg_options::SEND_MODE_ALL:
            sendAll(msg);
            break;
        case cfg_options::SEND_MODE_ROUND_ROBIN:
            //todo
            break;
        default:
            throw std::runtime_error("Unknown operational mode!");
    }
}


void Forwarder::sendAll(ipx_msg_t *msg) {
    // First we get input session to identify output connection
    ipx_msg_ipfix_t *msg_ipfix = ipx_msg_base2ipfix(msg);
    ipx_msg_ctx * msg_ctx = ipx_msg_ipfix_get_ctx(msg_ipfix);

    for(auto dest_it = destinations.begin(); dest_it!=destinations.end(); dest_it++){
        destination_info *dest = *dest_it;

        // find corresponding output session to the input session
        for (auto conn_it = dest->connections.begin(); conn_it!=dest->connections.end(); conn_it++){
            connection *conn = *conn_it;

            if ((conn->input_session == msg_ctx->session) && (conn->status != STATUS_CLOSED)){
                // Found -> send!
                printf("Sending\n");
                conn->status = packetSender(builder_all, conn, true);


                if (conn->status == STATUS_CLOSED){
                    printf("CLOSED\n");
                    idle_connections.push_back(conn);
                } else{
                    printf("SENT\n");

                }
                break;
            }
        }
    }

}

SEND_STATUS Forwarder::packetSender(fwd_bldr_t *builder, struct connection *conn, bool req_flag) {
    int pkt_cnt = bldr_pkts_cnt(builder);
    enum SEND_STATUS stat;

    struct iovec *pkt_parts;
    size_t size;
    size_t rec_cnt;

    uint32_t odid = bldr_pkts_get_odid(builder);
    uint32_t seq_num = 424242;

    for (int i = 0; i < pkt_cnt; i++){
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

        seq_num += rec_cnt;
        req_flag = true; // Remaining packets are always required
    }
    return STATUS_OK;

}
