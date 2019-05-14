/**
 * \file ipfixTransportSessionTable.cpp
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief MIB table functions needed by Net-SNMP library
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
/*
 * Note: this file originally auto-generated by mib2c
 * using mib2c.table_data.conf
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <iostream>
#include "ipfixTransportSessionTable.h"


/** Initialize the ipfixTransportSessionTable table by defining its contents and how it's structured */
ipfixTransportSession_cache_data *
initialize_table_ipfixTransportSessionTable(TransportSessionTable_t *storage,  uint data_timeout)
{
    const oid ipfixTransportSessionTable_oid[] = {1,3,6,1,2,1,193,1,1,1};
    const size_t ipfixTransportSessionTable_oid_len   = OID_LENGTH(ipfixTransportSessionTable_oid);
    netsnmp_handler_registration     *reg;
    netsnmp_tdata                    *table_data;
    netsnmp_cache                    *cache;
    ipfixTransportSession_cache_data *cache_data;

    DEBUGMSGTL(("ipfixTransportSessionTable:init", "initializing table ipfixTransportSessionTable\n"));

    reg = netsnmp_create_handler_registration(
              "ipfixTransportSessionTable",     ipfixTransportSessionTable_handler,
              ipfixTransportSessionTable_oid, ipfixTransportSessionTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_data = netsnmp_tdata_create_table( "ipfixTransportSessionTable", 0 );
    if (NULL == table_data) {
        snmp_log(LOG_ERR,"error creating tdata table for ipfixTransportSessionTable\n");
        return NULL;
    }
    cache = netsnmp_cache_create(data_timeout,
              ipfixTransportSessionTable_load, ipfixTransportSessionTable_free,
              ipfixTransportSessionTable_oid, ipfixTransportSessionTable_oid_len);

    // Creating data for cache to read while reloading MIB
    cache_data = static_cast<ipfixTransportSession_cache_data *>(malloc(sizeof(ipfixTransportSession_cache_data)));
    cache_data->table = table_data;
    cache_data->storage = storage;

    if (NULL == cache) {
        snmp_log(LOG_ERR,"error creating cache for ipfixTransportSessionTable\n");
    }
    else{
        cache->flags = NETSNMP_CACHE_DONT_FREE_EXPIRED | NETSNMP_CACHE_AUTO_RELOAD;
        cache->magic = (void *)cache_data;
    }
    cache_data->table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    if (NULL == cache_data->table_info) {
        snmp_log(LOG_ERR,"error creating table info for ipfixTransportSessionTable\n");
        return NULL;
    }
    netsnmp_table_helper_add_indexes(cache_data->table_info,
                           ASN_UNSIGNED,  /* index: ipfixTransportSessionIndex */
                           0);

    cache_data->table_info->min_column = COLUMN_IPFIXTRANSPORTSESSION_PROTOCOL;
    cache_data->table_info->max_column = COLUMN_IPFIXTRANSPORTSESSION_STATUS;
    
    netsnmp_tdata_register( reg, table_data, cache_data->table_info );
    if (cache)
        netsnmp_inject_handler_before( reg, netsnmp_cache_handler_get(cache),
                                       TABLE_TDATA_NAME);
    cache_data->cache = cache;
    return cache_data;
}

/* create a new row in the table */
netsnmp_tdata_row *
ipfixTransportSessionTable_createEntry(netsnmp_tdata *table_data
                 , uint32_t  ipfixTransportSessionIndex
                ) {
    TransportSessionEntry_t *entry;
    netsnmp_tdata_row *row;

    entry = SNMP_MALLOC_TYPEDEF(TransportSessionEntry_t);
    if (!entry)
        return NULL;

    row = netsnmp_tdata_create_row();
    if (!row) {
        SNMP_FREE(entry);
        return NULL;
    }
    row->data = entry;

    DEBUGMSGT(("ipfixTransportSessionTable:entry:create", "row 0x%lu\n", (uintptr_t)row));
    entry->Index = ipfixTransportSessionIndex;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->Index),
                                 sizeof(entry->Index));
    if (table_data)
        netsnmp_tdata_add_row( table_data, row );
    return row;
}

