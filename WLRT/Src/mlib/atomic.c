#include "atomic.h"
#include "build.h"
#include "thread.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#include <intrin.h>
	#include <Windows.h>
#else
#endif

#if BUILD_IS_PLATFORM_X86
	#define ATOMICPTR_SELECT(func, ...) matomic32_##func(__VA_ARGS__)
	#define ATOMICPTR_TYPE              uint32_t
#elif BUILD_IS_PLATFORM_AMD64
	#define ATOMICPTR_SELECT(func, ...) matomic64_##func(__VA_ARGS__)
	#define ATOMICPTR_TYPE              uint64_t
#else
#endif

#define ATOMICBOOL_SELECT(func, ...) matomic8_##func(__VA_ARGS__)
#define ATOMICBOOL_TYPE              uint8_t

void matomic_wait(volatile void* address, const void* expected, size_t size)
{
	mwait_on_address(address, expected, size);
}

void matomic8_wait(volatile uint8_t* memory, uint8_t expected)
{
	matomic_wait(memory, &expected, sizeof(expected));
}

void matomic16_wait(volatile uint16_t* memory, uint16_t expected)
{
	matomic_wait(memory, &expected, sizeof(expected));
}

void matomic32_wait(volatile uint32_t* memory, uint32_t expected)
{
	matomic_wait(memory, &expected, sizeof(expected));
}

void matomic64_wait(volatile uint64_t* memory, uint64_t expected)
{
	matomic_wait(memory, &expected, sizeof(expected));
}

void matomicbool_wait(volatile bool* memory, bool expected)
{
	ATOMICBOOL_SELECT(wait, (volatile ATOMICBOOL_TYPE*) memory, (ATOMICBOOL_TYPE) expected);
}

void matomicptr_wait(void* volatile* memory, void* expected)
{
	ATOMICPTR_SELECT(wait, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) expected);
}

void matomic_wake_one(volatile void* address)
{
	mwake_by_address_one(address);
}

void matomic_wake_all(volatile void* address)
{
	mwake_by_address_all(address);
}

void matomic8_notify_one(volatile uint8_t* memory)
{
	matomic_wake_one((volatile void*) memory);
}

void matomic16_notify_one(volatile uint16_t* memory)
{
	matomic_wake_one((volatile void*) memory);
}

void matomic32_notify_one(volatile uint32_t* memory)
{
	matomic_wake_one((volatile void*) memory);
}

void matomic64_notify_one(volatile uint64_t* memory)
{
	matomic_wake_one((volatile void*) memory);
}

void matomicbool_notify_one(volatile bool* memory)
{
	ATOMICBOOL_SELECT(notify_one, (volatile ATOMICBOOL_TYPE*) memory);
}

void matomicptr_notify_one(void* volatile* memory)
{
	ATOMICPTR_SELECT(notify_one, (volatile ATOMICPTR_TYPE*) memory);
}

void matomic8_notify_all(volatile uint8_t* memory)
{
	matomic_wake_all((volatile void*) memory);
}

void matomic16_notify_all(volatile uint16_t* memory)
{
	matomic_wake_all((volatile void*) memory);
}

void matomic32_notify_all(volatile uint32_t* memory)
{
	matomic_wake_all((volatile void*) memory);
}

void matomic64_notify_all(volatile uint64_t* memory)
{
	matomic_wake_all((volatile void*) memory);
}

void matomicbool_notify_all(volatile bool* memory)
{
	ATOMICBOOL_SELECT(notify_all, (volatile ATOMICBOOL_TYPE*) memory);
}

void matomicptr_notify_all(void* volatile* memory)
{
	ATOMICPTR_SELECT(notify_all, (volatile ATOMICPTR_TYPE*) memory);
}

void matomic8_store(volatile uint8_t* memory, uint8_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	*memory = value;
#else
#endif
}

void matomic16_store(volatile uint16_t* memory, uint16_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	*memory = value;
#else
#endif
}

void matomic32_store(volatile uint32_t* memory, uint32_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	*memory = value;
#else
#endif
}

void matomic64_store(volatile uint64_t* memory, uint64_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	*memory = value;
#else
#endif
}

void matomicbool_store(volatile bool* memory, bool value)
{
	ATOMICBOOL_SELECT(store, (volatile ATOMICBOOL_TYPE*) memory, (ATOMICBOOL_TYPE) value);
}

void matomicptr_store(void* volatile* memory, void* value)
{
	ATOMICPTR_SELECT(store, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_load(volatile uint8_t* memory)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return *memory;
#else
#endif
}

uint16_t matomic16_load(volatile uint16_t* memory)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return *memory;
#else
#endif
}

