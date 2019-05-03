// SNMP demon, which initializes the MIB modules and dispatches SNMP messages
// Created by root on 11.12.18.
//
#include <iostream>
#include "../Interface.h"
#include "snmp.h"
#include "../Config.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/config_api.h>
#include <sys/types.h>
#include <atomic>
#include <csignal>

SNMPService::SNMPService(Storage *storage, Config *config) {
    this->storage = storage;
    this->config = config;
}

void SNMPService::worker() {

    // make this a client of agentx
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);

    /* initialize tcpip, if necessary */
    SOCK_STARTUP;

    // Initialize sub-agent
    init_agent("ipfixcol2-demon");

    cfg_snmp *cfg = config->outputs.snmp;
    // Register MIB structure to NetSNMP library
    initialize_table_ipfixTransportSessionTable(
            &storage->TransportSessionTable, static_cast<uint>(cfg->timeouts.TransportSessionTable));
    initialize_table_ipfixTransportSessionStatsTable(
            &storage->TransportSessionStatsTable, static_cast<uint>(cfg->timeouts.TemplateTable));
    initialize_table_ipfixTemplateTable(
            &storage->TemplateTable, static_cast<uint>(cfg->timeouts.TemplateDefinitionTable));
    initialize_table_ipfixTemplateStatsTable(
            &storage->TemplateStatsTable, static_cast<uint>(cfg->timeouts.TransportSessionStatsTable));
    initialize_table_ipfixTemplateDefinitionTable(
            &storage->TemplateDefinitionTable, static_cast<uint>(cfg->timeouts.TemplateStatsTable));

    // Initialize demon
    init_snmp("ipfixcol2-demon");

    fd_set read_fd;
    FD_ZERO(&read_fd);
    int numfds;
    int block;
    struct timeval timeout;


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
    snmp_shutdown("ipfixcol2-demon");
    SOCK_CLEANUP;
}

void SNMPService::run() {
    if (pipe(termination_fd) == -1){
        throw std::runtime_error("Failed to create pipe in SNMP module!");
    }
    thread = std::thread(&SNMPService::worker, this);
}



SNMPService::~SNMPService() {
    kill_me = true;

    write(termination_fd[1], "END", 3);

    if (thread.joinable()){
        thread.join();
    }
    close(termination_fd[1]);

}



