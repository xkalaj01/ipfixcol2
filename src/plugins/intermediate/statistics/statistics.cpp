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

/** Plugin description */
IPX_API struct ipx_plugin_info ipx_plugin_info = {
        // Plugin type
        .type = IPX_PT_INTERMEDIATE,
        // Plugin identification name
        .name = "statistics",
        // Brief description of plugin
        .dsc = "Operational statistics module",
        // Configuration flags (reserved for future use)
        .flags = 0,
        // Plugin version string (like "1.2.3")
        .version = "1.0.0",
        // Minimal IPFIXcol version string (like "1.2.3")
        .ipx_min = "2.0.0"
};

int
ipx_plugin_init(ipx_ctx_t *ctx, const char *params)
{
    (void) ctx;
    (void) params;
    return IPX_OK;
}

void
ipx_plugin_destroy(ipx_ctx_t *ctx, void *cfg)
{
    (void) ctx;
    (void) cfg;
}

int
ipx_plugin_process(ipx_ctx_t *ctx, void *cfg, ipx_msg_t *msg)
{
    (void) cfg;

    ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);
    

    ipx_ctx_msg_pass(ctx, msg);
    return IPX_OK;
}