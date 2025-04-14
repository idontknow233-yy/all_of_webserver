#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE

#include <string>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unordered_map>
#include <fcntl.h>

#include "../log/log.h"
#include "../buffer/buffer.h"

class httpresponse
{
public:
    httpresponse();
    ~httpresponse();
    void Init(int code_ = -1, std::string dir_ = "", std::string path_ = "", bool isKeepAlive_ = 0);

    void MakeResponse(buffer &buff);

    char *File() { return mmFile; };
    size_t FileLen() { return mmFileStat.st_size; };
    int Code() { return code; };

    void unmapFile();

private:
    void ErrorHtml();
    void AddStateLine(buffer &buff);
    void AddHearder(buffer &buff);
    void AddContent(buffer &buff);
    std::string GetFileType();

    void ErrorContent(buffer &buff);

    int code;
    std::string dir;
    std::string path;
    bool isKeepAlive;

    char *mmFile;
    struct stat mmFileStat;

    static const std::unordered_map<int, std::string> CODE_PATH;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
};

#endif