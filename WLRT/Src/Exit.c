#include "Exit.h"
#include "Build.h"
#include "DynArray.h"

#include <stdlib.h>

typedef struct WLRTExitHandlerData
{
	WLRTExitCallbackFn callback;
	void*              userdata;
} WLRTExitHandlerData;

typedef struct WLRTExitData
{
	WLRTDynArray handlers;
} WLRTExitData;

static WLRTExitData s_ExitData;

bool WLRTExitSetup()
{
	if (!WLRTDynArraySetup(&s_ExitData.handlers, 32, sizeof(WLRTExitHandlerData)))
		return false;
	return true;
}

void WLRTExitHandle()
{
	for (size_t i = s_ExitData.handlers.size; i > 0; --i)
	{
		WLRTExitHandlerData* handler = (WLRTExitHandlerData*) WLRTDynArrayGet(&s_ExitData.handlers, i - 1);
		handler->callback(handler->userdata);
	}
	WLRTDynArrayCleanup(&s_ExitData.handlers);
}

void WLRTExitRegister(WLRTExitCallbackFn callback, void* userdata)
{
	if (!callback)
		return;

	WLRTExitHandlerData handler = {
		.callback = callback,
		.userdata = userdata
	};
	WLRTDynArrayPushBack(&s_ExitData.handlers, &handler);
}

void WLRTExitAssert(bool statement, int code)
{
	if (statement)
		return;

#if BUILD_IS_SYSTEM_WINDOWS && BUILD_IS_CONFIG_DEBUG
	__debugbreak();
#endif

	WLRTExitHandle();
	exit(code);
}