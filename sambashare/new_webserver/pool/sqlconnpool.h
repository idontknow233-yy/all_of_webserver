#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <queue>
#include <mutex>
#include <mysql/mysql.h>
#include <semaphore.h>
#include <assert.h>
#include <string>
#include <../log/log.h>

class sqlconnpool
{
public:
    static sqlconnpool *Instance();
    void init(const char *host, int port, const char *user, const char *pwd, const char *dbName, int connSize = 10);
    MYSQL *get_conn();
    void free_conn(MYSQL *sql_ptr);
    int get_conn_cnt();
    void close_pool();

private:
    sqlconnpool() = default;
    ~sqlconnpool() = default;

    int MAX_CONN;
    std::queue<MYSQL *> connQue;
    sem_t sem_id;
    std::mutex mtx;
};

class sqlconnRAII
{
public:
    sqlconnRAII(MYSQL **sql, sqlconnpool *sqlconn_prt)
    {
        assert(sqlconn_prt);
        *sql = sqlconn_prt->get_conn();
        sql_ = *sql;
        sqlconn_ptr_ = sqlconn_prt;
    }
    ~sqlconnRAII()
    {
        if (sql_)
            sqlconn_ptr_->free_conn(sql_);
    }

private:
    MYSQL *sql_;
    sqlconnpool *sqlconn_ptr_;
};

#endif