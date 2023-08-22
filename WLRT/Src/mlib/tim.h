#pragma once

#include "str.h"

#include <stdint.h>

typedef struct mdate_t
{
	uint16_t year;
	uint8_t  month     : 4;
	uint8_t  dayOfWeek : 4;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  minute;
	uint8_t  second;
	uint16_t millisecond;
	int8_t   timezone;
} mdate_t;

bool mtime_init();
void mtime_deinit();

mdate_t  mdate_utc_now();
mdate_t  mdate_local_now();
int8_t   mdate_timezone();
int64_t  mdate_to_unix(mdate_t date);
mdate_t  mdate_from_unix(int64_t unix);
uint64_t mtime_steady();
uint64_t mtime_high_res();

mstringview_t mmonth_to_string(uint8_t month);
mstringview_t mmonth_to_short_string(uint8_t month);
mstringview_t mday_of_week_to_string(uint8_t dayOfWeek);
mstringview_t mday_of_week_to_short_string(uint8_t dayOfWeek);