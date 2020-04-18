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
};

struct destination_info{
    cfg_host*                  info;
    std::vector<connection*>   connections;
};

class Forwarder {
public:
    Forwarder(Config* config);
    ~Forwarder();

//    void reconnectorStart();
//    void reconnectorStop();

    void processMsg(ipx_msg_t *msg, const fds_iemgr_t *iemgr);

    void processIPFIX(ipx_msg_ipfix_t *msg_ipfix);
    void processSession(ipx_msg_session_t *msg_session);

    void processTemplateSet(fds_ipfix_set_hdr *tset);
    void processDataSet(ipx_msg_ipfix_t *msg, fds_ipfix_set_hdr *dset);

    void forward(ipx_msg_t *msg);

    void sendAll(ipx_msg_t *msg);

    SEND_STATUS packetSender(fwd_bldr_t *builder, struct connection *conn, bool req_flag);






private:
    ipx_ctx_t*      ctx;
    Config*         config;
    fwd_bldr_t*     builder_all;
    fwd_bldr_t*     builder_tmplt;
    fwd_sender_t*   sender;

    std::vector<destination_info*>   destinations;
    std::vector<connection*>         idle_connections;
    std::atomic_flag                 destinations_lock;

    // Reconnection thread, that tries to reconnect disconnected
    void reconnector();
    std::thread             reconnection_thread;
    std::mutex              reconnection_thread_mutex;
    std::condition_variable reconnection_thread_cv;


};


#endif //IPFIXCOL2_FORWARDER_H
