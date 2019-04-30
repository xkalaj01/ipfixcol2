/**
 * \file Interface.cpp
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \brief Interface for managing output submodules (source file)
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

#include <iostream>
#include <ipfixcol2.h>
#include "Interface.h"
#include "snmp/snmp.h"
#include "../../../../src/core/message_ipfix.h"
#include "Config.h"
#include <memory>

StatisticsInterface::~StatisticsInterface() {
    thread_cv.notify_all();
    for (auto const &i: services){
        delete i;
    }
}

void StatisticsInterface::Start() {
    for(auto const& service: services) {
        service->run();
    }

}

void StatisticsInterface::Worker() {
    std::unique_lock<std::mutex> lock(thread_mutex);
    std::chrono::seconds sec(1);

    while (thread_cv.wait_for(lock, sec) == std::cv_status::timeout){
        for (auto const& s : services){
            s->on_notify();
        }
    }
}

StatisticsInterface::StatisticsInterface(Storage *storage, Config* cfg) {
    // Saving pointer to main storage
    this->storage = storage;

    // Creating services based on config//
    if (cfg->outputs.snmp != NULL){
        std::unique_ptr<StatisticsService> snmp(new SNMPService(storage, cfg));
        services.push_back(snmp.release());
    }

}


