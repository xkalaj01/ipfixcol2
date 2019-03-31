//
// Created by root on 11.12.18.
//

#ifndef IPFIXCOL2_SERVICES_H
#define IPFIXCOL2_SERVICES_H

#include <atomic>
#include <thread>
#include <vector>
#include "service.h"
#include "storage.h"

class StatisticsInterface{
public:
    StatisticsInterface(Storage *storage);

    void Start();
    void Stop();

    bool kill_me = 0;
private:
    Storage *storage;
    void Worker();

    std::thread thread;
    std::vector<StatisticsService *> services;
};

#endif //IPFIXCOL2_SERVICES_H
