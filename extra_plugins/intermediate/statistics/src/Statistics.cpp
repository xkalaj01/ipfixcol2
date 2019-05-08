/**
 * \file Statistics.cpp
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Module for collecting and exporting operational statistics
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

#include <ipfixcol2.h>
#include <bits/unique_ptr.h>
#include "Interface.h"
#include <thread>
#include <iostream>
#include "../../../../src/core/message_ipfix.h"
#include "Config.h"

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

/** Statistics instance data                                */
struct Instance {
    /** Configuration for whole module                      */
    Config *config;
    /** Interface for output sub-modules management         */
    StatisticsInterface *interface;
    /** Storage of operational statistics                   */
    Storage *storage;
};

int
ipx_plugin_init(ipx_ctx_t *ctx, const char *params)
{
    (void) ctx;
    (void) params;

    struct Instance *data = nullptr;
    try{
        std::unique_ptr<Instance> ptr(new Instance);
        std::unique_ptr<Config> config(new Config(params));
        std::unique_ptr<Storage> storage(new Storage(config.get()));
        std::unique_ptr<StatisticsInterface> interface(new StatisticsInterface(storage.get(), config.get()));

        data = ptr.release();
        data->config = config.release();
        data->storage = storage.release();
        data->interface = interface.release();
    }
    catch (const std::exception &ex){
        IPX_CTX_ERROR(ctx, "Exception has occured in Statistics module - Init",'\0');
        IPX_CTX_ERROR(ctx, ex.what());
        return IPX_ERR_DENIED;
    }
    catch (...){
        IPX_CTX_ERROR(ctx, "Exception has occured in Statistics module - Init",'\0');
        return IPX_ERR_DENIED;
    }

    try {
        data->interface->Start();
    }
    catch (const std::exception &ex){
        IPX_CTX_ERROR(ctx, "Exception has occured in Statistics module - Services start",'\0');
        IPX_CTX_ERROR(ctx, ex.what());
        delete data->interface;
        delete data->config;
        delete data->storage;
        delete data;
        return IPX_ERR_DENIED;
    }


    // Subscribe to recieve session messages
    ipx_ctx_private_set(ctx, data);
    ipx_msg_mask_t mask = IPX_MSG_IPFIX|IPX_MSG_SESSION;
    ipx_ctx_subscribe(ctx, &mask, nullptr);

    return IPX_OK;
}

void
ipx_plugin_destroy(ipx_ctx_t *ctx, void *cfg)
{
    (void) ctx;
    struct Instance *data = reinterpret_cast<Instance *>(cfg);
    delete data->interface;
    delete data->config;
    delete data->storage;
    delete data;
}

int
ipx_plugin_process(ipx_ctx_t *ctx, void *cfg, ipx_msg_t *msg)
{
    struct Instance *data = reinterpret_cast<Instance *>(cfg);
    // Process IPFIX message in storage
    data->storage->process_message(msg);
    ipx_ctx_msg_pass(ctx, msg);

    return IPX_OK;
}