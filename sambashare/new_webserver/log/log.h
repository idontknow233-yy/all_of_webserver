#ifndef LOG_H
#define LOG_H

#include "blockqueue.h"
#include "../buffer/buffer.h"
#include <thread>
#include <assert.h>
#include <stdarg.h>
#include <sys/stat.h>


class log{

public:
    static log* Instance();
    void init(const char* t_path, const char* t_suffix, bool t_isAsync, int t_level);
    void write_file(int level, const char* format, ...);
    int get_level();
    void set_level(int t_level);
    void flush();
    void add_level_to_buffer(int t_leve);
    bool IsOpen();
    int level;
private:
    log(int t_MAX_LIME = 10000, bool t_isAsync = false);
    ~log();

    static void flush_write_thread();
    void Async_write();
    
    const char* path;
    const char* suffix;
    int MAX_LINES;
    int line_count;
    int today_date;
    bool isOpen;
    buffer buff_;
    
    bool isAsync;
    FILE *fp;

    std::unique_ptr<blockqueue<std::string>> deq_;
    std::unique_ptr<std::thread> wirite_thread;
    std::mutex mtx;
};

#define LOG_BASE(level, format, ...) \
    do {\
        log* log = log::Instance();\
        if (log->IsOpen() && log->get_level() <= level) {\
            log->write_file(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);    
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif