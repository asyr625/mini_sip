#ifndef MY_TIME_H
#define MY_TIME_H
#include <string>
unsigned long long my_time(const bool &start_at_epoch = false);

unsigned long long utime(const bool &start_at_epoch = false);

int my_sleep(int msec);

std::string now_str();

#endif // MY_TIME_H
