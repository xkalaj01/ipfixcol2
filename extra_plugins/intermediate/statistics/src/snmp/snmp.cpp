//
// Created by root on 11.12.18.
//
#include <iostream>
#include "../services.h"
#include "../map.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/config_api.h>

void variables_init(StatMap *stats);

void snmp_agent(StatMap *stats, uint16_t *killme){
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
    variables_init(stats);

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

    /* your main loop here... */
    while(!*killme) {
        /* if you use select(), see snmp_select_info() in snmp_api(3) */
        /*     --- OR ---  */
        agent_check_and_process(1); /* 0 == don't block */
        printf("Packet counter: [%lu] KillMe: [%d] \n",stats->pkt_counter, *killme);
    }

    /* at shutdown time */
    snmp_shutdown("ipfixcol2-demon");
    SOCK_CLEANUP;

}

void variables_init(StatMap *stats){
    static oid ipfixTransportSessionIndex_oid[] = { 1, 3, 6, 1, 2, 1, 193, 1, 1, 1, 1, 1, 0 };
    netsnmp_register_read_only_counter32_instance("nstAgentSubagentObject",
                                   ipfixTransportSessionIndex_oid,
                                   OID_LENGTH(ipfixTransportSessionIndex_oid),
                                   &stats->pkt_counter, NULL);
}