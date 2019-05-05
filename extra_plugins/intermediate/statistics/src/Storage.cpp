/**
 * \file Storage.cpp
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Storage for operational statistics based on IPFIX-MIB (source file)
 * \date 2019
 */

/* Copyright (C) 2019 CESNET, z.s.p.o.
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
#include <ipfixcol2/session.h>
#include <zconf.h>
#include <chrono>
#include "Storage.h"
#include "../../../../src/core/message_ipfix.h"
#include "../../../../src/core/message_session.c"

// Lock initialization
std::atomic_flag storage_lock = ATOMIC_FLAG_INIT;

Storage::Storage(Config *config) {
    exporter_counter = 0;
    this->config = config;
    // Starting statistics threads
    rate_counter_thread = std::thread(&Storage::stat_rate_updater, this);
    session_activity_thread = std::thread(&Storage:: session_activity_updater, this);
}

Storage::~Storage() {
    // Terminate all statistics threads
    stat_threads_cv.notify_all();

    if (rate_counter_thread.joinable()){
        rate_counter_thread.join();
    }
    if (session_activity_thread.joinable()){
        session_activity_thread.join();
    }
}

int Storage::process_message(ipx_msg_t *msg) {
    int ret = IPX_OK;
    while(storage_lock.test_and_set(std::memory_order_acquire));
    if (ipx_msg_get_type(msg) == IPX_MSG_IPFIX){
        // Processing IPFIX message
        ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);
        ret = process_ipfix_msg(ipfix_msg);
    }
    else if (ipx_msg_get_type(msg) == IPX_MSG_SESSION){
        // Processing session message
        ipx_msg_session_t *session_msg = ipx_msg_base2session(msg);
        ret = process_session_msg(session_msg);
    }
    storage_lock.clear(std::memory_order_release);
    return ret;
}

/**
 * \brief Processes IPFIX message. Extracts and saves data into internal tables
 * \param msg IPFIX message to process
 * \return IPX_OK IPFIX message processed successfully
 * \return IPX_NOT_FOUND Context in message couldn't be assigned to internal exporter info
 * \return IPX_ERR_FORMAT Error in message format
 */
