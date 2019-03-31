// SNMP demon, which initializes the MIB modules and dispatches SNMP messages
// Created by root on 11.12.18.
//
#include <iostream>
#include "../interface.h"
#include "snmp.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/config_api.h>
#include <sys/types.h>
#include <atomic>


void SNMPService::worker() {

    // make this a client of agentx
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);

    /* initialize tcpip, if necessary */
    SOCK_STARTUP;

    // Initialize sub-agent
    init_agent("ipfixcol2-demon");

    // Register MIB structure to NetSNMP library
    initialize_table_ipfixTransportSessionTable(&this->storage->TransportSessionTable, 1);
    initialize_table_ipfixTransportSessionStatsTable(&this->storage->TransportSessionStatsTable, 1);
    initialize_table_ipfixTemplateTable(&this->storage->TemplateTable, 1);
    initialize_table_ipfixTemplateStatsTable(&this->storage->TemplateStatsTable, 1);
    initialize_table_ipfixTemplateDefinitionTable(&this->storage->TemplateDefinitionTable, 1);

    // Initialize demon
    init_snmp("ipfixcol2-demon");

    // Logging the status of this demon
//    snmp_log(LOG_INFO,"ipfixcol2-demon is up and running.\n");

    fd_set read_fd;
    int numfds;
    int block;
    struct timeval timeout;

    // Main loop of thread for dispatching processes
    while(!this->kill_me){

        // Wait for incoming SNMP requests
        snmp_select_info(&numfds, &read_fd, &timeout, &block);
        select(numfds, &read_fd, nullptr, nullptr, nullptr);

        // Dispatch all requests waiting to be processed
        while (agent_check_and_process(0)){
        }

    }

    snmp_shutdown("ipfixcol2-demon");
    SOCK_CLEANUP;
    std::cout<<"SNMP module ends"<<std::endl;
}

void SNMPService::run() {
    this->thread = std::thread(&SNMPService::worker, this);
}

SNMPService::SNMPService(Storage *storage) {
    this->storage = storage;
}

void SNMPService::destroy() {
    this->kill_me = 1;
    this->thread.join();
    std::cout<<"SNMP Service ends"<<std::endl;
}



