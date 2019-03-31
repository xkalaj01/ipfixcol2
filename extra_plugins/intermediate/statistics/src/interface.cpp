//
// Created by root on 26.2.19.
//

#include <iostream>
#include <ipfixcol2.h>
#include "interface.h"
#include "snmp/snmp.h"
#include "../../../../src/core/message_ipfix.h"
#include <memory>

void StatisticsInterface::Start() {
    for(auto const& service: this->services) {
        service->run();
    }
    this->thread = std::thread(&StatisticsInterface::Worker, this);
}

void StatisticsInterface::Worker() {
    while (!this->kill_me){
        sleep(1);
        std::map<std::string, struct ExporterInfo>::iterator it;
        while(storage_lock.test_and_set(std::memory_order_acquire));
        for ( it = storage->active_exporters.begin(); it != storage->active_exporters.end(); it++){
            storage->TransportSessionStatsTable[it->second.TransportSessionTableId].Rate = it->second.RateCounter;
            it->second.RateCounter = 0;
        }
        storage_lock.clear(std::memory_order_release);
        for (auto const& s : this->services){
            s->on_notify();
        }
    }
}

StatisticsInterface::StatisticsInterface(Storage *storage) {
    // Saving pointer to main storage
    this->storage = storage;

    // Registration of new services //
    std::unique_ptr<SNMPService> snmp(new SNMPService(storage));
    this->services.push_back(snmp.release());

}

void StatisticsInterface::Stop() {
    this->kill_me=1;
    for (auto const &i: this->services){
        i->destroy();
    }
    thread.join();
    std::cout<<"Interface ends"<<std::endl;
}