/* remove a row from the table */
void
ipfixTransportSessionTable_removeEntry(netsnmp_tdata     *table_data, 
                 netsnmp_tdata_row *row) {
    TransportSessionEntry_t *entry;

    if (!row)
        return;    /* Nothing to remove */

    DEBUGMSGT(("ipfixTransportSessionTable:entry:remove", "row 0x%lu\n", (uintptr_t)row));

    entry = (TransportSessionEntry_t *)row->data;
    SNMP_FREE( entry );   /* XXX - release any other internal resources */

    if (table_data)
        netsnmp_tdata_remove_and_delete_row( table_data, row );
    else
        netsnmp_tdata_delete_row( row );    
}

/* Example cache handling - set up table_data list from a suitable file */
int
ipfixTransportSessionTable_load( netsnmp_cache *cache, void *vmagic) {
    (void) cache;
    netsnmp_tdata     *table;
    netsnmp_tdata_row *row;
    TransportSessionEntry_t *mib_row;
    ipfixTransportSession_cache_data *data;

    data = static_cast<ipfixTransportSession_cache_data *>(vmagic);
    table = data->table;

    while(storage_lock.test_and_set(std::memory_order_acquire));
    // Recreate the table accordingly
    for ( auto it : *data->storage) {

        row = ipfixTransportSessionTable_createEntry(table
                         , it.first
                        );
        if (row == NULL)
            continue;
        mib_row = (TransportSessionEntry_t *)row->data;

        // copying values to internal MIB structure
        memcpy(mib_row, &it.second, sizeof(TransportSessionEntry_t));
        mib_row->valid = 1;
    }
    storage_lock.clear(std::memory_order_release);

    return 0;  /* OK */
}

void
ipfixTransportSessionTable_free( netsnmp_cache *cache, void *vmagic ) {
    (void) cache;
    ipfixTransportSession_cache_data *data = (ipfixTransportSession_cache_data *) vmagic;
    netsnmp_tdata     *table = data->table;
    netsnmp_tdata_row *mib_row;

    while ((mib_row = netsnmp_tdata_row_first(table))) {
        ipfixTransportSessionTable_removeEntry(table, mib_row);
    }
}

/** handles requests for the ipfixTransportSessionTable table */
int
ipfixTransportSessionTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    (void) handler;
    (void) reginfo;

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    TransportSessionEntry_t    *table_entry;


    DEBUGMSGTL(("ipfixTransportSessionTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (TransportSessionEntry_t *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IPFIXTRANSPORTSESSION_PROTOCOL:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->Protocol);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_SOURCEADDRESSTYPE:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                if (table_entry->SourceAddressType == AF_INET){
                    snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER, 1);
                } else {
                    snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER, 2);
                }
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_SOURCEADDRESS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                if (table_entry->SourceAddressType == AF_INET){
                    snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          &table_entry->SourceAddress.IPv4,
                                          sizeof(table_entry->SourceAddress.IPv4));
                } else {
                    snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                              &table_entry->SourceAddress.IPv6,
                                              sizeof(table_entry->SourceAddress.IPv6));
                }
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_DESTINATIONADDRESSTYPE:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                if (table_entry->DestinationAddressType == AF_INET){
                    snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER, 1);
                } else{
                    snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER, 2);
                }
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_DESTINATIONADDRESS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                if (table_entry->DestinationAddressType == AF_INET){
                    snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          &table_entry->DestinationAddress.IPv4,
                                          sizeof(table_entry->DestinationAddress.IPv4));
                } else{
                    snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                              &table_entry->DestinationAddress.IPv6,
                                              sizeof(table_entry->DestinationAddress.IPv6));
                }
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_SOURCEPORT:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->SourcePort);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_DESTINATIONPORT:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->DestinationPort);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_SCTPASSOCID:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->SctpAssocId);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_DEVICEMODE:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->DeviceMode);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_TEMPLATEREFRESHTIMEOUT:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->TemplateRefreshTimeout);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_OPTIONSTEMPLATEREFRESHTIMEOUT:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->OptionsTemplateRefreshTimeout);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_TEMPLATEREFRESHPACKET:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->TemplateRefreshPacket);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_OPTIONSTEMPLATEREFRESHPACKET:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->OptionsTemplateRefreshPacket);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_IPFIXVERSION:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->IpfixVersion);
                break;
            case COLUMN_IPFIXTRANSPORTSESSION_STATUS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->Status);
                break;
            default:
                netsnmp_set_request_error(reqinfo, request,
                                          SNMP_NOSUCHOBJECT);
                break;
            }
        }
        break;

    }
    return SNMP_ERR_NOERROR;
}
