#include "exit.h"
#include "build.h"
#include "dynarray.h"
#include "thread.h"

#include <stdlib.h>

#if BUILD_IS_SYSTEM_WINDOWS
	#include <intrin.h>
#else
#endif

typedef struct mexit_handler_t
{
	mexit_func_t func;
	void*        data;
} mexit_handler_t;

typedef struct mexit_data_t
{
	mdynarray_t handlers;
} mexit_data_t;

static size_t s_ExitTLS = 0;

bool mexit_init()
{
	s_ExitTLS              = mthread_storage_alloc();
	mexit_data_t* exitData = (mexit_data_t*) mmalloc(sizeof(mexit_data_t));
	mdynarray_cstr(&exitData->handlers, 32, sizeof(mexit_handler_t));
	mthread_storage_set(s_ExitTLS, exitData);
	return true;
}

void mexit_deinit()
{
	mexit_handle();
	mthread_storage_free(s_ExitTLS);
}

void mexit_register(mexit_func_t func, void* data)
{
	if (!func)
		return;

	mexit_data_t* exitData = (mexit_data_t*) mthread_storage_get(s_ExitTLS);
	if (!exitData)
	{
		exitData = (mexit_data_t*) mmalloc(sizeof(mexit_data_t));
		mdynarray_cstr(&exitData->handlers, 32, sizeof(mexit_handler_t));
		mthread_storage_set(s_ExitTLS, exitData);
	}
	mexit_handler_t handler = {
		.func = func,
		.data = data
	};
	mdynarray_pushback(&exitData->handlers, &handler);
}

void massert_register(bool statement, uint64_t exitCode, mexit_func_t func, void* data)
{
	massert(statement, exitCode);
	mexit_register(func, data);
}

void mexit_handle()
{
	mexit_data_t* exitData = (mexit_data_t*) mthread_storage_get(s_ExitTLS);
	if (exitData)
	{
		for (size_t i = 0; i < exitData->handlers.size; ++i)
		{
			mexit_handler_t* handler = (mexit_handler_t*) mdynarray_get(&exitData->handlers, exitData->handlers.size - i - 1);
			handler->func(handler->data);
		}
		mdynarray_dstr(&exitData->handlers);
		mfree(exitData);
	}
}

void mexit(uint64_t exitCode)
{
	mexit_handle();
	mthread_t* current = mthread_current();
	if (current)
		mthread_exit(exitCode);
	else
		exit((int) exitCode);
}

void massert(bool statement, uint64_t exitCode)
{
	if (statement)
		return;

#if BUILD_IS_CONFIG_DEBUG
	#if BUILD_IS_SYSTEM_WINDOWS
	__debugbreak();
	#else
	#endif
#endif
	mexit(exitCode);
}