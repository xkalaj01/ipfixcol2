//
// Created by root on 11.12.18.
//

#ifndef IPFIXCOL2_SERVICES_H
#define IPFIXCOL2_SERVICES_H

#include <atomic>
#include <thread>
#include "map.h"
#include "snmp/mib.h"

/** Instance */
struct Instance {
    uint16_t snmp_kill;
    uint uns_var;
    int  int_var;
    std::atomic_flag mib_lock = ATOMIC_FLAG_INIT;
    MIBBase *mib;
    StatMap *stats;
    std::vector <std::thread *> services;
};

void snmp_agent(struct Instance *data);


#endif //IPFIXCOL2_SERVICES_H
