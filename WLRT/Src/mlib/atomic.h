#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

void matomic_wait(volatile void* address, const void* expected, size_t size);
void matomic8_wait(volatile uint8_t* memory, uint8_t expected);
void matomic16_wait(volatile uint16_t* memory, uint16_t expected);
void matomic32_wait(volatile uint32_t* memory, uint32_t expected);
void matomic64_wait(volatile uint64_t* memory, uint64_t expected);
void matomicbool_wait(volatile bool* memory, bool expected);
void matomicptr_wait(void* volatile* memory, void* expected);

void matomic_wake_one(volatile void* address);
void matomic_wake_all(volatile void* address);
void matomic8_notify_one(volatile uint8_t* memory);
void matomic16_notify_one(volatile uint16_t* memory);
void matomic32_notify_one(volatile uint32_t* memory);
void matomic64_notify_one(volatile uint64_t* memory);
void matomicbool_notify_one(volatile bool* memory);
void matomicptr_notify_one(void* volatile* memory);
void matomic8_notify_all(volatile uint8_t* memory);
void matomic16_notify_all(volatile uint16_t* memory);
void matomic32_notify_all(volatile uint32_t* memory);
void matomic64_notify_all(volatile uint64_t* memory);
void matomicbool_notify_all(volatile bool* memory);
void matomicptr_notify_all(void* volatile* memory);

void matomic8_store(volatile uint8_t* memory, uint8_t value);
void matomic16_store(volatile uint16_t* memory, uint16_t value);
void matomic32_store(volatile uint32_t* memory, uint32_t value);
void matomic64_store(volatile uint64_t* memory, uint64_t value);
void matomicbool_store(volatile bool* memory, bool value);
void matomicptr_store(void* volatile* memory, void* value);

uint8_t  matomic8_load(volatile uint8_t* memory);
uint16_t matomic16_load(volatile uint16_t* memory);
uint32_t matomic32_load(volatile uint32_t* memory);
uint64_t matomic64_load(volatile uint64_t* memory);
bool     matomicbool_load(volatile bool* memory);
void*    matomicptr_load(void* volatile* memory);

uint8_t  matomic8_exchange(volatile uint8_t* memory, uint8_t value);
uint16_t matomic16_exchange(volatile uint16_t* memory, uint16_t value);
uint32_t matomic32_exchange(volatile uint32_t* memory, uint32_t value);
uint64_t matomic64_exchange(volatile uint64_t* memory, uint64_t value);
bool     matomicbool_exchange(volatile bool* memory, bool value);
void*    matomicptr_exchange(void* volatile* memory, void* value);

uint8_t  matomic8_compare_exchange_strong(volatile uint8_t* memory, uint8_t expected, uint8_t value);
uint16_t matomic16_compare_exchange_strong(volatile uint16_t* memory, uint16_t expected, uint16_t value);
uint32_t matomic32_compare_exchange_strong(volatile uint32_t* memory, uint32_t expected, uint32_t value);
uint64_t matomic64_compare_exchange_strong(volatile uint64_t* memory, uint64_t expected, uint64_t value);
bool     matomicbool_compare_exchange_strong(volatile bool* memory, bool expected, bool value);
void*    matomicptr_compare_exchange_strong(void* volatile* memory, void* expected, void* value);
uint8_t  matomic8_compare_exchange_weak(volatile uint8_t* memory, uint8_t expected, uint8_t value);
uint16_t matomic16_compare_exchange_weak(volatile uint16_t* memory, uint16_t expected, uint16_t value);
uint32_t matomic32_compare_exchange_weak(volatile uint32_t* memory, uint32_t expected, uint32_t value);
uint64_t matomic64_compare_exchange_weak(volatile uint64_t* memory, uint64_t expected, uint64_t value);
bool     matomicbool_compare_exchange_weak(volatile bool* memory, bool expected, bool value);
void*    matomicptr_compare_exchange_weak(void* volatile* memory, void* expected, void* value);

uint8_t  matomic8_fetch_add(volatile uint8_t* memory, uint8_t value);
uint16_t matomic16_fetch_add(volatile uint16_t* memory, uint16_t value);
uint32_t matomic32_fetch_add(volatile uint32_t* memory, uint32_t value);
uint64_t matomic64_fetch_add(volatile uint64_t* memory, uint64_t value);
void*    matomicptr_fetch_add(void* volatile* memory, void* value);
uint8_t  matomic8_fetch_sub(volatile uint8_t* memory, uint8_t value);
uint16_t matomic16_fetch_sub(volatile uint16_t* memory, uint16_t value);
uint32_t matomic32_fetch_sub(volatile uint32_t* memory, uint32_t value);
uint64_t matomic64_fetch_sub(volatile uint64_t* memory, uint64_t value);
void*    matomicptr_fetch_sub(void* volatile* memory, void* value);

uint8_t  matomic8_fetch_or(volatile uint8_t* memory, uint8_t value);
uint16_t matomic16_fetch_or(volatile uint16_t* memory, uint16_t value);
uint32_t matomic32_fetch_or(volatile uint32_t* memory, uint32_t value);
uint64_t matomic64_fetch_or(volatile uint64_t* memory, uint64_t value);
void*    matomicptr_fetch_or(void* volatile* memory, void* value);

uint8_t  matomic8_fetch_xor(volatile uint8_t* memory, uint8_t value);
uint16_t matomic16_fetch_xor(volatile uint16_t* memory, uint16_t value);
uint32_t matomic32_fetch_xor(volatile uint32_t* memory, uint32_t value);
uint64_t matomic64_fetch_xor(volatile uint64_t* memory, uint64_t value);
void*    matomicptr_fetch_xor(void* volatile* memory, void* value);

uint8_t  matomic8_fetch_and(volatile uint8_t* memory, uint8_t value);
uint16_t matomic16_fetch_and(volatile uint16_t* memory, uint16_t value);
uint32_t matomic32_fetch_and(volatile uint32_t* memory, uint32_t value);
uint64_t matomic64_fetch_and(volatile uint64_t* memory, uint64_t value);
void*    matomicptr_fetch_and(void* volatile* memory, void* value);
