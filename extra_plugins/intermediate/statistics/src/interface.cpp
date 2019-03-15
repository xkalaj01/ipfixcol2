//
// Created by root on 26.2.19.
//

#include <iostream>
#include <ipfixcol2.h>
#include "interface.h"
#include "snmp/snmp.h"
#include "../../../../src/core/message_ipfix.h"

void StatisticsInterface::Start() {
    for(auto const& service: this->services) {
        service->run();
    }
    this->thread = std::thread(&StatisticsInterface::Worker, this);
}

void StatisticsInterface::Worker() {
    while (!this->kill_me){
        sleep(5);
        for (auto const& s : this->services){
            s->on_notify();
        }
    }
}

StatisticsInterface::~StatisticsInterface() {
    this->kill_me=1;
    thread.join();
}

StatisticsInterface::StatisticsInterface() {
    std::unique_ptr<SNMPService> snmp(new SNMPService(&this->storage));
    this->services.push_back(snmp.release());
    this->exporter_counter = 0;
}

void StatisticsInterface::processPacket(ipx_msg_ipfix_t *msg) {
    std::string ip_add;
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(msg->ctx.session->udp.net.addr_src.ipv4), ip, INET_ADDRSTRLEN );
    ip_add = ip;
    int port = msg->ctx.session->udp.net.port_src;
    std::pair <std::string, int> id;
    id = std::make_pair(ip, port);
    sessionTableEntry *sessTab = NULL;
    if (this->storage.find(id) == this->storage.end()){
        // Adding new session
        this->exporter_counter++;
        sessTab = &(this->storage[id]);
        sessTab->Protocol = msg->ctx.session->type;
        sessTab->id = exporter_counter;

        const ipx_session_net *net = NULL;
        if (msg->ctx.session->type == FDS_SESSION_UDP) {
            net = &msg->ctx.session->udp.net;
            sessTab->TemplateRefreshTimeout = msg->ctx.session->udp.lifetime.tmplts;
            sessTab->OptionsTemplateRefreshTimeout =  msg->ctx.session->udp.lifetime.tmplts;
        }
        else if (msg->ctx.session->type == FDS_SESSION_TCP){
            net = &msg->ctx.session->tcp.net;
        }
        else if(msg->ctx.session->type == FDS_SESSION_SCTP){
            net= &msg->ctx.session->sctp.net;
        }
        else{
            // Unknown protocol -> skip this message;
            return;
        }

        if (net->l3_proto == AF_INET){
            sessTab->SourceAddressType = AF_INET;
            sessTab->SourceAddress.IPv4 = net->addr_src.ipv4;
            sessTab->DestAddressType = AF_INET;
            sessTab->DestAddress.DestIPv4 = net->addr_dst.ipv4;
        }
        else if (net->l3_proto == AF_INET6){
            sessTab->SourceAddressType = AF_INET6;
            sessTab->SourceAddress.IPv6 = net->addr_src.ipv6;
            sessTab->DestAddressType = AF_INET6;
            sessTab->DestAddress.DestIPv6 = net->addr_dst.ipv6;
        }
        sessTab->SourcePort = net->port_src;
        sessTab->DestPort = net->port_dst;
        sessTab->DeviceMode = DEVICE_MODE_COLLECTING;
        sessTab->Status = TRANSPORT_SESSION_ACTIVE;
    }
}
