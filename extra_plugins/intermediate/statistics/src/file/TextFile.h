//
// Created by root on 24.7.19.
//

#ifndef STATISTICS_TEXTFILE_H
#define STATISTICS_TEXTFILE_H

#include "../Service.h"
#include "../Config.h"
#include <stdarg.h>
#include <ctime>

class TextFileService: public StatisticsService{
public:
    explicit TextFileService(Storage *storage, cfg_text_file *config);
    ~TextFileService() override;

    void run() override;
    void on_notify() override {};

private:
    void worker();
    std::mutex              worker_mutex;
    std::condition_variable worker_cv;
    std::thread             worker_thread;

    bool        write_to_file;
    FILE*       fout;
    Storage*    storage;
    cfg_text_file *config;

    void start_writing();
    void stop_writing();

    void write_header();
    void write_body();


    void write_table_transport_session();
    void write_table_transport_session_stats();
    void write_table_template();
    void write_table_template_stats();
    void write_table_template_definition();

    void write_line(const char *str, ...);
    void write_dashes(int length);

    void time2str(char *buff, size_t buff_size, time_t timeval);
};


#endif //STATISTICS_TEXTFILE_H
