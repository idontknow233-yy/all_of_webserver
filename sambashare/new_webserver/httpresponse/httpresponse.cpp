#include "httpresponse.h"

const std::unordered_map<int, std::string> httpresponse::CODE_PATH = {
    {400, "/400.html"}, {403, "/401.html"}, {404, "/404.html"}};

const std::unordered_map<int, std::string> httpresponse::CODE_STATUS = {
    {200, "OK"}, {400, "Bad Request"}, {403, "Forbidden"}, {404, "Not Found"}};

const std::unordered_map<std::string, std::string> httpresponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", ""},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "}};


httpresponse::httpresponse()
{
    code = -1;
    dir = "";
    path = "";
    isKeepAlive = false;
    mmFile = nullptr;
    mmFileStat = {0};
}

httpresponse::~httpresponse()
{
    unmapFile();
}

void httpresponse::unmapFile()
{
    if(mmFile)
    {
        munmap(mmFile, mmFileStat.st_size);
        mmFile = nullptr;
    }
}

void httpresponse::Init(int code_, std::string dir_, std::string path_, bool isKeepAlive_)
{
    code = code_;
    dir = dir_;
    path = path_;
    isKeepAlive = isKeepAlive_;
    mmFile = nullptr;
    mmFileStat = {0};
}

void httpresponse::MakeResponse(buffer &buff)
{
    if(stat((dir + path).data(), &mmFileStat) == -1 || S_ISDIR(mmFileStat.st_mode))
    {
        code = 404;
    }
    else if(mmFileStat.st_mode & S_IROTH == 0)
    {
        code = 403;
    }
    code = 200;
    ErrorHtml();
    AddStateLine(buff);
    AddHearder(buff);
    AddContent(buff);
}

void httpresponse::ErrorHtml()
{
    if(CODE_PATH.count(code))
    {
        path = CODE_PATH.find(code)->second;
        stat((dir + path).data(), &mmFileStat);
    }
}

void httpresponse::AddStateLine(buffer &buff)
{
    std::string status;
    if(CODE_STATUS.count(code))
    {
        status = CODE_STATUS.find(code)->second;
    }
    else
    {
        code = 400;
        status = CODE_STATUS.find(code)->second;
    }
    buff.Append("Http/1.1 " + std::to_string(code) + status + "\r\n");
}

void httpresponse::AddHearder(buffer &buff)
{
    buff.Append("Contention: ");
    if(isKeepAlive)
    {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    }
    else
    {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType() + "\r\n");
}

std::string httpresponse::GetFileType()
{
    std::string::size_type idx = path.find_last_of('.');
    if(idx == std::string::npos)
    {
        return "text/plain";
    }
    std::string suf = path.substr(idx);
    if(SUFFIX_TYPE.count(suf))
    {
        return SUFFIX_TYPE.find(suf)->second;
    }
    else
    {
        return "text/plain";
    }
}

void httpresponse::AddContent(buffer &buff)
{
    int fd = open((dir + path).data(), O_RDONLY);
    if(fd == -1)
    {
        ErrorContent(buff);
        return;
    }
    int *mmRef = (int*)mmap(0, mmFileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(*mmRef == -1)
    {
        ErrorContent(buff);
        return;
    }

    LOG_DEBUG("file path is %s", (dir + path).data());
    mmFile = (char*)mmRef;
    close(fd);
    buff.Append("Content-length: " + std::to_string(mmFileStat.st_size) + "\r\n\r\n");
}

void httpresponse::ErrorContent(buffer &buff)
{
    std::string body;
    std::string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code) == 1) {
        status = CODE_STATUS.find(code)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code) + " : " + status  + "\n";
    body += "<p>File NotFound!</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}