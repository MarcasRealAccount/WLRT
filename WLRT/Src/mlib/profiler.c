#include "profiler.h"

// TODO(MarcasRealAccount): Implement mprofiler
bool mprofiler_init()
{
	return true;
}

void mprofiler_deinit()
{
}

void mprofiler_thread_begin(mprofiler_thread_state_t* state)
{
}

void mprofiler_thread_end(mprofiler_thread_state_t* state)
{
}

void mprofiler_frame(mprofiler_thread_state_t* state, uint64_t frame)
{
}

void mprofiler_func_begin(mprofiler_thread_state_t* state, void* address)
{
}

void mprofiler_func_end(mprofiler_thread_state_t* state)
{
}