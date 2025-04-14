#include <sqlconnpool.h>

sqlconnpool *sqlconnpool::Instance()
{
    static sqlconnpool sql_conn;
    return &sql_conn;
}

void sqlconnpool::init(const char *host, int port, const char *user, const char *pwd, const char *dbName, int connSize)
{
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++)
    {
        MYSQL *conn = nullptr;
        conn = mysql_init(conn);
        if (!conn)
        {
            LOG_ERROR("Mysql init error!");
            assert(conn);
        }
        conn = mysql_real_connect(conn, host, user, pwd, dbName, port, nullptr, 0);
        if (!conn)
        {
            LOG_ERROR("Mysql connect error!");
            assert(conn);
        }
        connQue.emplace(conn);
    }
    MAX_CONN = connSize;
    sem_init(&sem_id, 0, connSize);
}

MYSQL *sqlconnpool::get_conn()
{
    std::lock_guard<std::mutex> locker(mtx);
    if (connQue.empty())
    {
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&sem_id);
    MYSQL *sql_ptr = connQue.front();
    connQue.pop();
    return sql_ptr;
}

void sqlconnpool::free_conn(MYSQL *sql_ptr)
{
    std::lock_guard<std::mutex> locker(mtx);
    sem_post(&sem_id);
    connQue.emplace(sql_ptr);
}

int sqlconnpool::get_conn_cnt()
{
    std::lock_guard<std::mutex> locker(mtx);
    return connQue.size();
}

void sqlconnpool::close_pool()
{
    std::lock_guard<std::mutex> locker(mtx);
    while (connQue.size())
    {
        MYSQL *sql_ptr = connQue.front();
        connQue.pop();
        mysql_close(sql_ptr);
    }
    mysql_library_end();
}