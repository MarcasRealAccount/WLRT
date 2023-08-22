#include "mutex.h"
#include "atomic.h"

mmutex_t mmutex()
{
	mmutex_t mtx;
	memset(&mtx, 0, sizeof(mtx));
	mmutex_cstr(&mtx);
	return mtx;
}

mmutex_t* mmutex_new()
{
	mmutex_t* mtx = (mmutex_t*) mmalloc(sizeof(mmutex_t));
	mmutex_cstr(mtx);
	return mtx;
}

void mmutex_del(mmutex_t* self)
{
	if (!self)
		return;
	mmutex_dstr(self);
	mfree(self);
}

bool mmutex_cstr(mmutex_t* self)
{
	if (!self)
		return false;
	matomicbool_store(&self->value, false);
	return true;
}

void mmutex_dstr(mmutex_t* self)
{
	if (!self)
		return;
	matomicbool_store(&self->value, false);
}

void mmutex_lock(mmutex_t* self)
{
	if (!self)
		return;
	bool val = 0;
	while ((val = matomicbool_compare_exchange_weak(&self->value, false, true)) == true)
		matomicbool_wait(&self->value, val);
}

bool mmutex_try_lock(mmutex_t* self)
{
	if (!self)
		return false;
	return matomicbool_compare_exchange_weak(&self->value, false, true) == false;
}

void mmutex_unlock(mmutex_t* self)
{
	if (!self)
		return;
	matomicbool_store(&self->value, false);
	matomicbool_notify_one(&self->value);
}

void mmutex_lock_busy(mmutex_t* self)
{
	if (!self)
		return;
	bool val = 0;
	while ((val = matomicbool_compare_exchange_weak(&self->value, false, true)) == true)
		;
}

void mmutex_unlock_busy(mmutex_t* self)
{
	if (!self)
		return;
	matomicbool_store(&self->value, false);
}

mrecursive_mutex_t mrecursive_mutex()
{
	mrecursive_mutex_t mtx;
	memset(&mtx, 0, sizeof(mtx));
	mrecursive_mutex_cstr(&mtx);
	return mtx;
}

mrecursive_mutex_t* mrecursive_mutex_new()
{
	mrecursive_mutex_t* mtx = (mrecursive_mutex_t*) mmalloc(sizeof(mrecursive_mutex_t));
	mrecursive_mutex_cstr(mtx);
	return mtx;
}

void mrecursive_mutex_del(mrecursive_mutex_t* self)
{
	if (!self)
		return;
	mrecursive_mutex_dstr(self);
	mfree(self);
}

bool mrecursive_mutex_cstr(mrecursive_mutex_t* self)
{
	if (!self)
		return false;
	matomic64_store(&self->value, 0);
	return true;
}

void mrecursive_mutex_dstr(mrecursive_mutex_t* self)
{
	if (!self)
		return;
	matomic64_store(&self->value, 0);
}

void mrecursive_mutex_lock(mrecursive_mutex_t* self)
{
	if (!self)
		return;
	mthreadid_t tid = mthread_current_id();
	uint64_t    val = matomic64_load(&self->value);
	if (val >> 32 == tid)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	uint64_t value = (uint64_t) tid << 32 | 1;
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, value)) != 0)
		matomic64_wait(&self->value, val);
}

bool mrecursive_mutex_try_lock(mrecursive_mutex_t* self)
{
	if (!self)
		return false;
	mthreadid_t tid = mthread_current_id();
	uint64_t    val = matomic64_load(&self->value);
	if (val >> 32 == tid)
	{
		matomic64_fetch_add(&self->value, 1);
		return true;
	}
	return matomic64_compare_exchange_weak(&self->value, 0, (uint64_t) tid << 32 | 1) == 0;
}

void mrecursive_mutex_unlock(mrecursive_mutex_t* self)
{
	if (!self)
		return;
	if ((matomic64_fetch_sub(&self->value, 1) & 0xFFFFFFFF) == 1)
		matomic64_store(&self->value, 0);
	matomic64_notify_one(&self->value);
}

void mrecursive_mutex_lock_busy(mrecursive_mutex_t* self)
{
	if (!self)
		return;
	mthreadid_t tid = mthread_current_id();
	uint64_t    val = matomic64_load(&self->value);
	if (val >> 32 == tid)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	uint64_t value = (uint64_t) tid << 32 | 1;
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, value)) != 0)
		;
}

void mrecursive_mutex_unlock_busy(mrecursive_mutex_t* self)
{
	if (!self)
		return;
	if ((matomic64_fetch_sub(&self->value, 1) & 0xFFFFFFFF) == 1)
		matomic64_store(&self->value, 0);
}

mshared_mutex_t mshared_mutex()
{
	mshared_mutex_t mtx;
	memset(&mtx, 0, sizeof(mtx));
	mshared_mutex_cstr(&mtx);
	return mtx;
}

mshared_mutex_t* mshared_mutex_new()
{
	mshared_mutex_t* mtx = (mshared_mutex_t*) mmalloc(sizeof(mshared_mutex_t));
	mshared_mutex_cstr(mtx);
	return mtx;
}

void mshared_mutex_del(mshared_mutex_t* self)
{
	if (!self)
		return;
	mshared_mutex_dstr(self);
	mfree(self);
}

bool mshared_mutex_cstr(mshared_mutex_t* self)
{
	if (!self)
		return false;
	matomic64_store(&self->value, 0);
	return true;
}

void mshared_mutex_dstr(mshared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_store(&self->value, 0);
}

