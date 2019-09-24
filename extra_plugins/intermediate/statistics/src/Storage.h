/**
 * \file Storage.h
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Storage for operational statistics based on IPFIX-MIB (header file)
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

#ifndef STATISTICS_STORAGE_H
#define STATISTICS_STORAGE_H

#include <netinet/in.h>
#include <libfds/template_mgr.h>
#include <map>
#include <ipfixcol2.h>
#include <vector>
#include <atomic>
#include <thread>
#include <condition_variable>
#include "Config.h"

/** Lock to prevent data race on storage    */
extern std::atomic_flag storage_lock;

/** Interval for updating information about session rate */
#define RATE_UPDATER_INTERVAL 1

/** Table entry for storing information about sessions between collector and exporter   */
struct TransportSessionEntry{
    /** Index of individual session                                                     */
    uint32_t Index;
    /** Transport protocol used in session (TCP/UDP/SCTP)                               */
    uint32_t Protocol;
    /** Type of IP address (IPv4/IPv6) of exporter - source                             */
    uint32_t SourceAddressType;
    /** IP address of exporter - source                                                 */
    union {
        in_addr IPv4;
        in6_addr IPv6;
    }SourceAddress;
    /** Type of IP address (IPv4/IPv6) of collector - destination                       */
    uint32_t DestinationAddressType;
    /** IP address of collector - destination                                           */
    union {
        in_addr IPv4;
        in6_addr IPv6;
    }DestinationAddress;
    /** Source port of exporter                                                         */
    uint32_t SourcePort;
    /** Destination port of collector                                                   */
    uint32_t DestinationPort;
    /** Association ID of SCTP session - Currently not implemented                      */
    uint32_t SctpAssocId;
    /** Mode of current device - collector in this case                                 */
    uint32_t DeviceMode;
    /** UDP specific timeout of template validity - ms                                  */
    uint32_t TemplateRefreshTimeout;
    /** UDP specific number of packets of template validity - not implemented           */
    uint32_t TemplateRefreshPacket;
    /** UDP specific timeout of Options template validity - ms                          */
    uint32_t OptionsTemplateRefreshTimeout;
    /** UDP specific number of packets of Options template validity - not implemented   */
    uint32_t OptionsTemplateRefreshPacket;
    /** Version of IPFIX protocol used in session                                       */
    uint32_t IpfixVersion;
    /** Status of session - active/inactive                                             */
    int Status;

    /** Validity field for SNMP library                                                 */
    int valid;
};

/** Data type of transport session table entry                                            */
typedef struct TransportSessionEntry                                TransportSessionEntry_t;
/** Data type of index to transport session table                                         */
typedef uint32_t                                                    TransportSessionIndex_t;
/** Data type of transport session table                                                  */
typedef std::map<TransportSessionIndex_t, TransportSessionEntry_t>  TransportSessionTable_t;

/** Table entry for stroing information about processed templates                       */
struct TemplateEntry{
    /** Index of session entry to which this entry belongs                              */
    uint32_t TransportSessionIndex;
    /** Observation domain ID for which the template was exported                       */
    uint32_t ObservationDomainId;
    /** ID of template                                                                  */
    uint32_t Id;
    /** ID of set which contains the template definition                                */
    uint32_t SetId;
    /** Time of last access to the template                                             */
    uint32_t AccessTime;

    /** Validity field for SNMP library                                                 */
    int valid;
};

/** Data type of template table entry                                   */
typedef struct TemplateEntry                                TemplateEntry_t;
/** Data type of template table index                                   */
typedef std::tuple<TransportSessionIndex_t , uint, uint>    TemplateIndex_t;
/** Data type of template table                                         */
typedef std::map<TemplateIndex_t, TemplateEntry_t>          TemplateTable_t;

/** Table entry for storing information about information elements included in templates    */
struct TemplateDefinitionEntry{
    /** Index of transport session to which the template belongs                            */
    uint32_t TransportSessionIndex;
    /** Observation domain ID for which was the template exported                           */
    uint32_t ObservationDomainId;
    /** ID of the template to which the definition of information element belongs           */
    uint32_t TemplateId;
    /** Index of the information element in template (order)                                */
    uint32_t Index;
    /** ID of information element                                                           */
    uint32_t Id;
    /** Length of the information element data in bytes                                     */
    uint32_t Length;
    /** Enterprise ID of the element                                                        */
    uint32_t EnterpriseNumber;
    /** Special attributes of element represented as bitmask                                */
    uint16_t Flags;

    /** Validity field for SNMP library                                                     */
    int valid;
};

/** Data type of definition table entry                                                     */
typedef struct TemplateDefinitionEntry                                  TemplateDefinitionEntry_t;
/** Data type of definition table index                                                     */
typedef std::tuple<TemplateIndex_t, uint>                               TemplateDefinitionIndex_t;
/** Data type of definition table                                                           */
typedef std::map<TemplateDefinitionIndex_t, TemplateDefinitionEntry_t>  TemplateDefinitionTable_t;

