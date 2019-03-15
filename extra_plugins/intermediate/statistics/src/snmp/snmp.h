//
// Created by root on 26.2.19.
//

#ifndef STATISTICS_SNMP_H
#define STATISTICS_SNMP_H

#include "../service.h"

class SNMPService: public StatisticsService{
public:
    SNMPService(std::map<std::pair<std::string, int>, sessionTableEntry> *storage);

    virtual ~SNMPService();

    void run() override;
    void on_notify() override;
private:
    void worker();
    MIBBase mib;
    std::thread thread;
    bool kill_me = 0;
};

#endif //STATISTICS_SNMP_H