void mshared_mutex_lock(mshared_mutex_t* self)
{
	if (!self)
		return;
	uint64_t val = 0;
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, ~0ULL)) != 0)
		matomic64_wait(&self->value, val);
}

bool mshared_mutex_try_lock(mshared_mutex_t* self)
{
	if (!self)
		return false;
	return matomic64_compare_exchange_weak(&self->value, 0, ~0ULL) == 0;
}

void mshared_mutex_unlock(mshared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_store(&self->value, 0);
	matomic64_notify_all(&self->value);
}

void mshared_mutex_shared_lock(mshared_mutex_t* self)
{
	if (!self)
		return;
	uint64_t val = matomic64_load(&self->value);
	if (val > 0 && val != ~0ULL)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, 1)) != 0)
		matomic64_wait(&self->value, val);
}

bool mshared_mutex_try_shared_lock(mshared_mutex_t* self)
{
	if (!self)
		return false;
	uint64_t val = matomic64_load(&self->value);
	if (val > 0 && val != ~0ULL)
	{
		matomic64_fetch_add(&self->value, 1);
		return true;
	}
	return matomic64_compare_exchange_weak(&self->value, 0, 1) == 0;
}

void mshared_mutex_shared_unlock(mshared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_fetch_sub(&self->value, 1);
	matomic64_notify_all(&self->value);
}

void mshared_mutex_lock_busy(mshared_mutex_t* self)
{
	if (!self)
		return;
	uint64_t val = 0;
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, ~0ULL)) != 0)
		;
}

void mshared_mutex_unlock_busy(mshared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_store(&self->value, 0);
}

void mshared_mutex_shared_lock_busy(mshared_mutex_t* self)
{
	if (!self)
		return;
	uint64_t val = matomic64_load(&self->value);
	if (val > 0 && val != ~0ULL)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, 1)) != 0)
		;
}

void mshared_mutex_shared_unlock_busy(mshared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_fetch_sub(&self->value, 1);
}

mrecursive_shared_mutex_t mrecursive_shared_mutex()
{
	mrecursive_shared_mutex_t mtx;
	memset(&mtx, 0, sizeof(mtx));
	mrecursive_shared_mutex_cstr(&mtx);
	return mtx;
}

mrecursive_shared_mutex_t* mrecursive_shared_mutex_new()
{
	mrecursive_shared_mutex_t* mtx = (mrecursive_shared_mutex_t*) mmalloc(sizeof(mrecursive_shared_mutex_t));
	mrecursive_shared_mutex_cstr(mtx);
	return mtx;
}

void mrecursive_shared_mutex_del(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	mrecursive_shared_mutex_dstr(self);
	mfree(self);
}

bool mrecursive_shared_mutex_cstr(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return false;
	matomic64_store(&self->value, 0);
	return true;
}

void mrecursive_shared_mutex_dstr(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_store(&self->value, 0);
}

void mrecursive_shared_mutex_lock(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	mthreadid_t tid = mthread_current_id();
	uint64_t    val = matomic64_load(&self->value);
	if (val >> 32 == tid)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	uint64_t value = (uint64_t) tid << 32 | 1;
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, value)) != 0)
		matomic64_wait(&self->value, val);
}

bool mrecursive_shared_mutex_try_lock(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return false;
	mthreadid_t tid = mthread_current_id();
	uint64_t    val = matomic64_load(&self->value);
	if (val >> 32 == tid)
	{
		matomic64_fetch_add(&self->value, 1);
		return true;
	}
	return matomic64_compare_exchange_weak(&self->value, 0, (uint64_t) tid | 1) == 0;
}

void mrecursive_shared_mutex_unlock(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	if ((matomic64_fetch_sub(&self->value, 1) & 0xFFFFFFFF) == 1)
		matomic64_store(&self->value, 0);
	matomic64_notify_all(&self->value);
}

void mrecursive_shared_mutex_shared_lock(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	uint64_t val = matomic64_load(&self->value);
	if (val > 0 && val <= 0xFFFFFFFF)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, 1)) != 0)
		matomic64_wait(&self->value, val);
}

bool mrecursive_shared_mutex_try_shared_lock(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return false;
	uint64_t val = matomic64_load(&self->value);
	if (val > 0 && val <= 0xFFFFFFFF)
	{
		matomic64_fetch_add(&self->value, 1);
		return true;
	}
	return matomic64_compare_exchange_weak(&self->value, 0, 1) == 0;
}

void mrecursive_shared_mutex_shared_unlock(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_fetch_sub(&self->value, 1);
	matomic64_notify_all(&self->value);
}

void mrecursive_shared_mutex_lock_busy(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	mthreadid_t tid = mthread_current_id();
	uint64_t    val = matomic64_load(&self->value);
	if (val >> 32 == tid)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	uint64_t value = (uint64_t) tid << 32 | 1;
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, value)) != 0)
		;
}

void mrecursive_shared_mutex_unlock_busy(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	if ((matomic64_fetch_sub(&self->value, 1) & 0xFFFFFFFF) == 1)
		matomic64_store(&self->value, 0);
}

void mrecursive_shared_mutex_shared_lock_busy(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	uint64_t val = matomic64_load(&self->value);
	if (val > 0 && val <= 0xFFFFFFFF)
	{
		matomic64_fetch_add(&self->value, 1);
		return;
	}
	while ((val = matomic64_compare_exchange_weak(&self->value, 0, 1)) != 0)
		;
}

void mrecursive_shared_mutex_shared_unlock_busy(mrecursive_shared_mutex_t* self)
{
	if (!self)
		return;
	matomic64_fetch_sub(&self->value, 1);
}