uint32_t matomic32_load(volatile uint32_t* memory)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return *memory;
#else
#endif
}

uint64_t matomic64_load(volatile uint64_t* memory)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return *memory;
#else
#endif
}

bool matomicbool_load(volatile bool* memory)
{
	return (bool) ATOMICBOOL_SELECT(load, (volatile ATOMICBOOL_TYPE*) memory);
}

void* matomicptr_load(void* volatile* memory)
{
	return (void*) ATOMICPTR_SELECT(load, (volatile ATOMICPTR_TYPE*) memory);
}

uint8_t matomic8_exchange(volatile uint8_t* memory, uint8_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint8_t) _InterlockedExchange8((volatile char*) memory, (char) value);
#else
#endif
}

uint16_t matomic16_exchange(volatile uint16_t* memory, uint16_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint16_t) _InterlockedExchange16((volatile short*) memory, (short) value);
#else
#endif
}

uint32_t matomic32_exchange(volatile uint32_t* memory, uint32_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint32_t) _InterlockedExchange((volatile long*) memory, (long) value);
#else
#endif
}

uint64_t matomic64_exchange(volatile uint64_t* memory, uint64_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) _InterlockedExchange64((volatile long long*) memory, (long long) value);
#else
#endif
}

bool matomicbool_exchange(volatile bool* memory, bool value)
{
	return (bool) ATOMICBOOL_SELECT(exchange, (volatile ATOMICBOOL_TYPE*) memory, (ATOMICBOOL_TYPE) value);
}

void* matomicptr_exchange(void* volatile* memory, void* value)
{
	return (void*) ATOMICPTR_SELECT(exchange, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_compare_exchange_strong(volatile uint8_t* memory, uint8_t expected, uint8_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint8_t) _InterlockedCompareExchange8((volatile char*) memory, (char) value, (char) expected);
#else
#endif
}

uint16_t matomic16_compare_exchange_strong(volatile uint16_t* memory, uint16_t expected, uint16_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint16_t) _InterlockedCompareExchange16((volatile short*) memory, (short) value, (short) expected);
#else
#endif
}

uint32_t matomic32_compare_exchange_strong(volatile uint32_t* memory, uint32_t expected, uint32_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint32_t) _InterlockedCompareExchange((volatile long*) memory, (long) value, (long) expected);
#else
#endif
}

uint64_t matomic64_compare_exchange_strong(volatile uint64_t* memory, uint64_t expected, uint64_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) _InterlockedCompareExchange64((volatile long long*) memory, (long long) value, (long long) expected);
#else
#endif
}

bool matomicbool_compare_exchange_strong(volatile bool* memory, bool expected, bool value)
{
	return (bool) ATOMICBOOL_SELECT(compare_exchange_strong, (volatile ATOMICBOOL_TYPE*) memory, (ATOMICBOOL_TYPE) expected, (ATOMICBOOL_TYPE) value);
}

void* matomicptr_compare_exchange_strong(void* volatile* memory, void* expected, void* value)
{
	return (void*) ATOMICPTR_SELECT(compare_exchange_strong, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) expected, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_compare_exchange_weak(volatile uint8_t* memory, uint8_t expected, uint8_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return matomic8_compare_exchange_strong(memory, expected, value);
#else
#endif
}

uint16_t matomic16_compare_exchange_weak(volatile uint16_t* memory, uint16_t expected, uint16_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return matomic16_compare_exchange_strong(memory, expected, value);
#else
#endif
}

uint32_t matomic32_compare_exchange_weak(volatile uint32_t* memory, uint32_t expected, uint32_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return matomic32_compare_exchange_strong(memory, expected, value);
#else
#endif
}

uint64_t matomic64_compare_exchange_weak(volatile uint64_t* memory, uint64_t expected, uint64_t value)
{
#if BUILD_IS_PLATFORM_X86 || BUILD_IS_PLATFORM_AMD64
	return matomic64_compare_exchange_strong(memory, expected, value);
#else
#endif
}

bool matomicbool_compare_exchange_weak(volatile bool* memory, bool expected, bool value)
{
	return (bool) ATOMICBOOL_SELECT(compare_exchange_weak, (volatile ATOMICBOOL_TYPE*) memory, (ATOMICBOOL_TYPE) expected, (ATOMICBOOL_TYPE) value);
}

void* matomicptr_compare_exchange_weak(void* volatile* memory, void* expected, void* value)
{
	return (void*) ATOMICPTR_SELECT(compare_exchange_weak, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) expected, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_fetch_add(volatile uint8_t* memory, uint8_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint8_t) _InterlockedExchangeAdd8((volatile char*) memory, (char) value);
#else
#endif
}

uint16_t matomic16_fetch_add(volatile uint16_t* memory, uint16_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint16_t) _InterlockedExchangeAdd16((volatile short*) memory, (short) value);
#else
#endif
}

