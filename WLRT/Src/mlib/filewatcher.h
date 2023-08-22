#pragma once

#include "filesystem.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum mfilewatcher_action_e
{
	mfilewatcher_action_create = 0,
	mfilewatcher_action_modify,
	mfilewatcher_action_delete,
	mfilewatcher_action_rename
} mfilewatcher_action_e;

typedef struct mfilewatcher_data_t
{
	mfilewatcher_action_e action;
	void*                 data;
	const mpath_t*        file;
	const mpath_t*        newFile;
	uint64_t              id;
} mfilewatcher_data_t;

typedef void (*mfilewatcher_func_t)(const mfilewatcher_data_t* data);

bool mfilewatcher_init();
void mfilewatcher_deinit();

uint64_t mfilewatcher_watch(const mpath_t* file, mfilewatcher_func_t func, void* data);
void     mfilewatcher_unwatch(uint64_t id);