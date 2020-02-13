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
#include <librdkafka/rdkafka.h>

#include "Config.hpp"

/** XML nodes */
enum params_xml_nodes {

};

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

}

/**
 * \brief Reset all parameters to default values
 */
void
Config::default_set()
{
}

/**
 * \brief Check if parsed configuration is valid
 * \throw invalid_argument if the configuration is not valid
 */
void
Config::check_validity()
{
}

Config::Config(const char *params)
{
    default_set();
}

Config::~Config()
{
    // Nothing to do
}

int
parse_version(const std::string &str, int version[4])
{
    static const int FIELDS_MIN = 2;
    static const int FIELDS_MAX = 4;

    // Parse the required version
    std::istringstream parser(str);
    for (int i = 0; i < FIELDS_MAX; ++i) {
        version[i] = 0;
    }

    int idx;
    for (idx = 0; idx < FIELDS_MAX && !parser.eof(); idx++) {
        if (idx != 0 && parser.get() != '.') {
            return IPX_ERR_FORMAT;
        }

        parser >> version[idx];
        if (parser.fail() || version[idx] < 0) {
            return IPX_ERR_FORMAT;
        }
    }

    if (!parser.eof() || idx < FIELDS_MIN) {
        return IPX_ERR_FORMAT;
    }

    return IPX_OK;
}
