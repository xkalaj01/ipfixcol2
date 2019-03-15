//
// Created by root on 11.12.18.
//

#ifndef IPFIXCOL2_SERVICES_H
#define IPFIXCOL2_SERVICES_H

#include <atomic>
#include <thread>
#include "map.h"
#include "snmp/mib.h"
#include "service.h"

#define DEVICE_MODE_COLLECTING 2
#define TRANSPORT_SESSION_ACTIVE 2

struct sessionTableEntry{
    uint id;
    fds_session_type Protocol;
    uint SourceAddressType;
    union {
        in_addr IPv4;
        in6_addr IPv6;
    }SourceAddress;
    uint DestAddressType;
    union {
        in_addr DestIPv4;
        in6_addr DestIPv6;
    }DestAddress;
    uint SourcePort;
    uint DestPort;
    uint SctpAssocId;
    uint DeviceMode;
    uint TemplateRefreshTimeout;
    uint OptionsTemplateRefreshTimeout;
    uint IPFIXversion;
    uint Status;
};


class StatisticsInterface{
public:
    StatisticsInterface();

    virtual ~StatisticsInterface();

    void Start();
    void processPacket(ipx_msg_ipfix_t *msg);
    std::map<std::pair<std::string, int>, sessionTableEntry> storage;
    int counter = 0;
    bool kill_me = 0;
private:
    void Worker();
    uint exporter_counter;
    std::thread thread;
    std::vector<StatisticsService *> services;
};

#endif //IPFIXCOL2_SERVICES_H
