/**
 * \file snmp.c
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief SNMP agent for data export (source file)
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
#include "../Interface.h"
#include "snmp.h"
#include "../Config.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/config_api.h>
#include <net-snmp/session_api.h>
#include <sys/types.h>
#include <atomic>
#include <csignal>

/** Structure for keeping track of allocated memory during initialization */
struct table_caches{
    ipfixTransportSession_cache_data    *TransportSession;
    ipfixTransportSessionStats_cache_data *TransportSessionStats;
    ipfixTemplateDefinition_cache_data  *TemplateDefinition;
    ipfixTemplateStats_cache_data       *TemplateStats;
    ipfixTemplate_cache_data            *Template;
};

SNMPService::SNMPService(Storage *storage, Config *config) {
    this->storage = storage;
    this->config = config;
    init_mutex.lock();
}
SNMPService::~SNMPService() {
    // Kill thread
    kill_me = true;
    write(termination_fd[1], "END", 3);

    // Join thread
    if (thread.joinable()){
        thread.join();
    }
    // close pipe
    close(termination_fd[1]);
}
void SNMPService::run() {
    // Create pipe for thread termination
    if (pipe(termination_fd) == -1){
        throw std::runtime_error("Failed to create pipe in SNMP module!");
    }

    // Start thread and wait for it to be initialized
    thread = std::thread(&SNMPService::worker, this);
    init_mutex.lock();

    // check successful initialization
    if (!initialized){
        throw std::runtime_error("Failed to initialize SNMP module. Is the snmpd running as master AgentX?");
    }
}

/**
 * \brief Main worker of the agent. This function is ran in separate thread.
 */
void SNMPService::worker() {
    table_caches caches{};

    // make this a client of agentx
    if (netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1)!=0){
        initialized = false;
        kill_me = true;
        return;
    };

    /* initialize tcpip, if necessary */
    SOCK_STARTUP;

    // Initialize sub-agent
    if (init_agent("ipfixcol2-daemon") != 0){
        initialized = false;
        kill_me = true;
    }

    cfg_snmp *cfg = config->outputs.snmp;

    // Initialize the MIB tables and register them to master agent
    caches.TransportSession = initialize_table_ipfixTransportSessionTable(
            &storage->TransportSessionTable, static_cast<uint>(cfg->timeouts.TransportSessionTable));
    caches.TransportSessionStats = initialize_table_ipfixTransportSessionStatsTable(
            &storage->TransportSessionStatsTable, static_cast<uint>(cfg->timeouts.TransportSessionStatsTable));
    caches.Template = initialize_table_ipfixTemplateTable(
            &storage->TemplateTable, static_cast<uint>(cfg->timeouts.TemplateTable));
    caches.TemplateStats = initialize_table_ipfixTemplateStatsTable(
            &storage->TemplateStatsTable, static_cast<uint>(cfg->timeouts.TemplateStatsTable));
    caches.TemplateDefinition = initialize_table_ipfixTemplateDefinitionTable(
            &storage->TemplateDefinitionTable, static_cast<uint>(cfg->timeouts.TemplateDefinitionTable));

    // Initialize daemon
    init_snmp("ipfixcol2-daemon");

    // check connection to the master agent
    const char * agentx_socket = netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_X_SOCKET);
    netsnmp_transport *t = netsnmp_transport_open_client("agentx", agentx_socket);
    if (t == NULL){
        initialized = false;
        kill_me = true;
    } else{
        initialized = true;
    }

    fd_set read_fd;
    FD_ZERO(&read_fd);
    int numfds = 0;
    int block = 0;
    struct timeval timeout;
    timeout.tv_usec = 0;
    timeout.tv_sec = 0;

    // Signal, that the thread is done with initialization
    init_mutex.unlock();

    // Main loop of thread for dispatching processes
    while(!kill_me){

        // prepare descriptors and info for select()
        snmp_select_info(&numfds, &read_fd, &timeout, &block);
        FD_SET(termination_fd[0], &read_fd);

        // Wait for incoming SNMP requests
        select(numfds, &read_fd, nullptr, nullptr, nullptr);

        // Dispatch all requests waiting to be processed
        while (agent_check_and_process(0));
    }
    // cleanup
    close(termination_fd[0]);
    snmp_shutdown("ipfixcol2-daemon");
    netsnmp_transport_free(t);
    shutdown_agent();
    SOCK_CLEANUP;
    free_internals(&caches);
}

/**
 * \brief Frees memory allocated during initialization of MIB tables
 *
 * Net-SNMP library leaves some unfreed memory after shutdown functions in main worker.
 * This functions frees the hanging memory.
 * \param caches Structure containing the pointers to allocated memory
 */
void SNMPService::free_internals(table_caches *caches) {
    if (caches == NULL){
        return;
    }
    if (caches->TemplateDefinition != NULL){
        netsnmp_tdata_delete_table(caches->TemplateDefinition->table);
        netsnmp_cache_remove(caches->TemplateDefinition->cache);
        netsnmp_table_registration_info_free(caches->TemplateDefinition->table_info);
        free(caches->TemplateDefinition->cache->rootoid);
        free(caches->TemplateDefinition->cache);
        free(caches->TemplateDefinition);
    }

    if (caches->TemplateStats != NULL){
        netsnmp_tdata_delete_table(caches->TemplateStats->table);
        netsnmp_cache_remove(caches->TemplateStats->cache);
        netsnmp_table_registration_info_free(caches->TemplateStats->table_info);
        free(caches->TemplateStats->cache->rootoid);
        free(caches->TemplateStats->cache);
        free(caches->TemplateStats);
    }

    if (caches->Template != NULL){
        netsnmp_tdata_delete_table(caches->Template->table);
        netsnmp_cache_remove(caches->Template->cache);
        netsnmp_table_registration_info_free(caches->Template->table_info);
        free(caches->Template->cache->rootoid);
        free(caches->Template->cache);
        free(caches->Template);
    }

    if (caches->TransportSessionStats != NULL){
        netsnmp_tdata_delete_table(caches->TransportSessionStats->table);
        netsnmp_cache_remove(caches->TransportSessionStats->cache);
        netsnmp_table_registration_info_free(caches->TransportSessionStats->table_info);
        free(caches->TransportSessionStats->cache->rootoid);
        free(caches->TransportSessionStats->cache);
        free(caches->TransportSessionStats);
    }

    if (caches->TransportSession != NULL){
        netsnmp_tdata_delete_table(caches->TransportSession->table);
        netsnmp_cache_remove(caches->TransportSession->cache);
        netsnmp_table_registration_info_free(caches->TransportSession->table_info);
        free(caches->TransportSession->cache->rootoid);
        free(caches->TransportSession->cache);
        free(caches->TransportSession);
    }
}



