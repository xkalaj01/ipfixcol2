//
// Created by root on 26.2.19.
//

#ifndef STATISTICS_SERVICE_H
#define STATISTICS_SERVICE_H

#include "interface.h"

struct sessionTableEntry;
/***
 * Abstract class for creating new service instances
 */
class StatisticsService{
public:
    StatisticsService(std::map<std::pair<std::string, int>, sessionTableEntry> *storage) : storage(storage) {}

    virtual ~StatisticsService() {};

    virtual void on_notify() = 0;
    virtual void run() = 0;
    std::map<std::pair<std::string, int>, sessionTableEntry> *storage;
};

#endif //STATISTICS_SERVICE_H
