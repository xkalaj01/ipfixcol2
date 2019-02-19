//
// Created by root on 12.2.19.
//

#include "mib.h"
#include "../../../../../src/core/message_ipfix.h"

MIBBase::MIBBase() {
    // Initializing MainObjects
    this->TransportSessionTable_init();
    this->TemplateTable_init();
    this->TemplateDefinitionTable_init();
    this->ExportTable_init();
    this->MeteringProcessTable_init();
    this->ObservationPointTable_init();
    this->SelectionProcessTable_init();
}

void MIBBase::registerMIB(uint *num, int *num2) {
    // Register table
    DEBUGMSGTL(("IPFIX-MIB", "Registering TransportSessionTable\n"));
    oid TransportSessionTable_oid[] = { 1, 3, 6, 1, 2, 1, 193, 1, 1, 1 };
    netsnmp_register_table_data_set(
            netsnmp_create_handler_registration
                    ("ipfixTransportSessionTable",NULL,TransportSessionTable_oid,
                     OID_LENGTH(TransportSessionTable_oid),HANDLER_CAN_RWRITE)
            ,this->_MainObjects.ipfixTransportSessionTable, NULL);

    // Some example data to add to table
    netsnmp_table_row *row;
    row = netsnmp_create_table_data_row();

    //Index first
    netsnmp_table_row_add_index(row, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    //Rest of the data
    netsnmp_set_row_column(row, 2, ASN_UNSIGNED,  (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 3, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 4, ASN_OCTET_STR, "fe80::d703:750f:c8bd:a906", strlen("fe80::d703:750f:c8bd:a906"));
    netsnmp_set_row_column(row, 5, ASN_UNSIGNED,  (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 6, ASN_OCTET_STR, "10.10.10.10", strlen("10.10.10.10"));
    netsnmp_set_row_column(row, 7, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 8, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 9, ASN_INTEGER, (const u_char *)num2, sizeof(int));
    netsnmp_set_row_column(row, 10, ASN_INTEGER, (const u_char *)num2, sizeof(int));
    netsnmp_set_row_column(row, 11, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 12, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 13, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 14, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 15, ASN_UNSIGNED, (const u_char *)num, sizeof(uint));
    netsnmp_set_row_column(row, 16, ASN_INTEGER, (const u_char *)num2, sizeof(int));
    netsnmp_table_dataset_add_row(this->_MainObjects.ipfixTransportSessionTable, row);

    netsnmp_register_auto_data_table(this->_MainObjects.ipfixTransportSessionTable, NULL);

    DEBUGMSGTL(("IPFIX-MIB", "TransportSessionTable registered\n"));
}

void MIBBase::TransportSessionTable_init() {
    DEBUGMSGTL(("IPFIX-MIB", "Initalizing TransportSessionTable\n"));
    this->_MainObjects.ipfixTransportSessionTable = netsnmp_create_table_data_set("ipfixTransportSessionTable");
    netsnmp_table_dataset_add_index(this->_MainObjects.ipfixTransportSessionTable, ASN_UNSIGNED); //TransportSessionIndex
    netsnmp_table_set_multi_add_default_row(this->_MainObjects.ipfixTransportSessionTable
            ,2,  ASN_UNSIGNED,  0, NULL, 0 //Protocol
            ,3,  ASN_UNSIGNED,  0, NULL, 0 //SourceAddressType
            ,4,  ASN_OCTET_STR, 0, NULL, 0 //SourceAddress
            ,5,  ASN_UNSIGNED,  0, NULL, 0 //DestinationAddressType
            ,6,  ASN_OCTET_STR, 0, NULL, 0 //DestinationAddress
            ,7,  ASN_UNSIGNED,  0, NULL, 0 //SourcePort
            ,8,  ASN_UNSIGNED,  0, NULL, 0 //DestinationPort
            ,9,  ASN_INTEGER,   0, NULL, 0 //SctpAssocId
            ,10, ASN_INTEGER,   0, NULL, 0 //DeviceMode
            ,11, ASN_UNSIGNED,  0, NULL, 0 //TemplateRefreshTimeout
            ,12, ASN_UNSIGNED,  0, NULL, 0 //OptionsTemplateRefreshTimeout
            ,13, ASN_UNSIGNED,  0, NULL, 0 //TemplateRefreshPacket
            ,14, ASN_UNSIGNED,  0, NULL, 0 //OptionsTemplateRefreshPacket
            ,15, ASN_UNSIGNED,  0, NULL, 0 //IpfixVersion
            ,16, ASN_INTEGER,   0, NULL, 0 //SessionStatus
            ,0 //Done
    );
    DEBUGMSGTL(("IPFIX-MIB", "TransportSessionTable initialized\n"));
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

void MIBBase::processPacket(ipx_msg_ipfix_t *ipfix_msg) {
    (void) ipfix_msg;
}

