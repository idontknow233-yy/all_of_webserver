#include "httpconn.h"

bool httpconn::isET = true;
std::atomic<int> httpconn::user_cnt = 0;
const char *httpconn::src_dir = "";

httpconn::httpconn()
{
    isClose = true;
    fd = -1;
    addr = {0};
}

httpconn::~httpconn()
{
    Close();
}

void httpconn::Close()
{
    response.unmapFile();
    if (isClose == false)
    {
        isClose = true;
        fd = -1;
        addr = {0};
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd, get_ip(), get_port(), (int)user_cnt);
    }
}

void httpconn::init(int fd_, sockaddr_in &addr_)
{
    assert(fd != -1);
    isClose = false;
    user_cnt++;
    fd = fd_;
    addr = addr_;
    write_buf.Reset_buffer();
    read_buf.Reset_buffer();
    LOG_INFO("Client[%d](%s:%d) in, UserCount:%d", fd, get_ip(), get_port(), (int)user_cnt);
}

ssize_t httpconn::read(int *Error)
{
    ssize_t len = -1;
    do
    {
        len = read_buf.ReadFd(fd, Error);
        if (len <= 0)
        {
            break;
        }
    } while (isET);
    return len;
}

ssize_t httpconn::write(int *Error)
{
    ssize_t len = -1;
    do
    {
        len = writev(fd, iov, iov_cnt);
        if (len <= 0)
        {
            *Error = errno;
            break;
        }

        if(iov[0].iov_len + iov[1].iov_len == 0)
        {
            break;
        }
        else if(len > iov[0].iov_len)
        {
            iov[1].iov_base = iov[1].iov_base + len - iov[0].iov_len;
            iov[1].iov_len = iov[1].iov_len + len -iov[0].iov_len;
            iov[0].iov_base = iov[0].iov_base + iov[0].iov_len;
            iov[0].iov_len = 0;
        }
        else
        {
            iov[0].iov_base = iov[0].iov_base + iov[0].iov_len;
            iov[0].iov_len = 0;
        }
    } while (isET);
    return len;
}

bool httpconn::process()
{
    request.init();
    if(read_buf.readable_Size() <= 0)
    {
        return false;
    }
    else if(request.Parse(read_buf))
    {
        LOG_DEBUG("%s", request.Path().c_str());
        response.Init(200, src_dir, request.Path(), request.IsKeepAlive());
    }
    else
    {
        response.Init(400, src_dir, request.Path(), false);
    }

    response.MakeResponse(write_buf);

    iov[0].iov_base = write_buf.BeginPtr();
    iov[0].iov_len = write_buf.readable_Size();
    iov_cnt = 1;

    if(response.FileLen() > 0  && response.File()) {
        iov[1].iov_base = response.File();
        iov[1].iov_len = response.FileLen();
        iov_cnt = 2;        
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response.FileLen() , iov_cnt, iov[0].iov_len + iov[1].iov_len);
    return true;
}