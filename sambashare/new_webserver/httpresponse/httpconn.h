#ifndef HTTP_CONN
#define HTTP_CONN

#include <stdlib.h>
#include <sys/uio.h>
#include <arpa/inet.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../httprequest/httprequest.h"
#include "httpresponse.h"

class httpconn
{
public:
    httpconn();
    ~httpconn();

    void init(int fd_, sockaddr_in &addr_);

    ssize_t read(int *Error);
    ssize_t write(int *Error);
    bool process();

    int get_fd() { return fd; }
    int get_port() { return addr.sin_port; }
    const char *get_ip() { return inet_ntoa(addr.sin_addr); }
    sockaddr_in get_addr() { return addr; }

    static bool isET;
    static std::atomic<int> user_cnt;
    const static char *src_dir;

private:
    void Close();

    int fd;
    struct sockaddr_in addr;

    bool isClose;

    buffer read_buf, write_buf;

    int iov_cnt;
    struct iovec iov[2];
    httpresponse response;
    httprequest request;
};

#endif