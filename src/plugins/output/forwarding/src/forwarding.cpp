/**
 * \file src/plugins/output/json/src/json.cpp
 * \author Lukas Hutak <lukas.hutak@cesnet.cz>
 * \brief JSON output plugin (source file)
 * \date 2018-2019
 */

/* Copyright (C) 2018-2019 CESNET, z.s.p.o.
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

#include <libfds.h>
#include <ipfixcol2.h>
#include <memory>

#include "Config.hpp"
#include "Forwarder.h"

/** Plugin description */
IPX_API struct ipx_plugin_info ipx_plugin_info = {
    // Plugin identification name
    "forwarding",
    // Brief description of plugin
    "Conversion of IPFIX data into JSON format",
    // Plugin type
    IPX_PT_OUTPUT,
    // Configuration flags (reserved for future use)
    0,
    // Plugin version string (like "1.2.3")
    "2.0.0",
    // Minimal IPFIXcol version string (like "1.2.3")
    "2.0.0"
};

/** Forwarding instance data                                                                           */
struct Instance {
    /** Parser configuration                                                                     */
    Config *config;
    /** Forwarder */
    Forwarder *forwarder;
};


int
ipx_plugin_init(ipx_ctx_t *ctx, const char *params)
{
    struct Instance *data = nullptr;
    try {
        // Create and parse the configuration
        std::unique_ptr<Instance> ptr(new Instance);
        std::unique_ptr<Config> cfg(new Config(params));

        // Success
        data = ptr.release();
        data->config = cfg.release();

        std::unique_ptr<Forwarder> frwdr(new Forwarder(data->config));
        data->forwarder = frwdr.release();

    } catch (std::exception &ex) {
        IPX_CTX_ERROR(ctx, "%s", ex.what());
        return IPX_ERR_DENIED;
    } catch (...) {
        IPX_CTX_ERROR(ctx, "Unexpected exception has occurred!", '\0');
        return IPX_ERR_DENIED;
    }

    ipx_ctx_private_set(ctx, data);

    // Subscribe to receive session messages
    ipx_ctx_private_set(ctx, data);
    ipx_msg_mask_t mask = IPX_MSG_IPFIX|IPX_MSG_SESSION;
    ipx_ctx_subscribe(ctx, &mask, nullptr);

    return IPX_OK;
}

void
ipx_plugin_destroy(ipx_ctx_t *ctx, void *cfg)
{
    (void) ctx;

    struct Instance *data = reinterpret_cast<struct Instance *>(cfg);
    delete data->config;
    delete data->forwarder;
    delete data;
}

int
ipx_plugin_process(ipx_ctx_t *ctx, void *cfg, ipx_msg_t *msg)
{

    struct Instance *data = reinterpret_cast<struct Instance *>(cfg);
    const fds_iemgr_t *iemgr = ipx_ctx_iemgr_get(ctx);
    ipx_msg_type msg_type = ipx_msg_get_type(msg);

    data->forwarder->processMsg(msg, iemgr);


    if (msg_type == IPX_MSG_IPFIX) {
        data->forwarder->forward(msg);
    }

    return IPX_OK;
}
