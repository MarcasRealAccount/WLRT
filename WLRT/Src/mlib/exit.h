#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef void (*mexit_func_t)(void* data);

bool mexit_init();
void mexit_deinit();
void mexit_register(mexit_func_t func, void* data);
void massert_register(bool statement, uint64_t exitCode, mexit_func_t func, void* data);
void mexit_handle();
void mexit(uint64_t exitCode);
void massert(bool statement, uint64_t exitCode);