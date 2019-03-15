//
// Created by root on 12.2.19.
//

#include <iostream>
#include "mib.h"
#include "../../../../../src/core/message_ipfix.h"

MIBBase::MIBBase() {
    // Initializing MainObjects
//    this->TransportSessionTable_init();
    this->TemplateTable_init();
    this->TemplateDefinitionTable_init();
    this->ExportTable_init();
    this->MeteringProcessTable_init();
    this->ObservationPointTable_init();
    this->SelectionProcessTable_init();
}

void MIBBase::registerMIB(uint *num, int *num2) {
    // Register table
    this->TransportSessionTable_init();
}

void MIBBase::TransportSessionTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Registering TransportSessionTable\n"));

    oid TransportSessionTable_oid[] = { 1, 3, 6, 1, 2, 1, 193, 1, 1, 1 };
    std::string name("ipfixTransportSessionTable");
    this->_MainObjects.ipfixTransportSessionTable_new = netsnmp_tdata_create_table(name.c_str(), 0);
    netsnmp_table_registration_info *table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
    netsnmp_table_helper_add_indexes( table_info, ASN_UNSIGNED, 0);
    table_info->min_column = 1;
    table_info->max_column = 15;
    netsnmp_handler_registration *reg = netsnmp_create_handler_registration(name.c_str(), TransportSessionTable_handler, TransportSessionTable_oid, OID_LENGTH(TransportSessionTable_oid), HANDLER_CAN_RONLY);
    netsnmp_tdata_register ( reg, this->_MainObjects.ipfixTransportSessionTable_new, table_info);

    DEBUGMSGTL(("IPFIX-MIB", "TransportSessionTable registered\n"));
}

void MIBBase::TemplateTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Initalizing TemplateTable\n"));
    this->_MainObjects.ipfixTemplateTable = netsnmp_create_table_data_set("ipfixTemplateTable");
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTemplateTable, ASN_UNSIGNED); //TransportSessionIndex
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTemplateTable, ASN_UNSIGNED); //TemplateODID
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTemplateTable, ASN_UNSIGNED); //TemplateID
    netsnmp_table_set_multi_add_default_row(this->_MainObjects.ipfixTemplateTable
            ,2,  ASN_UNSIGNED,   0, NULL, 0 // TemplateSetId
            ,2,  ASN_OCTET_STR,  0, NULL, 0 // TemplateAccessTime
            ,0 //Done
    );
    DEBUGMSGTL(("IPFIX-MIB", "TemplateTable initialized\n"));
}

void MIBBase::TemplateDefinitionTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Initalizing TemplateDefinitionTable\n"));
    this->_MainObjects.ipfixTemplateDefinitionTable = netsnmp_create_table_data_set("ipfixTemplateDefinitionTable");
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTemplateDefinitionTable, ASN_UNSIGNED); //TransportSessionIndex
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTemplateDefinitionTable, ASN_UNSIGNED); //TemplateODID
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTemplateDefinitionTable, ASN_UNSIGNED); //TemplateID
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTemplateDefinitionTable, ASN_UNSIGNED); //TemplateDefinitionIndex

    netsnmp_table_set_multi_add_default_row(this->_MainObjects.ipfixTemplateDefinitionTable
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // TemplateDefinitionIeId
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // TemplateDefinitionLength
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // TemplateDefinitionEnterpriseNumber
            ,2,  ASN_BIT8,  0, NULL, 0     // TemplateDefinitionFlags
            ,0 //Done
    );
    DEBUGMSGTL(("IPFIX-MIB", "TemplateDefinitionTable initialized\n"));
}

void MIBBase::ExportTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Initalizing ExportTable\n"));
    this->_MainObjects.ipfixExportTable = netsnmp_create_table_data_set("ipfixExportTable");
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixExportTable, ASN_UNSIGNED); //MeteringProcessCacheId
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixExportTable, ASN_UNSIGNED); //TransportSessionIndex
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixExportTable, ASN_UNSIGNED); //ExportIndex
    netsnmp_table_set_multi_add_default_row(this->_MainObjects.ipfixExportTable
            ,2,  ASN_INTEGER,  0, NULL, 0 // ExportMemberType
            ,0 //Done
    );
    DEBUGMSGTL(("IPFIX-MIB", "ExportTable initialized\n"));
}

void MIBBase::MeteringProcessTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Initalizing MeteringProcessTable\n"));
    this->_MainObjects.ipfixMeteringProcessTable = netsnmp_create_table_data_set("ipfixMeteringProcessTable");
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixMeteringProcessTable, ASN_UNSIGNED); //MeteringProcessCacheId
    netsnmp_table_set_multi_add_default_row(this->_MainObjects.ipfixMeteringProcessTable
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // MeteringProcessObservationPointGroupRef
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // MeteringProcessCacheActiveTimeout
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // MeteringProcessCacheIdleTimeout
            ,0 //Done
    );
    DEBUGMSGTL(("IPFIX-MIB", "MeteringProcessTable initialized\n"));
}

