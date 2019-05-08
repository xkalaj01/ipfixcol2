/*
 * Note: this file originally auto-generated by mib2c
 * using mib2c.table_data.conf
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <iostream>
#include "ipfixTemplateDefinitionTable.h"

/** Initialize the ipfixTemplateDefinitionTable table by defining its contents and how it's structured */
struct ipfixTemplateDefinition_cache_data *
initialize_table_ipfixTemplateDefinitionTable(TemplateDefinitionTable_t *storage, uint data_timeout)
{
    const oid ipfixTemplateDefinitionTable_oid[] = {1,3,6,1,2,1,193,1,1,3};
    const size_t ipfixTemplateDefinitionTable_oid_len   = OID_LENGTH(ipfixTemplateDefinitionTable_oid);
    netsnmp_handler_registration                *reg;
    netsnmp_tdata                               *table_data;
    netsnmp_cache                               *cache;
    struct ipfixTemplateDefinition_cache_data   *cache_data;

    DEBUGMSGTL(("ipfixTemplateDefinitionTable:init", "initializing table ipfixTemplateDefinitionTable\n"));

    reg = netsnmp_create_handler_registration(
              "ipfixTemplateDefinitionTable",     ipfixTemplateDefinitionTable_handler,
              ipfixTemplateDefinitionTable_oid, ipfixTemplateDefinitionTable_oid_len,
              HANDLER_CAN_RONLY
              );

    table_data = netsnmp_tdata_create_table( "ipfixTemplateDefinitionTable", 0 );
    if (NULL == table_data) {
        snmp_log(LOG_ERR,"error creating tdata table for ipfixTemplateDefinitionTable\n");
        return NULL;
    }
    cache = netsnmp_cache_create(data_timeout,
                                  ipfixTemplateDefinitionTable_load, ipfixTemplateDefinitionTable_free,
                                  ipfixTemplateDefinitionTable_oid, ipfixTemplateDefinitionTable_oid_len);

    // Creating data for cache to read while reloading MIB
    cache_data = static_cast<ipfixTemplateDefinition_cache_data *>(malloc(sizeof(ipfixTemplateDefinition_cache_data)));
    cache_data->table = table_data;
    cache_data->storage = storage;

    if (NULL == cache) {
        snmp_log(LOG_ERR,"error creating cache for ipfixTemplateDefinitionTable\n");
    }
    else{
        cache->flags = NETSNMP_CACHE_DONT_FREE_EXPIRED | NETSNMP_CACHE_AUTO_RELOAD;
        cache->magic = (void *)cache_data;
    }
    cache_data->table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    if (NULL == cache_data->table_info) {
        snmp_log(LOG_ERR,"error creating table info for ipfixTemplateDefinitionTable\n");
        return NULL;
    }
    netsnmp_table_helper_add_indexes(cache_data->table_info,
                           ASN_UNSIGNED,  /* index: ipfixTransportSessionIndex */
                           ASN_UNSIGNED,  /* index: ipfixTemplateObservationDomainId */
                           ASN_UNSIGNED,  /* index: ipfixTemplateId */
                           ASN_UNSIGNED,  /* index: ipfixTemplateDefinitionIndex */
                           0);

    cache_data->table_info->min_column = COLUMN_IPFIXTEMPLATEDEFINITION_IEID;
    cache_data->table_info->max_column = COLUMN_IPFIXTEMPLATEDEFINITION_FLAGS;
    
    netsnmp_tdata_register( reg, table_data, cache_data->table_info );
    if (cache) 
        netsnmp_inject_handler_before( reg, netsnmp_cache_handler_get(cache),
                                       TABLE_TDATA_NAME);
    /* Initialise the contents of the table here */
    cache_data->cache = cache;
    return cache_data;
}