int Storage::process_ipfix_msg(ipx_msg_ipfix_t *msg) {
    std::string external_id = get_string_id(msg->ctx.session);

    uint32_t internal_id;

    // Finding internal id in internal mapper
    if (active_exporters.find(external_id) == active_exporters.end()){
        // Message from exporter which cannot be identified -> error
        return IPX_ERR_NOTFOUND;
    }
    internal_id = active_exporters[external_id].TransportSessionTableId;

    // Getting through the information in message and saving them in database
    TemplateIndex_t templateTable_index;

    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr = (const struct fds_ipfix_msg_hdr*)ipx_msg_ipfix_get_packet(msg);

    // Updating version number in Transport Session table
    TransportSessionTable[internal_id].IpfixVersion = ntohs(ipfix_msg_hdr->version);

    // Updating transport session stats table
    TransportSessionStatsEntry_t *entry;
    entry = &TransportSessionStatsTable[internal_id];

    check_and_set_discontinuity(entry->Bytes, ntohs(ipfix_msg_hdr->length), &entry->DiscontinuityTime);
    check_and_set_discontinuity(entry->Messages, 1, &entry->DiscontinuityTime);
    check_and_set_discontinuity(entry->Records, ipx_msg_ipfix_get_drec_cnt(msg), &entry->DiscontinuityTime);

    TransportSessionStatsTable[internal_id].Bytes += ntohs(ipfix_msg_hdr->length);
    TransportSessionStatsTable[internal_id].Messages += 1;
    TransportSessionStatsTable[internal_id].Records += ipx_msg_ipfix_get_drec_cnt(msg);

    // Updating internal information about session
    active_exporters[external_id].RateCounter += ntohs(ipfix_msg_hdr->length);
    active_exporters[external_id].LastActive = time(nullptr);

    // Preparing id to TemplateTable
    std::get<0>(templateTable_index) = internal_id; // Transport session id
    std::get<1>(templateTable_index) = msg->ctx.odid;

    struct ipx_ipfix_set *sets;
    size_t set_cnt;
    ipx_msg_ipfix_get_sets(msg, &sets, &set_cnt);

    // iteration through all sets in message
    for (uint32_t i =0; i< set_cnt; ++i) {
        struct ipx_ipfix_set *set = &sets[i];
        uint16_t set_id = ntohs(set->ptr->flowset_id);

        // extracting info about Template and OptionsTemplate sets
        if (set_id == FDS_IPFIX_SET_TMPLT || set_id == FDS_IPFIX_SET_OPTS_TMPLT) {
            struct fds_tset_iter tset_iter;
            fds_tset_iter_init(&tset_iter, set->ptr);

            TemplateDefinitionIndex_t templateDefinitionTable_index;

            // iteration through all templates within a set
            while (fds_tset_iter_next(&tset_iter) == FDS_OK) {
                void *ptr;
                enum fds_template_type type;

                switch (set_id) {
                    case FDS_IPFIX_SET_TMPLT:
                        // Increase template counter in stat table
                        check_and_set_discontinuity(entry->Templates, 1, &entry->DiscontinuityTime);
                        TransportSessionStatsTable[internal_id].Templates++;
                        type = FDS_TYPE_TEMPLATE;
                        ptr = tset_iter.ptr.trec;
                        break;
                    case FDS_IPFIX_SET_OPTS_TMPLT:
                        check_and_set_discontinuity(entry->OptionsTemplates, 1, &entry->DiscontinuityTime);
                        TransportSessionStatsTable[internal_id].OptionsTemplates++;
                        type = FDS_TYPE_TEMPLATE_OPTS;
                        ptr = tset_iter.ptr.opts_trec;
                        break;
                }

                // Parsing template fields
                uint16_t tmplt_size = tset_iter.size;
                struct fds_template *tmplt;
                if (fds_template_parse(type, ptr, &tmplt_size, &tmplt) != FDS_OK) {
                    // Error during parsing template fields
                    fds_template_destroy(tmplt);
                    return IPX_ERR_FORMAT;
                }

                // adding the last index for TemplateTable
                std::get<2>(templateTable_index) = tmplt->id;

                // Updating information in TemplateTable
                TemplateTable[templateTable_index].TransportSessionIndex = internal_id;
                TemplateTable[templateTable_index].ObservationDomainId = msg->ctx.odid;
                TemplateTable[templateTable_index].Id = tmplt->id;
                TemplateTable[templateTable_index].SetId = set_id;
                TemplateTable[templateTable_index].AccessTime = ntohl(ipfix_msg_hdr->export_time);

                // Filling TemplateDefinitionIndex
                std::get<0>(templateDefinitionTable_index) = templateTable_index;
                for (uint16_t i = 0; i < tmplt->fields_cnt_total; ++i) {
                    struct fds_tfield current = tmplt->fields[i];
                    std::get<1>(templateDefinitionTable_index) = i;

                    // Updating information in Template Definition Table
                    TemplateDefinitionTable[templateDefinitionTable_index].TransportSessionIndex = internal_id;
                    TemplateDefinitionTable[templateDefinitionTable_index].ObservationDomainId = msg->ctx.odid;
                    TemplateDefinitionTable[templateDefinitionTable_index].TemplateId = tmplt->id;
                    TemplateDefinitionTable[templateDefinitionTable_index].Index = i;
                    TemplateDefinitionTable[templateDefinitionTable_index].Id = current.id;
                    TemplateDefinitionTable[templateDefinitionTable_index].EnterpriseNumber = current.en;
                    TemplateDefinitionTable[templateDefinitionTable_index].Length = current.length;
                    TemplateDefinitionTable[templateDefinitionTable_index].Flags = current.flags;
                }
                fds_template_destroy(tmplt);
            }
        }
    }

    // iteration through all data sets in message for collecting statistics regarding used templates
    TemplateStatsIndex_t templateStatTable_index;
    std::get<0>(templateStatTable_index) = internal_id;
    std::get<1>(templateStatTable_index) = msg->ctx.odid;;

    uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);
    for (uint32_t i = 0; i< rec_cnt; ++i){
        struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(msg, i);
        if (ipfix_rec == nullptr) return IPX_ERR_FORMAT;

        // get the template id for indexing stat table
        std::get<2>(templateStatTable_index) = ipfix_rec->rec.tmplt->id;
        TemplateStatsTable[templateStatTable_index].TransportSessionIndex = internal_id;
        TemplateStatsTable[templateStatTable_index].ObservationDomainId = msg->ctx.odid;
        TemplateStatsTable[templateStatTable_index].TemplateId = ipfix_rec->rec.tmplt->id;
        check_and_set_discontinuity(TemplateStatsTable[templateStatTable_index].DataRecords, 1,
                &TemplateStatsTable[templateStatTable_index].DiscontinuityTime);
        TemplateStatsTable[templateStatTable_index].DataRecords += 1;
    }

    return IPX_OK;
}

