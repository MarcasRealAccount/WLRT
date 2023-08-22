#pragma once

#include "str.h"

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t mthreadid_t;

typedef uint64_t (*mthreadfunc_t)(void* data);
typedef struct mthread_t
{
	mthreadfunc_t func;
	void*         data;

	mstring_t name;

	void*    native;
	uint64_t exitCode;

	mthreadid_t id;
} mthread_t;

bool        mthread_init();
void        mthread_deinit();
mthread_t*  mthread_current();
mthreadid_t mthread_current_id();
void        mthread_wait_on_address(volatile void* address, const void* expected, size_t expectedSize);
void        mthread_wake_by_address_one(volatile void* address);
void        mthread_wake_by_address_all(volatile void* address);
size_t      mthread_storage_alloc();
void        mthread_storage_free(size_t index);
void*       mthread_storage_get(size_t index);
void        mthread_storage_set(size_t index, void* value);
void        mthread_sleep(size_t timeout);
void        mthread_exit(uint64_t exitCode);

mthread_t*    mthread_new(mthreadfunc_t func, void* data);
void          mthread_del(mthread_t* self);
bool          mthread_cstr(mthread_t* self, mthreadfunc_t func, void* data);
void          mthread_dstr(mthread_t* self);
void          mthread_set_name(mthread_t* self, mstringview_t name);
mstringview_t mthread_get_name(mthread_t* self);
mthreadid_t   mthread_get_id(mthread_t* self);
bool          mthread_join(mthread_t* self);
void          mthread_start(mthread_t* self);

void mwait_on_address(volatile void* address, const void* expected, size_t expectedSize);
void mwake_by_address_one(volatile void* address);
void mwake_by_address_all(volatile void* address);