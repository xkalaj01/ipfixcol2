/**
 * \file Config.cpp
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Configuration of Statistics intermediate plugin (source file)
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

#include <memory>
#include <set>

#include <libfds.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include "Config.h"

/** XML nodes */
enum params_xml_nodes {
    // Module properties
    SESSION_TIMEOUT,    /**< Timout for session activity                   */
    // Common output
    OUTPUT_LIST,       /**< List of output types                           */
    OUTPUT_SNMP,       /**< SNMP output                                    */
    // SNMP output
    SNMP_TIMEOUT,      /**< List of timeout values for cache expiration    */
    SNMP_TIMEOUT_TABLE,/**< Name of table for timeout                      */
    SNMP_TIMEOUT_VALUE,/**< Value of timeout                               */
};

/** Definition of the \<timeout\> node */
static const struct fds_xml_args args_timeout[] = {
        FDS_OPTS_ATTR(SNMP_TIMEOUT_TABLE, "table", FDS_OPTS_T_STRING, 0),
        FDS_OPTS_TEXT(SNMP_TIMEOUT_VALUE, FDS_OPTS_T_INT, 0),
        FDS_OPTS_END
};

/** Definition of the \<snmp\> node  */
static const struct fds_xml_args args_snmp[] = {
    FDS_OPTS_NESTED(SNMP_TIMEOUT, "cacheTimeout", args_timeout, FDS_OPTS_P_MULTI | FDS_OPTS_P_OPT),
    FDS_OPTS_END
};

/** Definition of the \<outputs\> node  */
static const struct fds_xml_args args_outputs[] = {
    FDS_OPTS_NESTED(OUTPUT_SNMP,  "snmp",  args_snmp,  FDS_OPTS_P_OPT),
    FDS_OPTS_END
};

/** Definition of the \<params\> node  */
static const struct fds_xml_args args_params[] = {
    FDS_OPTS_ROOT("params"),
    FDS_OPTS_NESTED(OUTPUT_LIST, "outputs",   args_outputs, 0),
    FDS_OPTS_ELEM(SESSION_TIMEOUT, "sessionActivityTimeout", FDS_OPTS_T_INT, FDS_OPTS_P_OPT),
    FDS_OPTS_END
};

/**
 * \brief Parse list of timeouts
 * \param[in] timeout Parsed XML context
 * \throw invalid_argument
 */
void
Config::parse_timeout(fds_xml_ctx_t *timeout)
{
    const struct fds_xml_cont *content;
    std::string table;
    int64_t *timeout_ptr = nullptr;
    while (fds_xml_next(timeout, &content)!= FDS_EOC) {
        switch (content->id) {
            case SNMP_TIMEOUT_TABLE:
                // Determine which table name is specified in argument
                table = content->ptr_string;
                if (table == "ipfixTransportSessionTable"){
                    timeout_ptr = &outputs.snmp->timeouts.TransportSessionTable;
                } else if (table == "ipfixTemplateTable"){
                    timeout_ptr = &outputs.snmp->timeouts.TemplateTable;
                } else if (table == "ipfixTemplateDefinitionTable"){
                    timeout_ptr = &outputs.snmp->timeouts.TemplateDefinitionTable;
                } else if (table == "ipfixTransportSessionStatsTable"){
                    timeout_ptr = &outputs.snmp->timeouts.TransportSessionStatsTable;
                } else if (table == "ipfixTemplateStatsTable"){
                    timeout_ptr = &outputs.snmp->timeouts.TemplateStatsTable;
                } else {
                    table.clear();
                    throw std::invalid_argument("Invalid name of the MIB table for cache timeout specified!");
                }
                break;
            case SNMP_TIMEOUT_VALUE:
                // Save value to correct table configuration
                if (!table.empty() && timeout_ptr != nullptr){
                    if (content->val_int <= 0 || content->val_int > INT64_MAX){
                        throw std::invalid_argument("Invalid timeout of cache for MIB table specified!");
                    }
                    *timeout_ptr = content->val_int;
                    table.clear();
                    timeout_ptr = nullptr;
                }
                break;
            default:
                throw std::invalid_argument("Unexpected element within <timeout>!");
        }
    }
}

/**
 * \brief Parse snmp node in XML
 * \param[in] snmp Parsed XML context
 * \throw invalid_argument
 */
void
Config::parse_snmp(fds_xml_ctx_t *snmp)
{
    const struct fds_xml_cont *content;
    while (fds_xml_next(snmp, &content) != FDS_EOC) {
        switch (content->id) {
        case SNMP_TIMEOUT:
            parse_timeout(content->ptr_ctx);
            break;
        default:
            throw std::invalid_argument("Unexpected element within <snmp>!");
        }
    }
}

/**
 * \brief Parse list of outputs
 * \param[in] outputs Parsed XML context
 * \throw invalid_argument
 */
void
Config::parse_outputs(fds_xml_ctx_t *outputs)
{
    const struct fds_xml_cont *content;
    while (fds_xml_next(outputs, &content) != FDS_EOC) {
        assert(content->type == FDS_OPTS_T_CONTEXT);
        switch (content->id) {
        case OUTPUT_SNMP:
            this->outputs.snmp = static_cast<cfg_snmp *>(calloc(1, sizeof(cfg_snmp)));
            snmp_default_set();
            parse_snmp(content->ptr_ctx);
            break;
        default:
            throw std::invalid_argument("Unexpected element within <outputs>!");
        }
    }
}


/**
 * \brief Parse all parameters
 *
 * This is the main parser function that process all format specifiers and parser all
 * specifications of outputs.
 * \param[in] params Initialized XML parser context of the root element
 * \throw invalid_argument
 */
void
Config::parse_params(fds_xml_ctx_t *params)
{
    const struct fds_xml_cont *content;
    while (fds_xml_next(params, &content) != FDS_EOC) {
        switch (content->id) {
        case OUTPUT_LIST: // List of output plugin
            assert(content->type == FDS_OPTS_T_CONTEXT);
            parse_outputs(content->ptr_ctx);
            break;
        case SESSION_TIMEOUT:
            assert(content->type == FDS_OPTS_T_INT);
            if (content->val_int <= 0 || content->val_int > INT64_MAX){
                throw std::invalid_argument("Invalid timeout for session activity specified!");
            }
            session_activity_timeout = content->val_int;
            break;
        default:
            throw std::invalid_argument("Unexpected element within <params>!");
        }
    }
}

/**
 * \brief Reset all parameters to default values
 */
void
Config::default_set()
{
    outputs.snmp = nullptr;
    session_activity_timeout = SESSION_ACIVITY_TIMEOUT_DEFAULT;
}

/**
 * \brief Reset all parameters of SNMP output submodule to default values
 */
void
Config::snmp_default_set() {
    outputs.snmp->timeouts.TransportSessionTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TemplateTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TemplateDefinitionTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TransportSessionStatsTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TemplateStatsTable = SNMP_TIMEOUT_DEFAULT;
}

Config::Config(const char *params)
{
    default_set();

    // Create XML parser
    static std::unique_ptr<fds_xml_t, decltype(&fds_xml_destroy)> xml(fds_xml_create(), &fds_xml_destroy);
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
    } catch (std::exception &ex) {
        throw std::runtime_error("Failed to parse the configuration: " + std::string(ex.what()));
    }
}

Config::~Config()
{
    free(outputs.snmp);
}