uint32_t matomic32_fetch_add(volatile uint32_t* memory, uint32_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint32_t) _InterlockedExchangeAdd((volatile long*) memory, (long) value);
#else
#endif
}

uint64_t matomic64_fetch_add(volatile uint64_t* memory, uint64_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) _InterlockedExchangeAdd64((volatile long long*) memory, (long long) value);
#else
#endif
}

void* matomicptr_fetch_add(void* volatile* memory, void* value)
{
	return (void*) ATOMICPTR_SELECT(fetch_add, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_fetch_sub(volatile uint8_t* memory, uint8_t value)
{
	return matomic8_fetch_add(memory, ~value + 1);
}

uint16_t matomic16_fetch_sub(volatile uint16_t* memory, uint16_t value)
{
	return matomic16_fetch_add(memory, ~value + 1);
}

uint32_t matomic32_fetch_sub(volatile uint32_t* memory, uint32_t value)
{
	return matomic32_fetch_add(memory, ~value + 1);
}

uint64_t matomic64_fetch_sub(volatile uint64_t* memory, uint64_t value)
{
	return matomic64_fetch_add(memory, ~value + 1);
}

void* matomicptr_fetch_sub(void* volatile* memory, void* value)
{
	return (void*) ATOMICPTR_SELECT(fetch_sub, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_fetch_or(volatile uint8_t* memory, uint8_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint8_t) _InterlockedOr8((volatile char*) memory, (char) value);
#else
#endif
}

uint16_t matomic16_fetch_or(volatile uint16_t* memory, uint16_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint16_t) _InterlockedOr16((volatile short*) memory, (short) value);
#else
#endif
}

uint32_t matomic32_fetch_or(volatile uint32_t* memory, uint32_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint32_t) _InterlockedOr((volatile long*) memory, (long) value);
#else
#endif
}

uint64_t matomic64_fetch_or(volatile uint64_t* memory, uint64_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) _InterlockedOr64((volatile long long*) memory, (long long) value);
#else
#endif
}

void* matomicptr_fetch_or(void* volatile* memory, void* value)
{
	return (void*) ATOMICPTR_SELECT(fetch_or, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_fetch_xor(volatile uint8_t* memory, uint8_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint8_t) _InterlockedXor8((volatile char*) memory, (char) value);
#else
#endif
}

uint16_t matomic16_fetch_xor(volatile uint16_t* memory, uint16_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint16_t) _InterlockedXor16((volatile short*) memory, (short) value);
#else
#endif
}

uint32_t matomic32_fetch_xor(volatile uint32_t* memory, uint32_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint32_t) _InterlockedXor((volatile long*) memory, (long) value);
#else
#endif
}

uint64_t matomic64_fetch_xor(volatile uint64_t* memory, uint64_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) _InterlockedXor64((volatile long long*) memory, (long long) value);
#else
#endif
}

void* matomicptr_fetch_xor(void* volatile* memory, void* value)
{
	return (void*) ATOMICPTR_SELECT(fetch_xor, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) value);
}

uint8_t matomic8_fetch_and(volatile uint8_t* memory, uint8_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint8_t) _InterlockedAnd8((volatile char*) memory, (char) value);
#else
#endif
}

uint16_t matomic16_fetch_and(volatile uint16_t* memory, uint16_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint16_t) _InterlockedAnd16((volatile short*) memory, (short) value);
#else
#endif
}

uint32_t matomic32_fetch_and(volatile uint32_t* memory, uint32_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint32_t) _InterlockedAnd((volatile long*) memory, (long) value);
#else
#endif
}

uint64_t matomic64_fetch_and(volatile uint64_t* memory, uint64_t value)
{
#if BUILD_IS_SYSTEM_WINDOWS
	return (uint64_t) _InterlockedAnd64((volatile long long*) memory, (long long) value);
#else
#endif
}

void* matomicptr_fetch_and(void* volatile* memory, void* value)
{
	return (void*) ATOMICPTR_SELECT(fetch_and, (volatile ATOMICPTR_TYPE*) memory, (ATOMICPTR_TYPE) value);
}