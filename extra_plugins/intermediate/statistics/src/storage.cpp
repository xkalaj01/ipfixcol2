//
// Created by root on 22.3.19.
//

#include <iostream>
#include <ipfixcol2/session.h>
#include "storage.h"
#include "../../../../src/core/message_ipfix.h"
#include "../../../../src/core/message_session.c"

std::atomic_flag storage_lock = ATOMIC_FLAG_INIT;

Storage::Storage() {
    exporter_counter = 0;
}

int Storage::process_message(ipx_msg_t *msg) {
    while(storage_lock.test_and_set(std::memory_order_acquire));
    if (ipx_msg_get_type(msg) == IPX_MSG_IPFIX){
        // Processing IPFIX message
        ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);
        this->process_ipfix_msg(ipfix_msg);
    }
    else if (ipx_msg_get_type(msg) == IPX_MSG_SESSION){
        // Processing session message
        ipx_msg_session_t *session_msg = ipx_msg_base2session(msg);
        this->process_session_msg(session_msg);
    }
    storage_lock.clear(std::memory_order_release);
    return IPX_OK;
}

int Storage::process_ipfix_msg(ipx_msg_ipfix_t *msg) {
    std::string external_id = msg->ctx.session->ident;
    uint32_t internal_id;

    // Finding internal id in internal mapper
    if (this->active_exporters.find(external_id) == this->active_exporters.end()){
        // Message from exporter which cannot be identified -> error
        return IPX_ERR_NOTFOUND;
    }
    internal_id = this->active_exporters[external_id].TransportSessionTableId;

    // Getting through the information in message and saving them in database
    TemplateTableIndex_t templateTable_index;

    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr = (const struct fds_ipfix_msg_hdr*)ipx_msg_ipfix_get_packet(msg);

    // Updating version number in Transport Session table
    this->TransportSessionTable[internal_id].IpfixVersion = ntohs(ipfix_msg_hdr->version);

    // Updating transport session stats table
    TransportSessionStatsEntry_t *entry;
    entry = &this->TransportSessionStatsTable[internal_id];

    this->check_and_set_discontinuity(entry->Bytes, ntohs(ipfix_msg_hdr->length), &entry->DiscontinuityTime);
    this->check_and_set_discontinuity(entry->Messages, 1, &entry->DiscontinuityTime);
    this->check_and_set_discontinuity(entry->Records, ipx_msg_ipfix_get_drec_cnt(msg), &entry->DiscontinuityTime);

    // For testing to see, if Discontinuity time works
//    if (this->TransportSessionStatsTable[internal_id].Messages == 10){
//        this->TransportSessionStatsTable[internal_id].DiscontinuityTime = time(nullptr);
//    }

    this->TransportSessionStatsTable[internal_id].Bytes += ntohs(ipfix_msg_hdr->length);
    this->TransportSessionStatsTable[internal_id].Messages += 1;
    this->TransportSessionStatsTable[internal_id].Records += ipx_msg_ipfix_get_drec_cnt(msg);
    this->active_exporters[external_id].RateCounter += ntohs(ipfix_msg_hdr->length);

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
                        this->check_and_set_discontinuity(entry->Templates, 1, &entry->DiscontinuityTime);
                        this->TransportSessionStatsTable[internal_id].Templates++;
                        type = FDS_TYPE_TEMPLATE;
                        ptr = tset_iter.ptr.trec;
                        break;
                    case FDS_IPFIX_SET_OPTS_TMPLT:
                        this->check_and_set_discontinuity(entry->OptionsTemplates, 1, &entry->DiscontinuityTime);
                        this->TransportSessionStatsTable[internal_id].OptionsTemplates++;
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
                this->TemplateTable[templateTable_index].TransportSessionIndex = internal_id;
                this->TemplateTable[templateTable_index].ObservationDomainId = msg->ctx.odid;
                this->TemplateTable[templateTable_index].Id = tmplt->id;
                this->TemplateTable[templateTable_index].SetId = set_id;
                this->TemplateTable[templateTable_index].AccessTime = ntohl(ipfix_msg_hdr->export_time);

                // Filling TemplateDefinitionIndex
                std::get<0>(templateDefinitionTable_index) = templateTable_index;
                for (uint16_t i = 0; i < tmplt->fields_cnt_total; ++i) {
                    struct fds_tfield current = tmplt->fields[i];
                    std::get<1>(templateDefinitionTable_index) = i;

                    // Updating information in Template Definition Table
                    this->TemplateDefinitionTable[templateDefinitionTable_index].TransportSessionIndex = internal_id;
                    this->TemplateDefinitionTable[templateDefinitionTable_index].ObservationDomainId = msg->ctx.odid;
                    this->TemplateDefinitionTable[templateDefinitionTable_index].TemplateId = tmplt->id;
                    this->TemplateDefinitionTable[templateDefinitionTable_index].Index = i;
                    this->TemplateDefinitionTable[templateDefinitionTable_index].Id = current.id;
                    this->TemplateDefinitionTable[templateDefinitionTable_index].EnterpriseNumber = current.en;
                    this->TemplateDefinitionTable[templateDefinitionTable_index].Length = current.length;
                    this->TemplateDefinitionTable[templateDefinitionTable_index].Flags = current.flags;
                }
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
        if (ipfix_rec == NULL) return IPX_ERR_FORMAT;

        // get the template id for indexing stat table
        std::get<2>(templateStatTable_index) = ipfix_rec->rec.tmplt->id;
        this->TemplateStatsTable[templateStatTable_index].TransportSessionIndex = internal_id;
        this->TemplateStatsTable[templateStatTable_index].ObservationDomainId = msg->ctx.odid;
        this->TemplateStatsTable[templateStatTable_index].TemplateId = ipfix_rec->rec.tmplt->id;
        this->check_and_set_discontinuity(this->TemplateStatsTable[templateStatTable_index].DataRecords, 1,
                &this->TemplateStatsTable[templateStatTable_index].DiscontinuityTime);
        this->TemplateStatsTable[templateStatTable_index].DataRecords += 1;
    }

    return IPX_OK;
}

