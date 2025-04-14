#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <error.h>
#include <algorithm>
#include <regex>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnpool.h"

class httprequest
{
public:
    enum PARSE_STATE
    {
        REQUSET_LINE,
        HEADERS,
        BODY,
        FINESH
    };

    httprequest()
    {
        init();
    }

    ~httprequest() = default;

    void init();

    bool Parse(buffer& buff);

    std::string Path() { return path; };

    bool IsKeepAlive() const;

private:
    bool ParseRequestLine(const std::string& line);
    void ParsePath();
    void ParseHeader(const std::string& line);
    void ParseBody(const std::string& line);

    void ParsePost();
    void ParseFromUrlencoded();
    bool UserVerify(const std::string &username, const std::string &password, bool islogin);

    int ConvertHex(char ch);

    PARSE_STATE state;
    std::string method, path, version, body;
    std::unordered_map<std::string, std::string> header, post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};

#endif