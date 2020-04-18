/**
 * \file src/plugins/output/json/src/Config.cpp
 * \author Lukas Hutak <lukas.hutak@cesnet.cz>
 * \brief Configuration of JSON output plugin (source file)
 * \date 2018-2020
 */

/* Copyright (C) 2018-2020 CESNET, z.s.p.o.
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

#include <cstdio>
#include <memory>
#include <set>
#include <sstream>

#include <libfds.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include "Config.hpp"

/** XML nodes */
enum params_xml_nodes {
    GLOBAL_PROTO,
    GLOBAL_MODE,
    GLOBAL_MTU,
    GLOBAL_CHECK_RATE,

    HOSTS,
    HOST,
    HOST_NAME,
    HOST_IPV4,
    HOST_PORT,

};

static const struct fds_xml_args args_host[] = {
        FDS_OPTS_ELEM(HOST_NAME, "name", FDS_OPTS_T_STRING, 0),
        FDS_OPTS_ELEM(HOST_IPV4, "ipv4", FDS_OPTS_T_STRING, 0),
        FDS_OPTS_ELEM(HOST_PORT, "port", FDS_OPTS_T_STRING, 0),
        FDS_OPTS_END
};

static const struct fds_xml_args args_hosts[] = {
        FDS_OPTS_NESTED(HOST, "host", args_host, FDS_OPTS_P_MULTI),
        FDS_OPTS_END
};

