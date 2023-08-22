#pragma once

#include "thread.h"

#include <stdbool.h>
#include <stdint.h>

typedef enum mprofiler_event_e
{
	mprofiler_event_unknown = 0,
	mprofiler_event_thread_bounds,
	mprofiler_event_thread_begin,
	mprofiler_event_thread_end,
	mprofiler_event_frame,
	mprofiler_event_function_begin,
	mprofiler_event_function_end,
} mprofiler_event_e;

typedef struct mprofiler_event_t
{
	uint8_t type;
	uint8_t data[31];
} mprofiler_event_t;

typedef struct mprofiler_event_time_t
{
	uint8_t  type;
	uint64_t time;
} mprofiler_event_time_t;

typedef struct mprofiler_event_thread_bounds_t
{
	uint8_t     type;
	mthreadid_t tid;
	uint32_t    length;
} mprofiler_event_thread_bounds_t;

typedef mprofiler_event_time_t mprofiler_event_thread_begin_t;
typedef mprofiler_event_time_t mprofiler_event_thread_end_t;

typedef struct mprofiler_event_frame_t
{
	uint8_t  type;
	uint64_t time;
	uint64_t frame;
} mprofiler_event_frame_t;

typedef struct mprofiler_event_function_begin_t
{
	uint8_t  type;
	uint64_t time;
	void*    address;
} mprofiler_event_function_begin_t;

typedef mprofiler_event_time_t mprofiler_event_function_end_t;

typedef struct mprofiler_thread_state_t
{
	mthreadid_t       tid          = 0;
	uint32_t          depth        = 0;
	uint8_t           currentIndex = 0;
	bool              capture      = false;
	mprofiler_event_t events[128];
} mprofiler_thread_state_t;

bool mprofiler_init();
void mprofiler_deinit();

void mprofiler_thread_begin(mprofiler_thread_state_t* state);
void mprofiler_thread_end(mprofiler_thread_state_t* state);
void mprofiler_frame(mprofiler_thread_state_t* state, uint64_t frame);
void mprofiler_func_begin(mprofiler_thread_state_t* state, void* address);
void mprofiler_func_end(mprofiler_thread_state_t* state);