//
// Created by root on 24.7.19.
//

#include "../Interface.h"
#include "TextFile.h"
#include "../Config.h"

TextFileService::TextFileService(Storage *storage, cfg_text_file *config) {
    this->storage = storage;
    this->config = config;
    write_to_file = !config->filename.empty();
}

void TextFileService::run() {
    worker_thread = std::thread(&TextFileService::worker, this);
}

void TextFileService::worker() {
    std::unique_lock<std::mutex> lock(worker_mutex);
    std::chrono::seconds sec(config->refresh);

    while (worker_cv.wait_for(lock, sec) == std::cv_status::timeout){
        while(storage_lock.test_and_set(std::memory_order_acquire));
        start_writing();
        write_header();
        write_body();
        stop_writing();
        storage_lock.clear(std::memory_order_release);
    }
}

TextFileService::~TextFileService() {
    // Terminate TextFIle thread
    worker_cv.notify_all();
    if (worker_thread.joinable()){
        worker_thread.join();
    }
}

void TextFileService::start_writing() {
    if (!write_to_file){
        fout = stdout;
        write_line("\033c");
    } else{
        if (config->rewrite){
            fout = fopen(config->filename.c_str(), "w");
        } else{
            fout = fopen(config->filename.c_str(), "a");
        }

        if (fout == nullptr){
            throw std::runtime_error("Failed to open the specified file!");
        }
    }
}

void TextFileService::stop_writing() {
    if (write_to_file){
        fclose(fout);
    }
}

void TextFileService::write_header() {
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    write_line("Date and time: %s\n", asctime(timeinfo));
}

void TextFileService::write_body() {
    if (config->tables.TransportSessionTable){
        write_table_transport_session();
        write_line("\n");
    }
    if (config->tables.TransportSessionStatsTable) {
        write_table_transport_session_stats();
        write_line("\n");
    }
    if (config->tables.TemplateTable){
        write_table_template();
        write_line("\n");
    }
    if (config->tables.TemplateStatsTable) {
        write_table_template_stats();
        write_line("\n");
    }
    if (config->tables.TemplateDefinitionTable) {
        write_table_template_definition();
        write_line("\n");
    }
}

void TextFileService::write_dashes(int length) {
    for (int i = 0; i< length; i++){
        write_line("-");
    }
    write_line("\n");
}

void TextFileService::write_table_transport_session() {
    write_line("Transport Sessions\n");
    write_line("| %s | %s | %-*s : %-*s | %-*s : %-*s | %-*s | %-*s | %-*s | %-*s |\n",
            "Index",
            "Proto",
            15, "Src",
            6, "Port",
            15, "Dst",
            6, "Port",
            16, "Template Refresh",
            19, "Opt. Templ. Refresh",
            7, "IPFIX Version",
            8, "Status"
            );
    write_dashes(139);
    for (auto const& item : storage->TransportSessionTable){
        const TransportSessionEntry_t* entry = &item.second;
        write_line("| %-*d |", 5, entry->Index);
        if (entry->Protocol == IPPROTO_UDP)
            write_line(" %-*s |", 5, "UDP");
        else if (entry->Protocol == IPPROTO_TCP)
            write_line(" %-*s |", 5, "TCP");

        char ip_add[100];
        inet_ntop(entry->SourceAddressType, &entry->SourceAddress, &ip_add[0], 100);
        write_line(" %-*s :", 15, ip_add);
        write_line(" %-*d |", 6, entry->SourcePort);

        inet_ntop(entry->DestinationAddressType, &entry->DestinationAddress, &ip_add[0], 100);
        write_line(" %-*s :", 15, ip_add);
        write_line(" %-*d |", 6, entry->DestinationPort);

        write_line(" %-*d s |", 14, entry->TemplateRefreshTimeout);
        write_line(" %-*d s |", 17, entry->OptionsTemplateRefreshTimeout);

        write_line(" %-*d |", 13, entry->IpfixVersion);
        if (entry->Status == IPX_SESSIONSTATUS_ACTIVE){
            write_line(" active   |");
        } else {
            write_line(" inactive |");
        }

        write_line("\n");
    }

}

