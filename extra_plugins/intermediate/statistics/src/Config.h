/**
 * \file Config.h
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Configuration of Statistics intermediate plugin (header file)
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

#ifndef STAT_CONFIG_H
#define STAT_CONFIG_H

#include <string>
#include <vector>
#include <ipfixcol2.h>

/** Default value of timeout for SNMP cache expiration                                                  */
#define SNMP_TIMEOUT_DEFAULT                1
/** Default value of timeout for session activity                                                       */
#define SESSION_ACIVITY_TIMEOUT_DEFAULT     10
/** Default value of timeout for refresh of text file                                                   */
#define TEXT_FILE_REFRESH_DEFAULT           1

/** Configuration for the SNMP output submodule                                                         */
struct cfg_snmp {
    struct {
        int64_t TransportSessionTable;
        int64_t TemplateTable;
        int64_t TemplateDefinitionTable;
        int64_t TransportSessionStatsTable;
        int64_t TemplateStatsTable;
    }timeouts; /**< Timeouts for SNMP cache expiration for all MIB tables                               */
};

struct cfg_text_file{
    bool        rewrite;
    uint64_t    refresh;
    std::string filename;
    struct{
        bool TransportSessionTable;
        bool TemplateTable;
        bool TemplateDefinitionTable;
        bool TransportSessionStatsTable;
        bool TemplateStatsTable;
    }tables;
};


/** Parsed configuration for statistics module and its output submodules                                */
class Config {
private:
    void default_set();
    void snmp_default_set();
    void text_file_default_set();

    void parse_text_file(fds_xml_ctx_t *text_file);
    void parse_timeout(fds_xml_ctx_t *timeout);
    void parse_snmp(fds_xml_ctx_t *snmp);
    void parse_outputs(fds_xml_ctx_t *outputs);
    void parse_params(fds_xml_ctx_t *params);
    void parse_custom_output(fds_xml_ctx_t *custom);

public:
    /** Timeout value. Sessions are marked as inactive if no data was transfered during this interval   */
    int64_t session_activity_timeout;

    struct {
        /** SNMP Output configuration                                                                   */
        struct cfg_snmp *snmp;
        /** Text File configuration                                                                     */
        std::vector<cfg_text_file> text_files;
    } outputs; /**< Outputs                                                                            */

    /**
     * \brief Create a new configuration
     * \param[in] params XML configuration of Statistics plugin
     * \throw runtime_error in case of invalid configuration
     */
    Config(const char *params);
    /**
     * \brief Configuration destructor
     */
    ~Config();
};

#endif // STAT_CONFIG_H
