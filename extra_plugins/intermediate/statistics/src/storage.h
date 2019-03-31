//
// Created by root on 22.3.19.
//

#ifndef STATISTICS_STORAGE_H
#define STATISTICS_STORAGE_H

#include <netinet/in.h>
#include <libfds/template_mgr.h>
#include <map>
#include <ipfixcol2.h>
#include <vector>
#include <atomic>

/**
 * Lock to prevent data race on storage
 */
extern std::atomic_flag storage_lock;


struct TransportSessionEntry{
    uint32_t Index;
    uint32_t Protocol;
    uint32_t SourceAddressType;
    union {
        in_addr IPv4;
        in6_addr IPv6;
    }SourceAddress;
    uint32_t DestinationAddressType;
    union {
        in_addr IPv4;
        in6_addr IPv6;
    }DestinationAddress;
    uint32_t SourcePort;
    uint32_t DestinationPort;
    uint32_t SctpAssocId;
    uint32_t DeviceMode;
    uint32_t TemplateRefreshTimeout;
    uint32_t TemplateRefreshPacket;
    uint32_t OptionsTemplateRefreshTimeout;
    uint32_t OptionsTemplateRefreshPacket;
    uint32_t IpfixVersion;
    int Status;

    int valid;
};
typedef struct TransportSessionEntry                                TransportSessionEntry_t;
typedef uint32_t                                                    TransportSessionIndex_t;
typedef std::map<TransportSessionIndex_t, TransportSessionEntry_t>  TransportSessionTable_t;

struct TemplateEntry{
    uint32_t TransportSessionIndex;
    uint32_t ObservationDomainId;
    uint32_t Id;
    uint32_t SetId;
    uint32_t AccessTime;

    int valid;
};
typedef struct TemplateEntry                                TemplateEntry_t;
typedef std::tuple<TransportSessionIndex_t , uint, uint>    TemplateTableIndex_t;
typedef std::map<TemplateTableIndex_t, TemplateEntry_t>     TemplateTable_t;

struct TemplateDefinitionEntry{
    uint32_t TransportSessionIndex;
    uint32_t ObservationDomainId;
    uint32_t TemplateId;
    uint32_t Index;
    uint32_t Id;
    uint32_t Length;
    uint32_t EnterpriseNumber;
    uint16_t Flags;

    int valid;
};
typedef struct TemplateDefinitionEntry                                  TemplateDefinitionEntry_t;
typedef std::tuple<TemplateTableIndex_t, uint>                          TemplateDefinitionIndex_t;
typedef std::map<TemplateDefinitionIndex_t, TemplateDefinitionEntry_t>  TemplateDefinitionTable_t;

struct TransportSessionStatsEntry{
    uint32_t TransportSessionIndex;
    uint32_t Rate;
    uint64_t Packets;
    uint64_t Bytes;
    uint64_t Messages;
    uint64_t DiscardedMessages;
    uint64_t Records;
    uint64_t Templates;
    uint64_t OptionsTemplates;
    time_t DiscontinuityTime;

    int valid;
};
typedef struct TransportSessionStatsEntry                                       TransportSessionStatsEntry_t;
typedef TransportSessionIndex_t                                                 TransportSessionStatsIndex_t;
typedef std::map<TransportSessionStatsIndex_t, TransportSessionStatsEntry_t>    TransportSessionStatsTable_t;

struct TemplateStatsEntry{
    uint32_t TransportSessionIndex;
    uint32_t ObservationDomainId;
    uint32_t TemplateId;
    uint64_t DataRecords;
    time_t DiscontinuityTime;

    int valid;
};

typedef struct TemplateStatsEntry                               TemplateStatsEntry_t;
typedef TemplateTableIndex_t                                    TemplateStatsIndex_t;
typedef std::map<TemplateStatsIndex_t, TemplateStatsEntry_t>    TemplateStatsTable_t;

struct ExporterInfo{
    uint32_t TransportSessionTableId;
    uint64_t RateCounter;
};
/**
 * Mode of operation of this device, which is collector
 */
#define IPX_DEVICEMODE_COLLECTING   2
/**
 * Defines status of transport session
 */
#define IPX_SESSIONSTATUS_INACTIVE   1
#define IPX_SESSIONSTATUS_ACTIVE     2

class Storage {
public:
    Storage();

    // Processing messages
    int process_message(ipx_msg_t *msg);

    // MIB Tables
    TransportSessionTable_t         TransportSessionTable;
    TemplateTable_t                 TemplateTable;
    TemplateDefinitionTable_t       TemplateDefinitionTable;
    TransportSessionStatsTable_t    TransportSessionStatsTable;
    TemplateStatsTable_t            TemplateStatsTable;

    /**
     * Mapper of string identificators of session to internal info structure about exporters
     */
    std::map<std::string, struct ExporterInfo> active_exporters;

    /**
     * Mapper of previously connected exporters string id to their ID which they had while they were connected
     */
    std::map<std::string, uint> previously_connected_exporters;

private:
    /**
     * Counter of all sessions that collector has with exporters
     */
    uint32_t exporter_counter;

    int process_ipfix_msg(ipx_msg_ipfix_t *msg);
    int process_session_msg(ipx_msg_session_t *msg);

    void check_and_set_discontinuity(uint64_t counter, uint64_t increment, time_t *discont_flag);

};


#endif //STATISTICS_STORAGE_H