/**
 * \brief Processes session message. Creates and deletes session records;
 * \param msg Session message to process
 * \return IPX_OK Session message processed successfully
 */
int Storage::process_session_msg(ipx_msg_session_t *msg) {
    // constructing string identificator of exporter from IP and port numbers
    std::string str_id = get_string_id(msg->session);

    if (ipx_msg_session_get_event(msg) == IPX_MSG_SESSION_OPEN){
        // adding new record to TransportSession table
        uint32_t int_id;

        if (previously_connected_exporters.find(str_id) != previously_connected_exporters.end()){
            // Withdrawal of ID from history of previously connected exporters
            int_id = previously_connected_exporters[str_id];
            previously_connected_exporters.erase(str_id);
        } else{
            // Assigning new ID to session
            int_id = exporter_counter;
            exporter_counter++;
        }
        active_exporters[str_id].TransportSessionTableId = int_id;
        active_exporters[str_id].RateCounter = 0;

        TransportSessionEntry_t *new_entry = &TransportSessionTable[int_id];
        bzero(new_entry, sizeof(*new_entry));
        new_entry->Index = int_id;

        // Saving specific information for each transport protocol
        const struct ipx_session_net *net = nullptr;
        if (msg->session->type == FDS_SESSION_UDP){
            new_entry->Protocol = IPPROTO_UDP;
            new_entry->TemplateRefreshTimeout = msg->session->udp.lifetime.tmplts;
            new_entry->OptionsTemplateRefreshTimeout = msg->session->udp.lifetime.opts_tmplts;
            net = &msg->session->udp.net;
        }
        else if (msg->session->type == FDS_SESSION_TCP){
            new_entry->Protocol = IPPROTO_TCP;
            net = &msg->session->tcp.net;
        }
        else if (msg->session->type == FDS_SESSION_SCTP){
            new_entry->Protocol = IPPROTO_SCTP;
            net = &msg->session->sctp.net;
            // SCTP Association ID can be added here
        }
        // Extracting Transport information
        if (net != nullptr){
            new_entry->SourceAddressType = net->l3_proto;
            new_entry->DestinationAddressType = net->l3_proto;
            if (net->l3_proto == AF_INET){
                new_entry->SourceAddress.IPv4 = net->addr_src.ipv4;
                new_entry->DestinationAddress.IPv4 = net->addr_dst.ipv4;
            }
            else {
                new_entry->SourceAddress.IPv6 = net->addr_src.ipv6;
                new_entry->DestinationAddress.IPv6 = net->addr_dst.ipv6;
            }
            new_entry->SourcePort = net->port_src;
            new_entry->DestinationPort = net->port_dst;
        }
        new_entry->DeviceMode = IPX_DEVICEMODE_COLLECTING;
        new_entry->Status = IPX_SESSIONSTATUS_ACTIVE;

        // Adding entry to TransportSessionStatsTable
        TransportSessionStatsTable[int_id].TransportSessionIndex = int_id;

    }
    else if (ipx_msg_session_get_event(msg) == IPX_MSG_SESSION_CLOSE){
        // delete from internal ID map
        uint32_t int_id;
        if (active_exporters.find(str_id) != active_exporters.end()){
            int_id = active_exporters[str_id].TransportSessionTableId;

            // Erasing records from MIB database
            TransportSessionTable.erase(int_id);
            TransportSessionStatsTable.erase(int_id);

            for (auto i = TemplateTable.begin(); i != TemplateTable.end(); ){
                if (i->second.TransportSessionIndex == int_id){
                    i = TemplateTable.erase(i);
                } else{
                    i++;
                }
            }
            for (auto i = TemplateDefinitionTable.begin(); i != TemplateDefinitionTable.end(); ){
                if (i->second.TransportSessionIndex == int_id){
                    i = TemplateDefinitionTable.erase(i);
                } else {
                    i++;
                }
            }
            for (auto i = TemplateStatsTable.begin(); i != TemplateStatsTable.end(); ){
                if (i->second.TransportSessionIndex == int_id){
                    i = TemplateStatsTable.erase(i);
                } else {
                    i++;
                }
            }
            // Saving the ID into history of previously connected exporters
            previously_connected_exporters[str_id] = int_id;
            // Deleting from active exporter mapper
            active_exporters.erase(str_id);
        }

    }
    return IPX_OK;
}

/**
 * \brief Checks if the counter will overflow in case of addition.
 * If so, the discontinuity timestamp will be set accordingly
 *
 * \param counter Value of counter which we need increment
 * \param increment Value of increment to counter
 * \param discont_flag Pointer to the timestamp which will be set in case of discontinuity
 */
void Storage::check_and_set_discontinuity(uint64_t counter, uint64_t increment, time_t *discont_flag) {
    if (counter + increment < counter){
        *discont_flag = time(nullptr);
    }
}

/**
 * \brief Function for thread rate_counter_thread.
 *
 * Loops through all saved sessions and updates rate value every second.
 * This is done by copying value from internal counter to MIB table structure, thus
 * the counter won't be steeply increasing every second.
 */
void Storage::stat_rate_updater() {
    std::unique_lock<std::mutex> lock(stat_threads_mutex);

    std::chrono::seconds sec(RATE_UPDATER_INTERVAL);

    // Main loop of thread
    while (stat_threads_cv.wait_for(lock, sec) == std::cv_status::timeout){
        std::map<std::string, struct ExporterInfo>::iterator it;

        while(storage_lock.test_and_set(std::memory_order_acquire));
        // iterate over all active sessions
        for ( it = active_exporters.begin(); it != active_exporters.end(); it++){
            // Copy counted bytes into MIB table
            TransportSessionStatsTable[it->second.TransportSessionTableId].Rate = it->second.RateCounter;
            it->second.RateCounter = 0;
        }
        storage_lock.clear(std::memory_order_release);
    }
}

/**
 * \brief Function for thread session_activity_thread.
 *
 * Loops through all saved sessions and checks and sets their activity.
 * Thread sleeps for time specified in configuration. If the transport
 * session isn't active for the same time or longer, it is marked as inactive.
 */
void Storage::session_activity_updater() {
    std::unique_lock<std::mutex> lock(stat_threads_mutex);

    std::chrono::seconds sec(config->session_activity_timeout);
    // Main loop of thread
    while (stat_threads_cv.wait_for(lock, sec) == std::cv_status::timeout){
        std::map<std::string, struct ExporterInfo>::iterator it;

        while(storage_lock.test_and_set(std::memory_order_acquire));
        // Iterate over all active sessions
        for ( it = active_exporters.begin(); it != active_exporters.end(); it++){
            if (difftime(time(nullptr), it->second.LastActive) >= config->session_activity_timeout){
                // If the time of last activity and now is bigger than timeout - mark as INACTIVE
                TransportSessionTable[it->second.TransportSessionTableId].Status = IPX_SESSIONSTATUS_INACTIVE;
            }else{
                // Otherwise mark as ACTIVE
                TransportSessionTable[it->second.TransportSessionTableId].Status = IPX_SESSIONSTATUS_ACTIVE;
            }
        }
        storage_lock.clear(std::memory_order_release);
    }
}

/**
 * \brief Creates unique string ID from information about session
 *
 * Function extracts information about session, converts them to string format
 * and returns this as a unique identification of each session.
 * ID is in format [SourceIP]:[SourcePort][Protocol][DestinationIP]:[DestinationPort]
 * In our case, destination IP will be IP address of collector in session. Same applies
 * for destination port. This format was chosen for easier debugging.
 * \param session Pointer to information about session
 * \return String representing unique ID of session
 */
std::string Storage::get_string_id(const ipx_session *session) {
    std::string str_id = session->ident;
    const struct ipx_session_net *net = nullptr;
    // Determine what protocol is used
    if (session->type == FDS_SESSION_UDP){
        str_id += "UDP";
        net = &session->udp.net;
    }
    else if (session->type == FDS_SESSION_TCP){
        str_id += "TCP";
        net = &session->tcp.net;
    }
    else if (session->type == FDS_SESSION_SCTP){
        str_id += "SCTP";
        net = &session->sctp.net;
    }
    else{
        throw std::runtime_error("Invalid session type!");
    }
    // Convert IP address of Destination to string
    char buff[INET6_ADDRSTRLEN];
    if (net->l3_proto == AF_INET){
        inet_ntop(AF_INET, &net->addr_dst.ipv4, buff, INET6_ADDRSTRLEN);
    } else {
        inet_ntop(AF_INET6, &net->addr_dst.ipv6, buff, INET6_ADDRSTRLEN);
    }
    // Concatenate all information and return
    str_id += buff;
    str_id += ":" + std::to_string(net->port_dst);
    return str_id;
}