void MIBBase::ObservationPointTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Initalizing ObservationPointTable\n"));
    this->_MainObjects.ipfixObservationPointTable = netsnmp_create_table_data_set("ipfixObservationPointTable");
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixObservationPointTable, ASN_UNSIGNED); //ObservationPointGroupId
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixObservationPointTable, ASN_UNSIGNED); //ObservationPointIndex
    netsnmp_table_set_multi_add_default_row(this->_MainObjects.ipfixObservationPointTable
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // ObservationPointODID
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // ObservationPointPhysicalEntity
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // ObservationPointPhysicalInterface
            ,2,  ASN_UNSIGNED,  0, NULL, 0 // ObservationPointPhysicalEntityDirection
            ,0 //Done
    );
    DEBUGMSGTL(("IPFIX-MIB", "ObservationPointTable initialized\n"));
}

void MIBBase::SelectionProcessTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Initalizing SelectionProcessTable\n"));
    this->_MainObjects.ipfixSelectionProcessTable = netsnmp_create_table_data_set("ipfixSelectionProcessTable");
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixSelectionProcessTable, ASN_UNSIGNED); //MeteringProcessCacheId
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixSelectionProcessTable, ASN_UNSIGNED); //ProcessIndex
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixSelectionProcessTable, ASN_UNSIGNED); //ProcessSelectorIndex
    netsnmp_table_set_multi_add_default_row(this->_MainObjects.ipfixSelectionProcessTable
            ,2,  ASN_OBJECT_ID,  0, NULL, 0 // SelectionProcessSelectorFunction
            ,0 //Done
    );
    DEBUGMSGTL(("IPFIX-MIB", "SelectionProcessTable initialized\n"));
}

void MIBBase::TransportSessionTable_update(std::map<std::pair<std::string, int>, sessionTableEntry> *storage) {

    while (netsnmp_tdata_row_count(this->_MainObjects.ipfixTransportSessionTable_new) > 0){
        netsnmp_tdata_row *row = netsnmp_tdata_row_first(this->_MainObjects.ipfixTransportSessionTable_new);
        netsnmp_tdata_remove_and_delete_row(this->_MainObjects.ipfixTransportSessionTable_new, row);
    }

    for (auto const& s : *storage){
        const sessionTableEntry *e = &s.second;
        struct TransportSessionTable_entry *entry = SNMP_MALLOC_TYPEDEF( struct TransportSessionTable_entry);
        netsnmp_tdata_row *row = netsnmp_tdata_create_row();

        char addr[INET6_ADDRSTRLEN];

        entry->Index = e->id;
        entry->Protocol = 5;
        entry->SourceAddresstype = e->SourceAddressType;
        inet_ntop(e->SourceAddressType, &e->SourceAddress, addr, INET6_ADDRSTRLEN);
        strcpy(entry->SourceAddress, addr);
        entry->DestinationAddressType = e->DestAddressType;
        inet_ntop(e->DestAddressType, &e->DestAddress, addr, INET6_ADDRSTRLEN);
        strcpy(entry->DestinationAddress, addr);
        entry->SourcePort = e->SourcePort;
        entry->DestinationPort = e->DestPort;
        entry->SctpAssocId = 0;
        entry->DeviceMode = DEVICE_MODE_COLLECTING;
        entry->TemplateRefreshTimeout = e->TemplateRefreshTimeout;
        entry->OptionsTemplateRefreshTimeout = e->OptionsTemplateRefreshTimeout;
        entry->TemplateRefreshPacket = 0;
        entry->OptionsTemplateRefreshPacket = 0;
        entry->IpfixVersion = 2;
        entry->SessionStatus = 2;

        entry->valid = 1;
        row->data = entry;
        netsnmp_tdata_row_add_index(row, ASN_UNSIGNED, &(entry->Index), sizeof(entry->Index));
        netsnmp_tdata_add_row( this->_MainObjects.ipfixTransportSessionTable_new, row);

    }
}

int MIBBase::TransportSessionTable_handler(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo,
                                            netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests) {
    if ( reqinfo->mode != MODE_GET ) return SNMP_ERR_NOERROR;
    for ( netsnmp_request_info *request = requests; request; request = request->next )
    {
        struct TransportSessionTable_entry *table_entry  = (struct TransportSessionTable_entry*)netsnmp_tdata_extract_entry( request );
        netsnmp_table_request_info *table_info = netsnmp_extract_table_info( request );

        if ( table_entry == NULL ) { netsnmp_set_request_error( reqinfo, request, SNMP_NOSUCHINSTANCE); continue; }
        std::cout<<"TABLE REQUEST!"<<std::endl;
        switch ( table_info->colnum )
        {
            case 1:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->Index); break;
            case 2:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->Protocol ); break;
            case 3:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->SourceAddresstype ); break;
            case 4:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR, table_entry->SourceAddress, INET6_ADDRSTRLEN ); break;
            case 5:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->DestinationAddressType ); break;
            case 6:
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR, table_entry->DestinationAddress, INET6_ADDRSTRLEN); break;
            case 7:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->SourcePort ); break;
            case 8:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->DestinationPort ); break;
            case 9:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->SctpAssocId ); break;
            case 10:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->DeviceMode ); break;
            case 11:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->TemplateRefreshTimeout ); break;
            case 12:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->OptionsTemplateRefreshTimeout ); break;
            case 13:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->TemplateRefreshPacket ); break;
            case 14:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->OptionsTemplateRefreshPacket ); break;
            case 15:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->IpfixVersion ); break;
            case 16:
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED, table_entry->SessionStatus ); break;
                // ...
            default: netsnmp_set_request_error( reqinfo, request, SNMP_NOSUCHOBJECT );
        }
    }
    return SNMP_ERR_NOERROR;

}

