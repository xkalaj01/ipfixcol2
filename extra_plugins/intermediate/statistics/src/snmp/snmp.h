/**
 * \file snmp.h
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief SNMP agent for data export (header file)
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

#ifndef STATISTICS_SNMP_H
#define STATISTICS_SNMP_H

#include "../Service.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table_dataset.h>
#include "mib_tables/ipfixTransportSessionTable.h"
#include "mib_tables/ipfixTransportSessionStatsTable.h"
#include "mib_tables/ipfixTemplateTable.h"
#include "mib_tables/ipfixTemplateStatsTable.h"
#include "mib_tables/ipfixTemplateDefinitionTable.h"
#include "../Config.h"

struct table_caches;

class SNMPService: public StatisticsService{
public:
    /**
     * \brief Constructor of the SNMP agent class
     * \param storage   Internal storage of data to transport
     * \param config    Configuration of statistics module
     */
    explicit SNMPService(Storage *storage, Config *config);

    /**
     * \brief Destructor
     */
    ~SNMPService() override;

    /**
     * \brief Start and run the agent
     */
    void run() override;

    /**
     * \brief Agent behavior, when interface notifies it - none in this case
     */
    void on_notify() override {};

private:
    void worker();
    void free_internals(table_caches *caches);

    /** Thread, in which the agent runs                                                 */
    std::thread thread;
    /** Flag for turning off the agent                                                  */
    bool kill_me = false;
    /** Pipe used for notifying the agent about shutdown signal                         */
    int termination_fd[2];
    /** Flag to determine if the initialization in agent was done successfully          */
    bool initialized = false;
    /** Mutex for synchronization of the agent and main thread during initialization    */
    std::mutex init_mutex;
    /** Internal storage containing data to export                                      */
    Storage *storage;
    /** Configuration of Statistics module containing configuration of this agent       */
    Config *config;
};

#endif //STATISTICS_SNMP_H
