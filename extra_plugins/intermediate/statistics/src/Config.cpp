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
    OUTPUT_TEXT_FILE,   /**< Text file output                              */
    // SNMP output
    SNMP_TIMEOUT,      /**< List of timeout values for cache expiration    */
    SNMP_TIMEOUT_TABLE,/**< Name of table for timeout                      */
    SNMP_TIMEOUT_VALUE,/**< Value of timeout                               */
    // Text File output
    TEXT_FILE_REFRESH ,/**< Refresh time of output file                    */
    TEXT_FILE_FILENAME,/**< Name of output file                            */
    TEXT_FILE_CUSTOM,/**< Only user specified tables will be printed       */
    TEXT_FILE_TABLE_NAME, /**< Name of specified table                     */
    TEXT_FILE_REWRITE /**< Rewrite file (text is appended by default)      */
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

/** Definition of the \<custom\> node */
static const struct fds_xml_args args_custom[] = {
        FDS_OPTS_ELEM(TEXT_FILE_TABLE_NAME, "table", FDS_OPTS_T_STRING, FDS_OPTS_P_MULTI),
        FDS_OPTS_END
};
/** Definition of the \<text_file\> node */
static const struct fds_xml_args args_text_file[] = {
        FDS_OPTS_NESTED(TEXT_FILE_CUSTOM, "customOutput", args_custom, FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(TEXT_FILE_REFRESH, "refresh", FDS_OPTS_T_UINT, FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(TEXT_FILE_FILENAME, "filename", FDS_OPTS_T_STRING, FDS_OPTS_P_OPT),
        FDS_OPTS_ELEM(TEXT_FILE_REWRITE, "rewrite", FDS_OPTS_T_BOOL, 0),
        FDS_OPTS_END
};

/** Definition of the \<outputs\> node  */
static const struct fds_xml_args args_outputs[] = {
    FDS_OPTS_NESTED(OUTPUT_SNMP,  "snmp",  args_snmp,  FDS_OPTS_P_OPT),
    FDS_OPTS_NESTED(OUTPUT_TEXT_FILE, "textFile", args_text_file, FDS_OPTS_P_MULTI |FDS_OPTS_P_OPT),
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
 * \brief Parse text file node in XML
 * \param[in] text_file Parsed XML context
 * \throw invalid_argument
 */
void Config::parse_text_file(fds_xml_ctx_t *text_file) {
    const struct fds_xml_cont *content;
    cfg_text_file *cfg = &outputs.text_files.back();
    while (fds_xml_next(text_file, &content) != FDS_EOC) {
        switch (content->id) {
            case TEXT_FILE_REFRESH:
                cfg->refresh = content->val_uint;
                break;
            case TEXT_FILE_FILENAME:
                cfg->filename = content->ptr_string;
                break;
            case TEXT_FILE_CUSTOM:
                parse_custom_output(content->ptr_ctx);
                break;
            case TEXT_FILE_REWRITE:
                cfg->rewrite = content->val_bool;
                break;
            default:
                throw std::invalid_argument("Unexpected element within <textFile>!");
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
    cfg_text_file new_text_file_cfg = {};
    while (fds_xml_next(outputs, &content) != FDS_EOC) {
        assert(content->type == FDS_OPTS_T_CONTEXT);
        switch (content->id) {
        case OUTPUT_SNMP:
            this->outputs.snmp = static_cast<cfg_snmp *>(calloc(1, sizeof(cfg_snmp)));
            snmp_default_set();
            parse_snmp(content->ptr_ctx);
            break;
        case OUTPUT_TEXT_FILE:
            this->outputs.text_files.push_back(new_text_file_cfg);
            text_file_default_set();
            parse_text_file(content->ptr_ctx);
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
 * \brief Reset all parameters of SNMP output service to default values
 */
void
Config::snmp_default_set() {
    outputs.snmp->timeouts.TransportSessionTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TemplateTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TemplateDefinitionTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TransportSessionStatsTable = SNMP_TIMEOUT_DEFAULT;
    outputs.snmp->timeouts.TemplateStatsTable = SNMP_TIMEOUT_DEFAULT;
}

/**
 * \brief Reset all parameters of Text File output service to default values
 */
void Config::text_file_default_set() {
    if (!outputs.text_files.empty()) {
        cfg_text_file *cfg = &outputs.text_files.back();
        cfg->refresh = TEXT_FILE_REFRESH_DEFAULT;
        cfg->tables.TransportSessionTable = true;
        cfg->tables.TemplateTable = true;
        cfg->tables.TemplateDefinitionTable = true;
        cfg->tables.TransportSessionStatsTable = true;
        cfg->tables.TemplateStatsTable = true;
    }
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
    outputs.text_files.clear();
}

void Config::parse_custom_output(fds_xml_ctx_t *custom) {
    std::string table_name;

    // First set all to false and only true will be the ones specified
    cfg_text_file *cfg = &outputs.text_files.back();
    cfg->tables.TransportSessionTable = false;
    cfg->tables.TemplateTable = false;
    cfg->tables.TemplateDefinitionTable = false;
    cfg->tables.TransportSessionStatsTable = false;
    cfg->tables.TemplateStatsTable = false;

    const struct fds_xml_cont *content;
    while (fds_xml_next(custom, &content) != FDS_EOC) {
        switch (content->id) {
            case TEXT_FILE_TABLE_NAME:
                table_name = content->ptr_string;
                if (table_name == "ipfixTransportSessionTable"){
                    cfg->tables.TransportSessionTable = true;
                } else if (table_name == "ipfixTemplateTable"){
                    cfg->tables.TemplateTable = true;
                } else if (table_name == "ipfixTemplateDefinitionTable"){
                    cfg->tables.TemplateDefinitionTable = true;
                } else if (table_name == "ipfixTransportSessionStatsTable"){
                    cfg->tables.TransportSessionStatsTable = true;
                } else if (table_name == "ipfixTemplateStatsTable"){
                    cfg->tables.TemplateStatsTable = true;
                } else {
                    table_name.clear();
                    throw std::invalid_argument("Invalid name of the MIB table for custom text file output!");
                }
                break;
            default:
                throw std::invalid_argument("Unexpected element within <custom>!");
        }
    }
}




