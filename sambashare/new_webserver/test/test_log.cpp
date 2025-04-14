#include <iostream>

#include <sys/time.h>

int main()
{

    time_t timer = time(nullptr);
    struct tm* systime = localtime(&timer);
    char file_name[50] = {0};

    std::cout << (int)systime->tm_year + 1900<< " " << (int)systime->tm_mon + 1 <<" "<< (int)systime->tm_mday<<std::endl;
    return 0;
}