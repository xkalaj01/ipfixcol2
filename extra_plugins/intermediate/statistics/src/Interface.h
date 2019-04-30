/**
 * \file Interface.h
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Interface for managing output submodules (header file)
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
#ifndef IPFIXCOL2_SERVICES_H
#define IPFIXCOL2_SERVICES_H

#include <atomic>
#include <thread>
#include <vector>
#include "Service.h"
#include "Storage.h"
#include "Config.h"

class StatisticsInterface{
public:
    /**
     * \brief Constructor of interface
     * \param storage Pointer to main storage of operational statistics
     * \param cfg Pointer to Configuration of whole Statistics module
     */
    StatisticsInterface(Storage *storage, Config* cfg);

    /**
     * \brief Destructor
     */
    ~StatisticsInterface();

    void Start();

private:
    Storage *storage;
    void Worker();

    std::thread                         thread;
    std::condition_variable             thread_cv;
    std::mutex                          thread_mutex;
    std::vector<StatisticsService *>    services;
};

#endif //IPFIXCOL2_SERVICES_H
