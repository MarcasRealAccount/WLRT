#include "Exit.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct ExitHandler
{
	ExitCallbackFn callback;
	void*          data;
} ExitHandler;

typedef struct ExitData
{
	size_t       handlerSize;
	size_t       handlerCapacity;
	ExitHandler* handlers;
} ExitData;

static ExitData* s_ExitData;

bool ExitSetup()
{
	s_ExitData = (ExitData*) malloc(sizeof(ExitData));
	if (!s_ExitData) return false;
	s_ExitData->handlerSize     = 0;
	s_ExitData->handlerCapacity = 16;
	s_ExitData->handlers        = (ExitHandler*) malloc(16 * sizeof(ExitHandler));
	return true;
}

void ExitHandle()
{
	if (!s_ExitData || !s_ExitData->handlers) return;
	for (size_t i = s_ExitData->handlerSize; i > 0; --i)
	{
		ExitHandler* handler = s_ExitData->handlers + i - 1;
		handler->callback(handler->data);
	}
	free(s_ExitData->handlers);
	free(s_ExitData);
	s_ExitData = NULL;
}

void ExitRegister(ExitCallbackFn callback, void* data)
{
	if (!s_ExitData || !callback) return;
	if ((++s_ExitData->handlerSize) >= s_ExitData->handlerCapacity)
	{
		size_t       newCapacity = s_ExitData->handlerCapacity << 1;
		ExitHandler* newHandlers = (ExitHandler*) malloc(newCapacity * sizeof(ExitHandler));
		if (!newHandlers) return;
		memcpy(newHandlers, s_ExitData->handlers, s_ExitData->handlerCapacity * sizeof(ExitHandler));
		memset(newHandlers + s_ExitData->handlerCapacity, 0, (newCapacity - s_ExitData->handlerCapacity) * sizeof(ExitHandler));
		free(s_ExitData->handlers);
		s_ExitData->handlerCapacity = newCapacity;
		s_ExitData->handlers        = newHandlers;
	}
	ExitHandler* handler = s_ExitData->handlers + s_ExitData->handlerSize - 1;
	handler->callback    = callback;
	handler->data        = data;
}

void ExitAssert(bool statement, int code)
{
	if (statement)
		return;

#define BUILD_CONFIG_DEBUG   1
#define BUILD_CONFIG_RELEASE 2
#define BUILD_CONFIG_DIST    3
#if defined(_MSC_VER) && (BUILD_CONFIG == BUILD_CONFIG_DEBUG)
	__debugbreak();
#endif
#undef BUILD_CONFIG_DEBUG
#undef BUILD_CONFIG_RELEASE
#undef BUILD_CONFIG_DIST

	ExitHandle();
	exit(code);
}