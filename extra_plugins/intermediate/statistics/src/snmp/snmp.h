//
// Created by root on 26.2.19.
//

#ifndef STATISTICS_SNMP_H
#define STATISTICS_SNMP_H

#include "../service.h"
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

class SNMPService: public StatisticsService{
public:
    explicit SNMPService(Storage *storage);

    ~SNMPService() override {};

    void run() override;
    void on_notify() override {};
    void destroy() override;

private:
    void worker();
    std::thread thread;
    bool kill_me = 0;
    Storage *storage;
};

#endif //STATISTICS_SNMP_H
