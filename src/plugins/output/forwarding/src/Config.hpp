/**
 * \file src/plugins/output/json/src/Config.hpp
 * \author Lukas Hutak <lukas.hutak@cesnet.cz>
 * \brief Configuration of JSON output plugin (header file)
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

#ifndef JSON_CONFIG_H
#define JSON_CONFIG_H

#include <map>
#include <string>
#include <vector>
#include <ipfixcol2.h>


/** Parsed configuration of an instance                                                          */
class Config {
private:
    void parse_params(fds_xml_ctx_t *params);
    void default_set();
    void check_validity();
public:


    /**
     * \brief Create a new configuration
     * \param[in] params XML configuration of JSON plugin
     * \throw runtime_error in case of invalid configuration
     */
    Config(const char *params);
    /**
     * \brief Configuration destructor
     */
    ~Config();
};

/**
 * \brief Parse application version (i.e. A.B.C.D)
 *
 * \note At least major and minor version must be specified. Undefined sub-versions are set to zero.
 * \param[in]  str     Version string
 * \param[out] version Parsed version
 * \return #IPX_OK on success
 * \return #IPX_ERR_FORMAT if the version string is malformed
 */
int
parse_version(const std::string &str, int version[4]);

#endif // JSON_CONFIG_H
