// SNMP demon, which initializes the MIB modules and dispatches SNMP messages
// Created by root on 11.12.18.
//
#include <iostream>
#include "../services.h"
#include "../map.h"
#include "mib.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/config_api.h>
#include <sys/types.h>
#include <atomic>

void snmp_agent(struct Instance *data){
    int agentx_subagent=1;  /* change this if you want to be a SNMP master agent <-- can be in configuration file */
    int background = 0;     /* change this if you want to run in the background <-- can be in configuration file */
    int syslog = 0;         /* change this if you want to use syslog <-- can be in configuration file */

    /* print log errors to syslog or stderr */
    if (syslog)
        snmp_enable_calllog();
    else
        snmp_enable_stderrlog();

    /* we're an agentx subagent? */
    if (agentx_subagent) {
        /* make us a agentx client. */
        netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
    }

    /* run in background, if requested */
    if (background && netsnmp_daemonize(1, !syslog))
        exit(1);

    /* initialize tcpip, if necessary */
    SOCK_STARTUP;

    /* initialize the agent library */
    init_agent("ipfixcol2-demon");

    /* initialize mib code here */
    init_mib_internals();

    /* mib code: init_nstAgentSubagentObject from nstAgentSubagentObject.C */
//    variables_init(stats);
    data->mib->registerMIB(&data->uns_var, &data->int_var);

    /* initialize vacm/usm access control  */
//    if (!agentx_subagent) {
//        init_vacm_vars();
//        init_usmUser();
//    }

    /* example-demon will be used to read example-demon.conf files. */
    init_snmp("ipfixcol2-demon");

    /* If we're going to be a snmp master agent, initial the ports */
    if (!agentx_subagent)
        init_master_agent();  /* open the port to listen on (defaults to udp:161) */

    snmp_log(LOG_INFO,"ipfixcol2-demon is up and running.\n");

    fd_set read_fd;
    int numfds;
    int block;
    int ready;
    struct timeval timeout;
    /* your main loop here... */
    while(!data->snmp_kill) {
        // Wait for incoming SNMP requests
        snmp_select_info(&numfds, &read_fd, &timeout, &block);
        select(numfds, &read_fd, NULL, NULL, NULL);

        // spinlock
        while (data->mib_lock.test_and_set(std::memory_order_acquire));
        printf("+SPINLOCK LOCKED IN DEMON\n");
        // Dispatch all requests waiting to be processed
        while (agent_check_and_process(0)){
            printf("SNMP request dispatched\n");
        }
        printf("-SPINLOCK UNLOCKED IN DEMON\n");
        data->mib_lock.clear(std::memory_order_release);
    }

    /* at shutdown time */
    snmp_shutdown("ipfixcol2-demon");
    printf("Shutting down ipfixcol2-snmp demon\n");
    SOCK_CLEANUP;

}