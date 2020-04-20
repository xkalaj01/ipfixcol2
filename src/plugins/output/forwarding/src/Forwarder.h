/**
 * \file src/plugins/output/forwarding/src/Forwarder.h
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Forwarding output (header file)
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

#ifndef IPFIXCOL2_FORWARDER_H
#define IPFIXCOL2_FORWARDER_H

#include <thread>
#include <condition_variable>
#include <atomic>
#include <ipfixcol2.h>
#include "Config.hpp"
#include "packet.h"
#include "sender.h"

/** Single output connection                                                */
struct connection{
    /** Sender for specific connection                                      */
    fwd_sender_t*                   sender;
    /** Corresponding input session                                         */
    const struct ipx_session*       input_session;
    /** Status of the connection                                            */
    SEND_STATUS                     status;
    /** Pointer to last templates, that has been sent to host               */
    const fds_tsnapshot_t*          tmplt_snapshot;
    /** Sequence number mapping to ODID numbers                             */
    std::map<uint32_t, uint32_t >   odid2seqnum_map;
};

/** Single destination information                                          */
struct destination_info{
    /** Information about destination inserted by user                      */
    cfg_host*                  info;
    /** All connections created for destination                             */
    std::vector<connection*>   connections;
    /** Next destination to send data for Round robin mode                  */
    destination_info *         next_destination;
};


class Forwarder {
public:
    /**
     * Constructor
     *
     * @param ctx       Context for logging
     * @param config    Configuration of the module
     */
    Forwarder(ipx_ctx_t* ctx, Config* config);

    /**
     * Destructor
     */
    ~Forwarder();

    /**
     * Process incoming message
     *
     * @brief Prepares internal structures for message forwarding
     * @param msg Incoming message
     */
    void processMsg(ipx_msg_t *msg);
    /**
     * Distribute the message to connected hosts
     *
     * @brief Distributes the message in mode given by the user
     * @param msg_ipfix Incoming IPFIX message
     */
    void forward(ipx_msg_ipfix_t *msg_ipfix);

private:
    /**
     * Process IPFIX message
     * @param msg_ipfix IPFIX message
     */
    void processIPFIX(ipx_msg_ipfix_t *msg_ipfix);
    /**
     * Process session message
     * @param msg_session Session message
     */
    void processSession(ipx_msg_session_t *msg_session);

    /**
     * Process single data set
     *
     * @brief Loops over the records in the dataset, counts them and
     * adds the set to the packet builder
     *
     * @param msg       Incoming IPFIX message
     * @param dset      Header of the current data set
     * @param set_end   Pointer to the end of the current set
     * @param rec_i     Index of the first record in the set
     */
    void processDataSet(ipx_msg_ipfix_t *msg, fds_ipfix_set_hdr *dset, uint8_t *set_end, uint32_t *rec_i );

    /**
     * Check for most current templates
     * @brief       Checks if the host has most current templates
     * @param conn  Connection, that will be used for sending the message
     * @return true  Host has the most recent templates
     * @return false Host has old templates
     */
    bool hasLatestTemplates(connection *conn);
    /**
     * Updates templates for host
     * @brief Creates template message and sends it through the specified connection
     * @param msg_ipfix  Incoming IPFIX message
     * @param conn       Connection used for forwarding message
     * @return STATUS_OK        Successfully sent templates
     * @return STATUS_CLOSED   Connection to the host is closed
     * @return STATUS_INVALID   Invalid parameters for connection
     */
    SEND_STATUS updateTemplates(ipx_msg_ipfix_t *msg_ipfix, connection *conn);

    /**
     * Forwards the message to all hosts
     *
     * @brief Loops over all destinations and tries to forward them message
     * If some of the destination is unreachable, drops message for the destination
     * If all the destinations are unreachable, drops message.
     *
     * @param msg_ipfix  Incoming IPFIX message
     */
    void forwardAll(ipx_msg_ipfix_t *msg_ipfix);
    /**
     * Forwards the message in Round robin manner
     * @brief Finds the next host to forward the message to.
     * If the next destination is unreachable, skips the destination and tries another one
     * If all the destinations are unreachable, drops message.
     *
     * @param msg_ipfix Incoming IPFIX message
     */
    void forwardRoundRobin(ipx_msg_ipfix_t *msg_ipfix);

    /**
     * Sends packet through created socket
     *
     * @brief Internal sender for specified connection is used for sending the message
     *
     * @param builder   Packet builder, that contains the message
     * @param conn      Connection used for forwarding the packet
     * @param req_flag  Is the delivery required
     * @return STATUS_OK        Successfully sent message
     * @return STATUS_CLOSED   Connection to the host is closed
     * @return STATUS_INVALID   Invalid parameters for connection
     */
    SEND_STATUS packetSender(fwd_bldr_t *builder, struct connection *conn, bool req_flag);

    /**
     * Finds corresponding output connection for input session
     *
     * @param dest       Destination with connections
     * @param msg_ipfix  Incoming message
     * @return           Pointer to the corresponding connection
     */
    connection *getOutputConnection(destination_info *dest, ipx_msg_ipfix_t *msg_ipfix);


private:
    /** Module context (for logging only!)                                          */
    ipx_ctx_t*      ctx;
    /** Configuration of the module                                                 */
    Config*         config;
    /** Packet builder for data sets only!                                          */
    fwd_bldr_t*     builder;

    /** Snapshot of the currently processed message                                 */
    const fds_tsnapshot_t* curr_msg_snapshot;
    /** ODID of the currently processed message                                     */
    uint32_t               curr_msg_odid;

    /** All the destinations used for forwarding                                    */
    std::vector<destination_info*>   destinations;
    /** Connections that has been closed                                            */
    std::vector<connection*>         idle_connections;
    /** Mutex for accessing destinations                                            */
    std::atomic_flag                 destinations_lock = ATOMIC_FLAG_INIT;

    /** Last destination used for Round robin forwarding                            */
    destination_info*                rr_last_destination;

    /**
     * Reconnection thread function
     * @brief Goes through idle connections and tries to reconnect them
     */
    void reconnector();
    /** Reconnection thread pointer                                                 */
    std::thread             reconnection_thread;
    /** Reconnection thread mutex                                                   */
    std::mutex              reconnection_thread_mutex;
    /** Reconnection thread condition variable for halting the thread               */
    std::condition_variable reconnection_thread_cv;

};


#endif //IPFIXCOL2_FORWARDER_H
