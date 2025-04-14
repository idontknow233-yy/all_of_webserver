#include "./httprequest.h"

const std::unordered_set<std::string> httprequest::DEFAULT_HTML =
    {"/index", "/login", "/register", "/welcome", "/video", "/picture"};

const std::unordered_map<std::string, int> httprequest::DEFAULT_HTML_TAG =
    {{"/register.html", 0}, {"/login.html", 1}};

void httprequest::init()
{
    method = path = version = body = "";
    header.clear();
    post.clear();
    state = REQUSET_LINE;
}

bool httprequest::Parse(buffer &buff)
{
    if (buff.readable_Size() <= 0)
        return false;
    const char CRLF[] = "\r\n";

    while (buff.readable_Size() > 0 && state != FINESH)
    {
        const char *line_end = std::search(buff.readPos_ptrConst(), buff.writePos_ptrConst(), CRLF, CRLF + 2);
        const std::string line(buff.readPos_ptrConst(), line_end);
        switch (state)
        {
        case REQUSET_LINE:
            if (ParseRequestLine(line) == 0)
                return false;
            ParsePath();
            break;
        case HEADERS:
            ParseHeader(line);
            if (buff.readable_Size() < 2)
                state = BODY;
            break;
        case BODY:
            ParseBody(line);
            state = FINESH;
            break;
        default:
            break;
        }
    }
    LOG_DEBUG("","");
    return true;
}

bool httprequest::ParseRequestLine(const std::string &line)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line, subMatch, patten))
    {
        method = subMatch[1];
        path = subMatch[2];
        version = subMatch[3];
        state = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error!");
    return false;
}

void httprequest::ParsePath()
{
    if(path == "/")
        path = "/index.html";
    else if(DEFAULT_HTML.count(path))
        path +=".html";
}

void httprequest::ParseHeader(const std::string& line)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line, subMatch, patten))
    {
        header[subMatch[1]] = subMatch[2];
    }
    else 
        state = BODY;
}

void httprequest::ParseBody(const std::string& line)
{
    body = line;
    ParsePost();
    state = FINESH;
    LOG_DEBUG("","");
}

void httprequest::ParsePost()
{
    if(method == "POST" &&  header["Content-Type"] == "application/x-www-form-urlencoded")
    {
        ParseFromUrlencoded();
        if(DEFAULT_HTML_TAG.count(path))
        {
            int tag = DEFAULT_HTML_TAG.find(path)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(UserVerify(post["username"], post["password"], tag))
            {
                path = "/welcome.html";
            }
            else
            {
                path = "/error.html";
            }
        }
    }
}

void httprequest::ParseFromUrlencoded()
{
    if(body.size() == 0) { return; }

    std::string key, value;
    int num = 0;
    int n = body.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body[i];
        switch (ch) {
        // key
        case '=':
            key = body.substr(j, i - j);
            j = i + 1;
            break;
        // 键值对中的空格换为+或者%20
        case '+':
            body[i] = ' ';
            break;
        case '%':
            num = ConvertHex(body[i + 1]) * 16 + ConvertHex(body[i + 2]);
            body[i + 2] = num % 10 + '0';
            body[i + 1] = num / 10 + '0';
            i += 2;
            break;
        // 键值对连接符
        case '&':
            value = body.substr(j, i - j);
            j = i + 1;
            post[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(post.count(key) == 0 && j < i) {
        value = body.substr(j, i - j);
        post[key] = value;
    }
}

bool httprequest::UserVerify(const std::string &username, const std::string &password, bool islogin)
{
    if(username == "" || password == "" ) 
        return false;
    
    LOG_INFO("Verify username: %s, password: %s", username, password);
    MYSQL *sql;
    sqlconnRAII(&sql, sqlconnpool::Instance());
    assert(sql);

    bool flag = !islogin;
    int j = 0;
    char order[256];
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;

    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", username.c_str());
    LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) 
    { 
        mysql_free_result(res);
        return false; 
    }

    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);
    while(MYSQL_ROW row = mysql_fetch_row(res))
    {
        LOG_DEBUG("MYSQL ROW: %s %s",row[0], row[1]);
        if(islogin)
        {
            if(password == row[1])
            {
                flag = true;
            }
            else
            {
                LOG_INFO("password is incorrect");
                flag = false;
                return false;
            }
        }
        else if(username == row[0])
        {
            flag = false;
            LOG_INFO("username has userd");
            return false;
        }
    }
    mysql_free_result(res);

    if(!islogin && flag)
    {
        LOG_DEBUG("register");
        bzero(order,256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", username.c_str(), password.c_str());
        LOG_DEBUG("order");
        if(mysql_query(sql, order))
        {
            LOG_INFO("insert error");
            flag = false;
            return false;
        }
    }

    LOG_DEBUG("Verify success");
    return true;
}

int httprequest::ConvertHex(char ch)
{
    if('A' <= ch && ch <= 'F') return ch -'A'+10;
    if('a' <= ch && ch <= 'f') return ch -'a'+10;
    else return ch - '0';
}

bool httprequest::IsKeepAlive() const {
    if(header.count("Connection") == 1) {
        return header.find("Connection")->second == "keep-alive" && version == "1.1";
    }
    return false;
}