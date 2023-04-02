#ifndef _EXS3_SNTP_H_
#define _EXS3_SNTP_H_

#include "esp_sntp.h"

#define SET_TIMEZONE()   setenv("TZ", "CST-8", 1);tzset();

void sntp_initalize(void);

void sntp_print_time(const struct tm *timeinfo);

bool sntp_check_time(time_t *time);

void sntp_obtain_time(void);

void sntp_user_sync_time_us(struct timeval *tv);

#endif