void TextFileService::write_table_transport_session_stats() {
    char time_buff[50];
    write_line("Transport Session Statistics\n");
    write_line("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
            16, "Transp. Sess. Id",
            12, "Message Rate",
            12, "Packets",
            12, "Bytes",
            12, "Messages",
            12, "Records",
            12, "Templates",
            15, "Opt. Templates",
            20, "Discontinuity Time");
    write_dashes(149);
    for (auto const& item : storage->TransportSessionStatsTable) {
        const TransportSessionStatsEntry_t* entry = &item.second;
        time2str(&time_buff[0], 50, entry->DiscontinuityTime);
        write_line("| %-*d | %-*d | %-*ld | %-*ld | %-*ld | %-*ld | %-*ld | %-*ld | %-*s |\n",
                16, entry->TransportSessionIndex,
                12, entry->Rate,
                12, entry->Packets,
                12, entry->Bytes,
                12, entry->Messages,
                12, entry->Records,
                12, entry->Templates,
                15, entry->OptionsTemplates,
                20, time_buff);
    }
}

void TextFileService::write_table_template() {
    char time_buffer[50];
    char discont_time_buff[50];

    write_line("Templates:\n");
    write_line("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
            15, "Session Index",
            10, "ODID",
            12, "Template Id",
            10, "Set Id",
            20, "Access Time",
            12, "Data Records",
            18, "Discontinuity Time");
    write_dashes(139);
    for (auto const& item : storage->TemplateTable) {
        const TemplateEntry_t *entry = &item.second;
        const TemplateStatsEntry_t *stats_entry = &storage->TemplateStatsTable[item.first];

        time2str(&time_buffer[0], 50, entry->AccessTime);
        time2str(&discont_time_buff[0], 50, stats_entry->DiscontinuityTime);
        write_line("| %-*d | %-*d | %-*d | %-*d | %-*s | %-*d | %-*s |\n",
                15, entry->TransportSessionIndex,
                10, entry->ObservationDomainId,
                12, entry->Id,
                10, entry->SetId,
                20, time_buffer,
                12, stats_entry->DataRecords,
                18, discont_time_buff);
    }
}

void TextFileService::write_line(const char *str, ...) {
    va_list argptr;
    va_start(argptr, str);
    vfprintf(fout, str, argptr);
    va_end(argptr);

}

void TextFileService::write_table_template_stats() {
    char time_buff[50];
    write_line("Template Statistics:\n");
    write_line("| %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               15, "Session Index",
               10, "ODID",
               12, "Template Id",
               12, "Data Records",
               20, "Discontinuity Time");
    write_dashes(139);
    for (auto const& item : storage->TemplateStatsTable) {
        const TemplateStatsEntry_t *entry = &item.second;
        time2str(&time_buff[0], 50, entry->DiscontinuityTime);
        write_line("| %-*d | %-*d | %-*d | %-*d | %-*s |\n",
                   15, entry->TransportSessionIndex,
                   10, entry->ObservationDomainId,
                   12, entry->TemplateId,
                   12, entry->DataRecords,
                   20, time_buff);
    }
}

void TextFileService::time2str(char *buff, size_t buff_size, time_t timeval) {
    if (timeval > 0){
        struct tm ts;
        ts = *localtime(&timeval);
        strftime(buff, buff_size, "%Y-%m-%d %H:%M:%S", &ts);
    } else {
        buff[0] = '\0';
    }
}

void TextFileService::write_table_template_definition() {
    write_line("Template Definitions:\n");
    write_line("| %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s |\n",
               15, "Session Index",
               10, "ODID",
               12, "Template Id",
               10, "IE Index",
               10, "Length",
               15, "Enterprise No.",
               10, "Flags");
    write_dashes(139);
    for (auto const& item : storage->TemplateDefinitionTable) {
        const TemplateDefinitionEntry_t *entry = &item.second;
        write_line("| %-*d | %-*d | %-*d | %-*d | %-*d | %-*d | %-*d |\n",
                   15, entry->TransportSessionIndex,
                   10, entry->ObservationDomainId,
                   12, entry->TemplateId,
                   12, entry->Index,
                   20, entry->Length,
                   15, entry->EnterpriseNumber,
                   10, entry->Flags);
    }
}


