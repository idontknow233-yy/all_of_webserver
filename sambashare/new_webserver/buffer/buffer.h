#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <atomic>
#include <string>
#include <sys/uio.h>
#include <unistd.h>


class buffer{
    public:
        buffer(int buffer_size = 1024);
        ~buffer() = default;

        void EnsuerWritable(size_t len);
        void MakeSpace(size_t len);

        void Append(const char* str,size_t len);
        void Append(const std::string& str);
        void Append(const void* data,size_t len);

        void Reset_buffer();
        std::string get_str();

        ssize_t ReadFd(int fd,int* Errno);
        ssize_t WriteFd(int fd,int* Errno);

        char* BeginPtr();
        const char* BeginPtrConst() const;
        char* readPos_ptr();
        const char* readPos_ptrConst() const;
        char* writePos_ptr();
        const char* writePos_ptrConst() const;

        size_t writeable_Size();
        size_t readable_Size();
        size_t Prependable_Size();

    private:
        std::vector<char> buffer_;
        std::atomic<std::size_t> readPos_, writePos_;       
};

#endif