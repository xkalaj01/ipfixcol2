/*
 * Note: this file originally auto-generated by mib2c
 * using mib2c.table_data.conf
 */
#ifndef IPFIXTRANSPORTSESSIONSTATSTABLE_H
#define IPFIXTRANSPORTSESSIONSTATSTABLE_H
#include "../../storage.h"

/* function declarations */
void initialize_table_ipfixTransportSessionStatsTable(TransportSessionStatsTable_t *storage, uint data_timeout);
Netsnmp_Node_Handler ipfixTransportSessionStatsTable_handler;
NetsnmpCacheLoad ipfixTransportSessionStatsTable_load;
NetsnmpCacheFree ipfixTransportSessionStatsTable_free;

/* column number definitions for table ipfixTransportSessionStatsTable */
       #define COLUMN_IPFIXTRANSPORTSESSION_RATE		       1
       #define COLUMN_IPFIXTRANSPORTSESSION_PACKETS		       2
       #define COLUMN_IPFIXTRANSPORTSESSION_BYTES		       3
       #define COLUMN_IPFIXTRANSPORTSESSION_MESSAGES		   4
       #define COLUMN_IPFIXTRANSPORTSESSION_DISCARDEDMESSAGES  5
       #define COLUMN_IPFIXTRANSPORTSESSION_RECORDS		       6
       #define COLUMN_IPFIXTRANSPORTSESSION_TEMPLATES		   7
       #define COLUMN_IPFIXTRANSPORTSESSION_OPTIONSTEMPLATES   8
       #define COLUMN_IPFIXTRANSPORTSESSION_DISCONTINUITYTIME  9
#endif /* IPFIXTRANSPORTSESSIONSTATSTABLE_H */
