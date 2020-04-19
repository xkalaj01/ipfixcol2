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


struct connection{
    fwd_sender_t*               sender;
    const struct ipx_session*   input_session;
    SEND_STATUS                 status;
    const fds_tsnapshot_t*      tmplt_snapshot;
    uint32_t                    seq_num;
};

struct destination_info{
    cfg_host*                  info;
    std::vector<connection*>   connections;
    destination_info *         next_destination;
};

class Forwarder {
public:
    Forwarder(ipx_ctx_t* ctx, Config* config);
    ~Forwarder();

    ipx_ctx_t*      ctx;

    void processMsg(ipx_msg_t *msg);
    void forward(ipx_msg_ipfix_t *msg_ipfix);

private:
    void processIPFIX(ipx_msg_ipfix_t *msg_ipfix);
    void processSession(ipx_msg_session_t *msg_session);

    void processTemplateSet(fds_ipfix_set_hdr *tset);
    void processDataSet(ipx_msg_ipfix_t *msg, fds_ipfix_set_hdr *dset);

    bool hasLatestTemplates(connection *conn);
    SEND_STATUS updateTemplates(ipx_msg_ipfix_t *msg_ipfix, connection *conn);

    void forwardAll(ipx_msg_ipfix_t *msg_ipfix);
    void forwardRoundRobin(ipx_msg_ipfix_t *msg_ipfix);

    SEND_STATUS packetSender(fwd_bldr_t *builder, struct connection *conn, bool req_flag);

    // Returns corresponding output connection for specified destination and input connection
    connection *getOutputConnection(destination_info *dest, ipx_msg_ipfix_t *msg_ipfix);


private:
    Config*         config;
    fwd_bldr_t*     builder;
    fwd_sender_t*   sender;

    const fds_tsnapshot_t* curr_msg_snapshot;

    std::vector<destination_info*>   destinations;
    std::vector<connection*>         idle_connections;
    std::atomic_flag                 destinations_lock = ATOMIC_FLAG_INIT;

    // Current destination to send data to for RoundRobin mode
    destination_info*                rr_last_destination;

    // Reconnection thread, that tries to reconnect disconnected
    void reconnector();
    std::thread             reconnection_thread;
    std::mutex              reconnection_thread_mutex;
    std::condition_variable reconnection_thread_cv;

};


#endif //IPFIXCOL2_FORWARDER_H
