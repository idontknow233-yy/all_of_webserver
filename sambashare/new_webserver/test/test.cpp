#include "../log/log.h"
#include "../pool/threadpool.h"
#include <features.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog() {
    int cnt = 0, level = 0;
    log::Instance()->init("./testlog1", ".log", 0, level);
    for(level = 3; level >= 0; level--) {
        log::Instance()->set_level(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 111111111 %d ============= ", "Test", cnt++);
            }
        }
    }
    cnt = 0;
    log::Instance()->init("./testlog2", ".log", 1, level);
    for(level = 0; level < 4; level++) {
        log::Instance()->set_level(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 222222222 %d ============= ", "Test", cnt++);
            }
        }
    }
}

void ThreadLogTask(int i, int cnt) {
    for(int j = 0; j < 10000; j++ ){
        LOG_BASE(i,"PID:[%04d]======= %05d ========= ", gettid(), cnt++);
    }
}

void TestThreadPool() {
    log::Instance()->init("./testThreadpool", ".log", 5000, 0);
    threadpool threadpool(6);
    for(int i = 0; i < 18; i++) {
        threadpool.add_task(std::bind(ThreadLogTask, i % 4, i * 10000));
    }
    getchar();
}

int main() {
    TestLog();
    TestThreadPool();
    return 0;
}
