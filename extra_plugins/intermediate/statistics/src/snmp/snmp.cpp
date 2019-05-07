// SNMP daemon, which initializes the MIB modules and dispatches SNMP messages
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
#include <net-snmp/session_api.h>
#include <sys/types.h>
#include <atomic>
#include <csignal>

SNMPService::SNMPService(Storage *storage, Config *config) {
    this->storage = storage;
    this->config = config;
    init_mutex.lock();
}

void SNMPService::worker() {

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
    // Register MIB structure to NetSNMP library
    initialize_table_ipfixTransportSessionTable(
            &storage->TransportSessionTable, static_cast<uint>(cfg->timeouts.TransportSessionTable));
    initialize_table_ipfixTransportSessionStatsTable(
            &storage->TransportSessionStatsTable, static_cast<uint>(cfg->timeouts.TransportSessionStatsTable));
    initialize_table_ipfixTemplateTable(
            &storage->TemplateTable, static_cast<uint>(cfg->timeouts.TemplateTable));
    initialize_table_ipfixTemplateStatsTable(
            &storage->TemplateStatsTable, static_cast<uint>(cfg->timeouts.TemplateStatsTable));
    initialize_table_ipfixTemplateDefinitionTable(
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
    SOCK_CLEANUP;
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