/** Table entry for storing statistics about active transport sessions                      */
struct TransportSessionStatsEntry{
    /** Index of transport session entry to which these statistics are measured             */
    uint32_t TransportSessionIndex;
    /** Number of bytes per second received                                                 */
    uint32_t Rate;
    /** Number of packets received                                                          */
    uint64_t Packets;
    /** Number of bytes received                                                            */
    uint64_t Bytes;
    /** Number of IPFIX messages received                                                   */
    uint64_t Messages;
    /** Number of discarded messages because of wrong format - currently not implemented    */
    uint64_t DiscardedMessages;
    /** Number of received records                                                          */
    uint64_t Records;
    /** Number of recieved templates                                                        */
    uint64_t Templates;
    /** Number of Options templates                                                         */
    uint64_t OptionsTemplates;
    /** Timestamp of event, when one of the above counters overflew                         */
    time_t DiscontinuityTime;

    /** Custom elements (Not based on RFC6615)                                              */
    /** Number of recieved records per second                                               */
    uint32_t RecordsRate;

    /** Validity field for SNMP library                                                     */
    int valid;
};

/** Data type of session stat table entry                                                   */
typedef struct TransportSessionStatsEntry                                       TransportSessionStatsEntry_t;
/** Data type of session stat table index                                                   */
typedef TransportSessionIndex_t                                                 TransportSessionStatsIndex_t;
/** Data type of session stat table                                                         */
typedef std::map<TransportSessionStatsIndex_t, TransportSessionStatsEntry_t>    TransportSessionStatsTable_t;

/** Table entry for storing statistics about templates                                      */
struct TemplateStatsEntry{
    /** Index of transport session entry to which the template belongs                      */
    uint32_t TransportSessionIndex;
    /** Observation domain ID for which the template was exported                           */
    uint32_t ObservationDomainId;
    /** ID of template                                                                      */
    uint32_t TemplateId;
    /** Number of records recieved per template                                             */
    uint64_t DataRecords;
    /** Timestamp of event when counter above overflew                                      */
    time_t DiscontinuityTime;

    /** Validity field for SNMP library                                                     */
    int valid;
};

/** Data type of template stat table entry                                      */
typedef struct TemplateStatsEntry                               TemplateStatsEntry_t;
/** Data type of template stat table index                                      */
typedef TemplateIndex_t                                         TemplateStatsIndex_t;
/** Data type of template stat table                                            */
typedef std::map<TemplateStatsIndex_t, TemplateStatsEntry_t>    TemplateStatsTable_t;

/** Table entry containing internal information for exporter identification
 *  This information is private to the module, thus is not recommended
 *  to use them outside of this module.                                         */
struct ExporterInfo{
    /** Mapped index to transport session table                                 */
    uint32_t TransportSessionTableId;
    /** Counter of recieved packets                                             */
    uint32_t RateCounter;
    /** Counter of recieved records                                             */
    uint32_t RecordRateCounter;
    /** Timestamp of last activity of session                                   */
    time_t   LastActive;
};

/** Mode of operation of this device, which is collector                        */
#define IPX_DEVICEMODE_COLLECTING    2
/** Defines status of inactive transport session                                */
#define IPX_SESSIONSTATUS_INACTIVE   1
/** Defines status of active transport session                                  */
#define IPX_SESSIONSTATUS_ACTIVE     2

/**
 * \brief The class representing storage for operational statistics
 */
class Storage {
public:
    /**
     * \brief Constructor
     * \param config    Pointer to configuration
     */
    Storage(Config *config);

    /**
     * \brief Destructor
     */
    ~Storage();

    /**
     * \brief Processing internal messages, both IPFIX and session messages.
     * Information from inside of the message are extracted and saved into
     * internal structures.
     *
     * \param msg Message to process
     * \return Status of message process by the functions process_ipfix_msg and process_session_msg
     */
    int process_message(ipx_msg_t *msg);


    /** Transport session table                         */
    TransportSessionTable_t         TransportSessionTable;
    /** Template table                                  */
    TemplateTable_t                 TemplateTable;
    /** Template definition table                       */
    TemplateDefinitionTable_t       TemplateDefinitionTable;
    /** Table for transport session statistics          */
    TransportSessionStatsTable_t    TransportSessionStatsTable;
    /** Table for template statistics                   */
    TemplateStatsTable_t            TemplateStatsTable;

    /**
     * Mapper of string identificators of session to internal info structure about exporters
     * identification is based on a string, which contains IP address and port numbers
     * */
    std::map<std::string, struct ExporterInfo> active_exporters;

    /**
     * Mapper of previously connected exporters string id to their ID which they
     * had while they were connected
     * */
    std::map<std::string, uint> previously_connected_exporters;

private:
    int process_ipfix_msg(ipx_msg_ipfix_t *msg);
    int process_session_msg(ipx_msg_session_t *msg);
    void check_and_set_discontinuity(uint64_t counter, uint64_t increment, time_t *discont_flag);
    std::string get_string_id(const ipx_session *session);
    void session_metering();

    /** Counter of all sessions that collector has with exporters           */
    uint32_t exporter_counter;
    /** Pointer to configuration class                                      */
    Config *config;
    /** Condition variable for controlling statistics threads               */
    std::condition_variable stat_threads_cv;
    /** Mutex for exclusive access to condition variable                    */
    std::mutex  stat_threads_mutex;
    /** Thread for updating rate in statistic table for transport sessions  */
    std::thread metering_thread;


};


#endif //STATISTICS_STORAGE_H
