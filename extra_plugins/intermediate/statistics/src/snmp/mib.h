#ifndef IPFIXCOL2_MIB_H
#define IPFIXCOL2_MIB_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table_dataset.h>
#include <ipfixcol2.h>


struct ipfixMainObjects{
    netsnmp_table_data_set *ipfixTransportSessionTable;
    netsnmp_table_data_set *ipfixTemplateTable;
    netsnmp_table_data_set *ipfixTemplateDefinitionTable;
    netsnmp_table_data_set *ipfixExportTable;
    netsnmp_table_data_set *ipfixMeteringProcessTable;
    netsnmp_table_data_set *ipfixObservationPointTable;
    netsnmp_table_data_set *ipfixSelectionProcessTable;
};

struct ipfixStatistics{
    netsnmp_table_data_set *ipfixTransportSessionStatsTable;
    netsnmp_table_data_set *ipfixTemplateStatsTable;
    netsnmp_table_data_set *ipfixMeteringProcessStatsTable;
    netsnmp_table_data_set *ipfixSelectionProcessStatsTable;
};

struct ipfixObjects{
    struct ipfixMainObjects ipfixMainObjects;
    struct ipfixStatistics  ipfixStatistics;
};

struct ipfixMIB{
    struct ipfixObjects ipfixObjects;
};

class MIBBase{
public:
    MIBBase();

    void registerMIB(uint *num, int *num2);
    void processPacket(ipx_msg_ipfix_t *ipfix_msg);

private:
    struct ipfixMainObjects _MainObjects;

    void TransportSessionTable_init();
    void TemplateTable_init();
    void TemplateDefinitionTable_init();
    void ExportTable_init();
    void MeteringProcessTable_init();
    void ObservationPointTable_init();
    void SelectionProcessTable_init();

};

#endif //IPFIXCOL2_MIB_H
