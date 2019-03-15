#ifndef IPFIXCOL2_MIB_H
#define IPFIXCOL2_MIB_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/table_dataset.h>
#include <ipfixcol2.h>
#include "../interface.h"

struct sessionTableEntry;

struct TransportSessionTable_entry{
    uint32_t Index;
    uint32_t Protocol;
    uint32_t SourceAddresstype;
    char     SourceAddress [INET6_ADDRSTRLEN];
    uint32_t DestinationAddressType;
    char     DestinationAddress [INET6_ADDRSTRLEN];
    uint32_t SourcePort;
    uint32_t DestinationPort;
    uint32_t SctpAssocId;
    uint32_t DeviceMode;
    uint32_t TemplateRefreshTimeout;
    uint32_t OptionsTemplateRefreshTimeout;
    uint32_t TemplateRefreshPacket;
    uint32_t OptionsTemplateRefreshPacket;
    uint32_t IpfixVersion;
    uint32_t SessionStatus;

    int valid;
};

struct ipfixMainObjects{
    netsnmp_table_data_set *ipfixTransportSessionTable;
    netsnmp_tdata *ipfixTransportSessionTable_new;
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

    void TransportSessionTable_update(std::map<std::pair<std::string, int>, sessionTableEntry> *storage);
    static int TransportSessionTable_handler(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests);
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