static const struct fds_xml_args args_params[] = {
        FDS_OPTS_ROOT("params"),
        FDS_OPTS_ELEM(GLOBAL_PROTO,      "protocol",   FDS_OPTS_T_STRING,  FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(GLOBAL_MODE,       "mode",       FDS_OPTS_T_STRING,  FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(GLOBAL_MTU,        "mtu",        FDS_OPTS_T_UINT,    FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(GLOBAL_CHECK_RATE, "check_rate", FDS_OPTS_T_UINT,    FDS_OPTS_P_OPT),
        FDS_OPTS_NESTED(HOSTS, "hosts", args_hosts, 0),
        FDS_OPTS_END
};

/**
 * \brief Parse <host> node
 *
 * \param[in] hosts Initialized XML parser context of the root element
 * \throw invalid_argument or runtime_error
 */
void
Config::parse_host(fds_xml_ctx_t *host)
{
    const struct fds_xml_cont *content;
    cfg_host new_host;
    while (fds_xml_next(host, &content) != FDS_EOC) {
        switch (content->id) {
            case (HOST_NAME):
                assert(content->type == FDS_OPTS_T_STRING);
                new_host.hostname = content->ptr_string;
                break;
            case (HOST_IPV4):
                assert(content->type == FDS_OPTS_T_STRING);
                new_host.addr = content->ptr_string;
                break;
            case (HOST_PORT):
                assert(content->type == FDS_OPTS_T_STRING);
                new_host.port = content->ptr_string;
                break;

            default:
                throw std::invalid_argument("Unexpected element within <host>!");
        }
    }
    hosts.push_back(new_host);
}

/**
 * \brief Parse <hosts> node
 *
 * \param[in] hosts Initialized XML parser context of the root element
 * \throw invalid_argument or runtime_error
 */
void
Config::parse_hosts(fds_xml_ctx_t *hosts)
{
    const struct fds_xml_cont *content;
    while (fds_xml_next(hosts, &content) != FDS_EOC) {
        switch (content->id) {
            case (HOST):
                parse_host(content->ptr_ctx);
                break;
            default:
                throw std::invalid_argument("Unexpected element within <hosts>!");
        }
    }
}

/**
 * \brief Parse all parameters
 *
 * This is the main parser function that process all format specifiers and parser all
 * specifications of outputs.
 * \param[in] params Initialized XML parser context of the root element
 * \throw invalid_argument or runtime_error
 */
void
Config::parse_params(fds_xml_ctx_t *params)
{
    const struct fds_xml_cont *content;
    while (fds_xml_next(params, &content) != FDS_EOC) {
        switch (content->id) {

            case GLOBAL_PROTO:
                assert(content->type == FDS_OPTS_T_STRING);
                options.proto = check_or("protocol", content->ptr_string, "UDP", "TCP")
                                ? IPPROTO_UDP : IPPROTO_TCP;
                break;
            case GLOBAL_MODE:
                assert(content->type == FDS_OPTS_T_STRING);
                if (strcasecmp(content->ptr_string, "round robin") == 0) {
                    options.oper_mode = cfg_options::SEND_MODE_ROUND_ROBIN;
                } else if (strcasecmp(content->ptr_string, "all") == 0) {
                    options.oper_mode = cfg_options::SEND_MODE_ALL;
                } else {
                    throw std::invalid_argument("Unsupported operational mode!");
                }
                break;
            case GLOBAL_MTU:
                assert(content->type == FDS_OPTS_T_UINT);
                options.mtu_size = (uint16_t ) content->val_uint;
                break;
            case GLOBAL_CHECK_RATE:
                assert(content->type == FDS_OPTS_T_UINT);
                options.check_rate = (uint16_t) content->val_uint;
                break;
            case HOSTS:
                parse_hosts(content->ptr_ctx);
                break;
            default:
                throw std::invalid_argument("Unexpected element within <params>!");
        }
    }
}

/**
 * \brief Check if a given string is a valid IPv4/IPv6 address
 * \param[in] ip_addr Address to check
 * \return True or false
 */
bool
Config::check_ip(const std::string &ip_addr)
{
    in_addr ipv4;
    in6_addr ipv6;

    return (inet_pton(AF_INET, ip_addr.c_str(), &ipv4) == 1
            || inet_pton(AF_INET6, ip_addr.c_str(), &ipv6) == 1);
}

/**
 * \brief Check one of 2 expected options
 *
 * \param[in] elem      XML element to check (just for exception purposes)
 * \param[in] value     String to check
 * \param[in] val_true  True string
 * \param[in] val_false False string
 * \throw invalid_argument if the value doesn't match any expected string
 * \return True of false
 */
bool
Config::check_or(const std::string &elem, const char *value, const std::string &val_true,
                 const std::string &val_false)
{
    if (strcasecmp(value, val_true.c_str()) == 0) {
        return true;
    }

    if (strcasecmp(value, val_false.c_str()) == 0) {
        return false;
    }

    // Error
    throw std::invalid_argument("Unexpected parameter of the element <" + elem + "> (expected '"
                                + val_true + "' or '" + val_false + "')");
}

/**
 * \brief Reset all parameters to default values
 */
void
Config::default_set()
{
    options.check_rate = 0;
    options.mtu_size = 1500;
    options.proto = IPPROTO_TCP;
    options.oper_mode = options.SEND_MODE_ALL;
}

/**
 * \brief Check if parsed configuration is valid
 * \throw invalid_argument if the configuration is not valid
 */
void
Config::check_validity()
{
    //TODO
}

Config::Config(const char *params)
{
    default_set();

    // Create XML parser
    std::unique_ptr<fds_xml_t, decltype(&fds_xml_destroy)> xml(fds_xml_create(), &fds_xml_destroy);
    if (!xml) {
        throw std::runtime_error("Failed to create an XML parser!");
    }

    if (fds_xml_set_args(xml.get(), args_params) != FDS_OK) {
        throw std::runtime_error("Failed to parse the description of an XML document!");
    }

    fds_xml_ctx_t *params_ctx = fds_xml_parse_mem(xml.get(), params, true);
    if (!params_ctx) {
        std::string err = fds_xml_last_err(xml.get());
        throw std::runtime_error("Failed to parse the configuration: " + err);
    }

    // Parse parameters and check configuration
    try {
        parse_params(params_ctx);
        check_validity();
    } catch (std::exception &ex) {
        throw std::runtime_error("Failed to parse the configuration: " + std::string(ex.what()));
    }
}

Config::~Config()
{
    // Nothing to do
}
