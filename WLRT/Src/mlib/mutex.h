#pragma once

#include "thread.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct mmutex_t
{
	bool value;
} mmutex_t;

typedef struct mrecursive_mutex_t
{
	uint64_t value;
} mrecursive_mutex_t;

typedef struct mshared_mutex_t
{
	uint64_t value;
} mshared_mutex_t;

typedef struct mrecursive_shared_mutex_t
{
	uint64_t value;
} mrecursive_shared_mutex_t;

mmutex_t  mmutex();
mmutex_t* mmutex_new();
void      mmutex_del(mmutex_t* self);
bool      mmutex_cstr(mmutex_t* self);
void      mmutex_dstr(mmutex_t* self);
void      mmutex_lock(mmutex_t* self);
bool      mmutex_try_lock(mmutex_t* self);
void      mmutex_unlock(mmutex_t* self);
void      mmutex_lock_busy(mmutex_t* self);
void      mmutex_unlock_busy(mmutex_t* self);

mrecursive_mutex_t  mrecursive_mutex();
mrecursive_mutex_t* mrecursive_mutex_new();
void                mrecursive_mutex_del(mrecursive_mutex_t* self);
bool                mrecursive_mutex_cstr(mrecursive_mutex_t* self);
void                mrecursive_mutex_dstr(mrecursive_mutex_t* self);
void                mrecursive_mutex_lock(mrecursive_mutex_t* self);
bool                mrecursive_mutex_try_lock(mrecursive_mutex_t* self);
void                mrecursive_mutex_unlock(mrecursive_mutex_t* self);
void                mrecursive_mutex_lock_busy(mrecursive_mutex_t* self);
void                mrecursive_mutex_unlock_busy(mrecursive_mutex_t* self);

mshared_mutex_t  mshared_mutex();
mshared_mutex_t* mshared_mutex_new();
void             mshared_mutex_del(mshared_mutex_t* self);
bool             mshared_mutex_cstr(mshared_mutex_t* self);
void             mshared_mutex_dstr(mshared_mutex_t* self);
void             mshared_mutex_lock(mshared_mutex_t* self);
bool             mshared_mutex_try_lock(mshared_mutex_t* self);
void             mshared_mutex_unlock(mshared_mutex_t* self);
void             mshared_mutex_shared_lock(mshared_mutex_t* self);
bool             mshared_mutex_try_shared_lock(mshared_mutex_t* self);
void             mshared_mutex_shared_unlock(mshared_mutex_t* self);
void             mshared_mutex_lock_busy(mshared_mutex_t* self);
void             mshared_mutex_unlock_busy(mshared_mutex_t* self);
void             mshared_mutex_shared_lock_busy(mshared_mutex_t* self);
void             mshared_mutex_shared_unlock_busy(mshared_mutex_t* self);

mrecursive_shared_mutex_t  mrecursive_shared_mutex();
mrecursive_shared_mutex_t* mrecursive_shared_mutex_new();
void                       mrecursive_shared_mutex_del(mrecursive_shared_mutex_t* self);
bool                       mrecursive_shared_mutex_cstr(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_dstr(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_lock(mrecursive_shared_mutex_t* self);
bool                       mrecursive_shared_mutex_try_lock(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_unlock(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_shared_lock(mrecursive_shared_mutex_t* self);
bool                       mrecursive_shared_mutex_try_shared_lock(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_shared_unlock(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_lock_busy(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_unlock_busy(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_shared_lock_busy(mrecursive_shared_mutex_t* self);
void                       mrecursive_shared_mutex_shared_unlock_busy(mrecursive_shared_mutex_t* self);