/* create a new row in the table */
netsnmp_tdata_row *
ipfixTemplateDefinitionTable_createEntry(netsnmp_tdata *table_data
                 , uint32_t  ipfixTransportSessionIndex
                 , uint32_t  ipfixTemplateObservationDomainId
                 , uint32_t  ipfixTemplateId
                 , uint32_t  ipfixTemplateDefinitionIndex
                ) {
    TemplateDefinitionEntry_t *entry;
    netsnmp_tdata_row *row;

    entry = SNMP_MALLOC_TYPEDEF(TemplateDefinitionEntry_t);
    if (!entry)
        return NULL;

    row = netsnmp_tdata_create_row();
    if (!row) {
        SNMP_FREE(entry);
        return NULL;
    }
    row->data = entry;

    DEBUGMSGT(("ipfixTemplateDefinitionTable:entry:create", "row 0x%lu\n", (uintptr_t)row));
    entry->TransportSessionIndex = ipfixTransportSessionIndex;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->TransportSessionIndex),
                                 sizeof(entry->TransportSessionIndex));
    entry->ObservationDomainId = ipfixTemplateObservationDomainId;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->ObservationDomainId),
                                 sizeof(entry->ObservationDomainId));
    entry->TemplateId = ipfixTemplateId;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->TemplateId),
                                 sizeof(entry->TemplateId));
    entry->Index = ipfixTemplateDefinitionIndex;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->Index),
                                 sizeof(entry->Index));
    if (table_data)
        netsnmp_tdata_add_row( table_data, row );
    return row;
}

/* remove a row from the table */
void
ipfixTemplateDefinitionTable_removeEntry(netsnmp_tdata     *table_data, 
                 netsnmp_tdata_row *row) {
    TemplateDefinitionEntry_t *entry;

    if (!row)
        return;    /* Nothing to remove */

    DEBUGMSGT(("ipfixTemplateDefinitionTable:entry:remove", "row 0x%lu\n", (uintptr_t)row));

    entry = ( TemplateDefinitionEntry_t *)row->data;
    SNMP_FREE( entry );

    if (table_data)
        netsnmp_tdata_remove_and_delete_row( table_data, row );
    else
        netsnmp_tdata_delete_row( row );    
}

/* Example cache handling - set up table_data list from a suitable file */
int
ipfixTemplateDefinitionTable_load( netsnmp_cache *cache, void *vmagic ) {
    (void) cache;
    netsnmp_tdata               *table;
    netsnmp_tdata_row           *row;
    TemplateDefinitionEntry_t   *mib_row;
    ipfixTemplateDefinition_cache_data           *data;

    data = static_cast<ipfixTemplateDefinition_cache_data *>(vmagic);
    table = data->table;

    while(storage_lock.test_and_set(std::memory_order_acquire));
    for ( auto it : *data->storage) {

        row = ipfixTemplateDefinitionTable_createEntry(table
                , it.second.TransportSessionIndex
                , it.second.ObservationDomainId
                , it.second.TemplateId
                , it.second.Index
                        );
        if (row == NULL)
            continue;
        mib_row = (TemplateDefinitionEntry_t *)row->data;
        memcpy(mib_row, &it.second, sizeof(TemplateDefinitionEntry_t));
        mib_row->valid = 1;
    }
    storage_lock.clear(std::memory_order_release);
    return 0;  /* OK */
}

void
ipfixTemplateDefinitionTable_free( netsnmp_cache *cache, void *vmagic ) {
    (void) cache;
    ipfixTemplateDefinition_cache_data *data = (ipfixTemplateDefinition_cache_data*) vmagic;
    netsnmp_tdata     *table = data->table;
    netsnmp_tdata_row *row;

    while ((row = netsnmp_tdata_row_first(table))) {
        ipfixTemplateDefinitionTable_removeEntry(table, row);
    }
}

/** handles requests for the ipfixTemplateDefinitionTable table */
int
ipfixTemplateDefinitionTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    (void) handler;
    (void) reginfo;

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    TemplateDefinitionEntry_t  *table_entry;

    DEBUGMSGTL(("ipfixTemplateDefinitionTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (TemplateDefinitionEntry_t *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IPFIXTEMPLATEDEFINITION_IEID:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->Id);
                break;
            case COLUMN_IPFIXTEMPLATEDEFINITION_IELENGTH:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->Length);
                break;
            case COLUMN_IPFIXTEMPLATEDEFINITION_ENTERPRISENUMBER:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->EnterpriseNumber);
                break;
            case COLUMN_IPFIXTEMPLATEDEFINITION_FLAGS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          &table_entry->Flags,
                                          sizeof(table_entry->Flags));
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
