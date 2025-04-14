#include "buffer.h"

buffer::buffer(int buffer_size) : buffer_(buffer_size)
{
    writePos_ = 0;
    readPos_ = 0;
} 

void buffer::EnsuerWritable(size_t len)
{
    if(writeable_Size() >= len) 
        return;
    MakeSpace(len);
}

void buffer::MakeSpace(size_t len)
{
    if(writeable_Size() + Prependable_Size() < len)
    {
        buffer_.resize(writePos_ + len + 1);
    }
    else
    {
        int readable = readable_Size();
        std::copy(BeginPtr() + readPos_,BeginPtr() + writePos_,BeginPtr());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
    }
}

void buffer::Append(const char* str,size_t len)
{
    EnsuerWritable(len);
    std::copy(str,str + len,writePos_ptr());
    writePos_ += len;
}

void buffer::Append(const std::string& str)
{
    Append(str.c_str(),str.size());
}

void buffer::Append(const void* data,size_t len)
{
    Append(static_cast<const char*>(data),len);
}

ssize_t buffer::ReadFd(int fd,int* Errno)
{
    char buff[65535];
    struct iovec iov[2];
    iov[0].iov_base = BeginPtr() + writePos_;
    iov[0].iov_len = writeable_Size();
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);
    const ssize_t len = readv(fd,iov,2);

    if(len < 0)
    {
        *Errno = errno;
    }
    else if(len <= writeable_Size())
    {
        writePos_ += len;
    }
    else
    {
        Append(buff, len - writeable_Size());
        writePos_ += len;
    }

    return len; 
}

ssize_t buffer::WriteFd(int fd,int* Errno)
{
    const ssize_t len = write(fd, readPos_ptr(), readable_Size());
    if(len < 0)
    {
        *Errno = errno;
    }
    else
    {
        writePos_ = readPos_ = 0;
    }
    return len;
}

char* buffer::BeginPtr()
{
    return &buffer_[0];
}

const char* buffer::BeginPtrConst() const
{
    return &buffer_[0];
}

char* buffer::readPos_ptr()
{
    return &buffer_[readPos_];
}

const char* buffer::readPos_ptrConst() const
{
    return &buffer_[readPos_];
}

char* buffer::writePos_ptr()
{
    return &buffer_[writePos_];
}

const char* buffer::writePos_ptrConst() const
{
    return &buffer_[writePos_];
}

size_t buffer::writeable_Size()
{
    return buffer_.size() - writePos_;
}

size_t buffer::readable_Size()
{
    return writePos_ - readPos_;
}

size_t buffer::Prependable_Size()
{
    return readPos_;
}

void buffer::Reset_buffer()
{
    writePos_ = readPos_ = 0;
}

std::string buffer::get_str()
{
    std::string str(BeginPtr(), readable_Size());
    Reset_buffer();
    return str;
}