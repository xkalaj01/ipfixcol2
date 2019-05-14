/**
 * \file ipfixTemplateStatsTable.h
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
#ifndef IPFIXTEMPLATESTATSTABLE_H
#define IPFIXTEMPLATESTATSTABLE_H

#include "../../Storage.h"

struct ipfixTemplateStats_cache_data{
    netsnmp_tdata                         *table;
    TemplateStatsTable_t                  *storage;
    netsnmp_table_registration_info       *table_info;
    netsnmp_cache                         *cache;
};
/* function declarations */
struct ipfixTemplateStats_cache_data *
initialize_table_ipfixTemplateStatsTable(TemplateStatsTable_t *storage, uint data_timeout);

Netsnmp_Node_Handler ipfixTemplateStatsTable_handler;
NetsnmpCacheLoad ipfixTemplateStatsTable_load;
NetsnmpCacheFree ipfixTemplateStatsTable_free;

/* column number definitions for table ipfixTemplateStatsTable */
       #define COLUMN_IPFIXTEMPLATE_DATARECORDS		    1
       #define COLUMN_IPFIXTEMPLATE_DISCONTINUITYTIME	2
#endif /* IPFIXTEMPLATESTATSTABLE_H */
