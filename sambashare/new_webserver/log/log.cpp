#include "log.h"

log::log(int t_MAX_LIME, bool t_isAsync)
{
    MAX_LINES = t_MAX_LIME;
    isOpen = true;
    isAsync = t_isAsync;
    fp = nullptr;
    wirite_thread = nullptr;
    deq_ = nullptr;
    today_date = 0;
}

log::~log()
{
    if (isAsync && deq_) {
        // 标记队列关闭并唤醒所有等待线程
        deq_->Close();
        
        // 等待日志写入线程完成剩余任务
        if (wirite_thread && wirite_thread->joinable()) {
            wirite_thread->join();
        }
    }

    // 刷新并关闭文件
    if (fp) {
        std::lock_guard<std::mutex> locker(mtx);
        flush();
        fclose(fp);
        fp = nullptr;
    }
    isOpen = false;
}

log* log::Instance()
{
    static log log_;
    return &log_;
}

void log::init(const char* t_path, const char* t_suffix, bool t_isAsync, int t_level)
{
    path = t_path;
    suffix = t_suffix;
    isAsync = t_isAsync;

    if(isAsync)
    {
        if(deq_ == nullptr)
        {
            std::unique_ptr<blockqueue<std::string>> newque(new blockqueue<std::string>);
            std::unique_ptr<std::thread> newthread(new std::thread(flush_write_thread));
            deq_ = std::move(newque);
            wirite_thread = std::move(newthread);
        }
    }

    line_count = 0;

    time_t timer = time(nullptr);
    struct tm* systime = localtime(&timer);
    char file_name[250] = {0};
    sprintf(file_name, "%s/%04d_%02d_%02d%s", path, systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday, suffix);
    today_date = systime->tm_mday;

    std::lock_guard<std::mutex> locker(mtx);

    if(fp && isAsync)
    {
        flush();
        fclose(fp);
    }

    buff_.Reset_buffer();

    fp = fopen(file_name, "a");
        //printf("*%s\n", file_name);
    if(fp == nullptr)
    {
        //printf("path = %s\n",path);
        mkdir(path, 0777);
        fp = fopen(file_name, "a");
    }

    assert(fp != nullptr);
}

void log::write_file(int t_level, const char* format, ...)
{
    
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    char file_name[250] = {0};
    
    if(today_date == t.tm_mday && line_count >= MAX_LINES && line_count % MAX_LINES == 0)
    {
        std::lock_guard<std::mutex> locker(mtx);
        sprintf(file_name, "%s/%04d_%02d_%02d_%d%s", 
            path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, line_count/MAX_LINES + 1, suffix);
        flush();
        fclose(fp);
        // printf("*%s*\n",file_name);
        fp = fopen(file_name, "a");
    }
    else if(today_date != t.tm_mday)
    {
        std::lock_guard<std::mutex> locker(mtx);
        sprintf(file_name, "%s/%04d_%02d_%02d%s", path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix);
        today_date = t.tm_mday;
        flush();
        fclose(fp);
        printf("*%s*\n",file_name);
        fp = fopen(file_name, "a");
    }

    std::unique_lock<std::mutex> locker(mtx);
    
    assert(fp != nullptr);
    
    line_count ++;

    char temp[250];
    size_t len = sprintf(temp, "%04d-%02d-%02d %02d:%02d:%02d:%06ld", 
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
    buff_.Append(temp, len);//printf("***\n");
    add_level_to_buffer(t_level);
    // printf("***\n");
    va_list valist_;
    va_start(valist_, format);
    len = vsnprintf(temp, 250, format, valist_);
    va_end(valist_);
    // printf("%s %s\n",format,temp);
    buff_.Append(temp, len);
    buff_.Append("\n\0");
    
    if(isAsync)
    {
        std::string str = buff_.get_str();//printf("%d %s\n", line_count, file_name);
        locker.unlock();
        deq_->push_back(str);
    }
    else
    {
        fputs(buff_.get_str().c_str(), fp);
        locker.unlock();
    }
}

int log::get_level()
{
    return level;
}

void log::set_level(int t_level)
{
    level = t_level;
}

void log::flush_write_thread()
{
    Instance()->Async_write();
}

void log::Async_write() {
    std::string str;
    while (deq_->pop(str)) {             // 无锁调用 pop()
        {   
            std::lock_guard<std::mutex> locker(mtx); // 仅保护文件写入
            fputs(str.c_str(), fp);
        }
    }
}

void log::flush()
{
    if(isAsync)
    {
        deq_->flush();
    }
    fflush(fp);
}

bool log::IsOpen()
{
    return isOpen;
}

void log::add_level_to_buffer(int t_level)
{
    switch(t_level) {
        case 0:
            buff_.Append("[debug]: ", 9);
            break;
        case 1:
            buff_.Append("[info] : ", 9);
            break;
        case 2:
            buff_.Append("[warn] : ", 9);
            break;
        case 3:
            buff_.Append("[error]: ", 9);
            break;
        default:
            buff_.Append("[info] : ", 9);
            break;
        }
}