int Storage::process_session_msg(ipx_msg_session_t *msg) {
    if (ipx_msg_session_get_event(msg) == IPX_MSG_SESSION_OPEN){

        // adding new record to TransportSession table
        uint32_t int_id;

        std::string str_id = msg->session->ident;

        if (this->previously_connected_exporters.find(str_id) != this->previously_connected_exporters.end()){
            // Whitdrawal of ID from history of previously connected exporters
            int_id = this->previously_connected_exporters[str_id];
            this->previously_connected_exporters.erase(str_id);
        } else{
            // Assigning new ID to session
            int_id = exporter_counter;
            this->exporter_counter++;
        }
        this->active_exporters[str_id].TransportSessionTableId = int_id;
        this->active_exporters[str_id].RateCounter = 0;

        TransportSessionEntry_t *new_entry = &this->TransportSessionTable[int_id];
        bzero(new_entry, sizeof(*new_entry));
        new_entry->Index = int_id;

        const struct ipx_session_net *net = nullptr;
        // Saving specific information for each transport protocol
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
        this->TransportSessionStatsTable[int_id].TransportSessionIndex = int_id;

    }
    else if (ipx_msg_session_get_event(msg) == IPX_MSG_SESSION_CLOSE){
        // delete from numerical ID map
        std::string str_id = msg->session->ident;
        uint32_t int_id;
        if (this->active_exporters.find(str_id) != this->active_exporters.end()){
            int_id = this->active_exporters[str_id].TransportSessionTableId;

            // Erasing records from MIB database
            this->TransportSessionTable.erase(int_id);
            this->TransportSessionStatsTable.erase(int_id);

            for (auto &i : this->TemplateTable){
                if (i.second.TransportSessionIndex == int_id){
                    this->TemplateTable.erase(i.first);
                }
            }
            for (auto &i : this->TemplateDefinitionTable){
                if (i.second.TransportSessionIndex == int_id){
                    this->TemplateDefinitionTable.erase(i.first);
                }
            }
            for (auto &i : this->TemplateStatsTable){
                if (i.second.TransportSessionIndex == int_id){
                    this->TemplateStatsTable.erase(i.first);
                }
            }
            // Saving the ID into history of exporters
            this->previously_connected_exporters[str_id] = int_id;
            // Deleting from active exporter mapper
            this->active_exporters.erase(str_id);
        }

    }
    return IPX_OK;
}

void Storage::check_and_set_discontinuity(uint64_t counter, uint64_t increment, time_t *discont_flag) {
    if (counter + increment < counter){
        *discont_flag = time(nullptr);
    }
}


