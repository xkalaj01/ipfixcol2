/**
 * \file src/plugins/intermediate/statistics.cpp
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Module for collecting and shaaring operational statistics
 * \date 2018
 */

/* Copyright (C) 2018 CESNET, z.s.p.o.
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

#include <ipfixcol2.h>
#include <bits/unique_ptr.h>
#include "map.h"
#include "services.h"
#include <thread>
//#include <ipfixcol2/message_ipfix.h>
#include "../../../../src/core/message_ipfix.h"
#include "snmp/mib.h"

/** Plugin description */
IPX_API struct ipx_plugin_info ipx_plugin_info = {
        // Plugin identification name
        "statistics",
        // Brief description of plugin
        "Operational statistics module",
        // Plugin type
        IPX_PT_INTERMEDIATE,
        // Configuration flags (reserved for future use)
        0,
        // Plugin version string (like "1.2.3")
        "1.0.0",
        // Minimal IPFIXcol version string (like "1.2.3")
        "2.0.0"
};


int
ipx_plugin_init(ipx_ctx_t *ctx, const char *params)
{
    (void) ctx;
    (void) params;

    struct Instance *data = nullptr;
    try{
        std::unique_ptr<Instance> ptr(new Instance);
        std::unique_ptr<StatMap> map(new StatMap);
        std::unique_ptr<MIBBase> mibClass(new MIBBase);

        data = ptr.release();
        data->stats = map.release();
        data->snmp_kill = 0;
        data->int_var = 0;
        data->uns_var = 0;
        data->mib = mibClass.release();
        data->stats->pkt_counter=0;

        std::unique_ptr<std::thread> s(new std::thread(snmp_agent, data));
        data->services.push_back(s.release());
    }
    catch (...){
        IPX_CTX_ERROR(ctx, "Exception has occured in Statistics module - Init",'\0');
        return IPX_ERR_DENIED;
    }

    ipx_ctx_private_set(ctx, data);

    return IPX_OK;
}

void
ipx_plugin_destroy(ipx_ctx_t *ctx, void *cfg)
{
    (void) ctx;

    struct Instance *data = reinterpret_cast<Instance *>(cfg);
    delete data->stats;
    data->snmp_kill = 1;

    std::vector<std::thread*>::iterator it;
    for (it = data->services.begin(); it != data->services.end(); it++){
        (*(*it)).join();
        delete *it;
    }
    delete data;
}

int
ipx_plugin_process(ipx_ctx_t *ctx, void *cfg, ipx_msg_t *msg)
{

    struct Instance *data = reinterpret_cast<Instance *>(cfg);
    ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);

    while(data->mib_lock.test_and_set(std::memory_order_acquire));
    data->mib->processPacket(ipfix_msg);
    data->int_var++;
    data->uns_var++;
    printf("IPFIX Packet processed [%d]\n",data->int_var);
    data->mib_lock.clear(std::memory_order_release);

    data->stats->pkt_counter++;

    ipx_ctx_msg_pass(ctx, msg);
    return IPX_OK;
}