#pragma once

#include "str.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t mtimestamp_t;

typedef mstring_t mpath_t;

typedef enum mfiletype
{
	mfiletype_none = 0,
	mfiletype_file,
	mfiletype_directory,
	mfiletype_symlink
} mfiletype;

typedef struct mfilestatus_t
{
	mtimestamp_t createTime;
	mtimestamp_t lastWriteTime;
	mtimestamp_t lastAccessTime;

	size_t size;
	size_t sizeOnDisk;

	mfiletype type;
} mfilestatus_t;

mpath_t       mpathcstr(const char* str);
mpath_t       mpathv(mstringview_t view);
mpath_t       mpaths(const mstring_t* string);
mpath_t       mpath(size_t initialCapacity);
mpath_t*      mpath_new(size_t initialCapacity);
void          mpath_del(mpath_t* self);
bool          mpath_cstr(mpath_t* self, size_t initialCapacity);
void          mpath_dstr(mpath_t* self);
bool          mpath_clear(mpath_t* self);
bool          mpath_resize(mpath_t* self, size_t newLength);
bool          mpath_reserve(mpath_t* self, size_t newCapacity);
bool          mpath_shrink_to_fit(mpath_t* self);
char*         mpath_begin(mpath_t* self);
char*         mpath_end(mpath_t* self);
char*         mpath_get(mpath_t* self, size_t index);
bool          mpath_set(mpath_t* self, size_t index, char c);
mstringview_t mpath_substr(const mpath_t* self, size_t index, size_t length);
bool          mpath_pushback(mpath_t* self, char c);
bool          mpath_pushfront(mpath_t* self, char c);
bool          mpath_insert(mpath_t* self, size_t index, char c);
bool          mpath_popback(mpath_t* self);
bool          mpath_popfront(mpath_t* self);
bool          mpath_assign(mpath_t* self, mstringview_t string);
bool          mpath_append(mpath_t* self, mstringview_t string);
bool          mpath_prepend(mpath_t* self, mstringview_t string);
bool          mpath_suffix(mpath_t* self, mstringview_t string);
bool          mpath_prefix(mpath_t* self, mstringview_t string);
bool          mpath_replace(mpath_t* self, size_t index, size_t length, mstringview_t replacement);
bool          mpath_erase(mpath_t* self, size_t index, size_t length);
bool          mpath_replace_extension(mpath_t* self, mstringview_t newExtension);
bool          mpath_replace_filename(mpath_t* self, mstringview_t newFilename);
bool          mpath_remove_filename(mpath_t* self);
bool          mpath_absolute(mpath_t* self);
bool          mpath_canonical(mpath_t* self);
bool          mpath_weakly_canonical(mpath_t* self);
bool          mpath_relative(mpath_t* self, mstringview_t root);
bool          mpath_normalize(mpath_t* self);
mstringview_t mpath_rootname(mpath_t* self);
mstringview_t mpath_rootdir(mpath_t* self);
mstringview_t mpath_rootpath(mpath_t* self);
mstringview_t mpath_dir(mpath_t* self);
mstringview_t mpath_filename(mpath_t* self);
mstringview_t mpath_stem(mpath_t* self);
mstringview_t mpath_extension(mpath_t* self);
mstringview_t mpath_parent(mpath_t* self);
bool          mpath_is_absolute(mpath_t* self);
bool          mpath_is_relative(mpath_t* self);
size_t        mpath_count_elements(mpath_t* self);
int           mpath_compare(const mpath_t* lhs, const mpath_t* rhs);

mstringview_t mstringviewp(const mpath_t* path);
mstringview_t mpathview_rootname(mstringview_t path);
mstringview_t mpathview_rootdir(mstringview_t path);
mstringview_t mpathview_rootpath(mstringview_t path);
mstringview_t mpathview_dir(mstringview_t path);
mstringview_t mpathview_filename(mstringview_t path);
mstringview_t mpathview_stem(mstringview_t path);
mstringview_t mpathview_extension(mstringview_t path);
mstringview_t mpathview_parent(mstringview_t path);
size_t        mpathview_count_elements(mstringview_t path);

mpath_t mfile_exec();
mpath_t mfile_cwd();
mpath_t mfile_temp();
void    mfile_set_cwd(mstringview_t path);
bool    mfile_copy(mstringview_t from, mstringview_t to);
bool    mfile_copy_file(mstringview_t from, mstringview_t to);
bool    mfile_create_directory(mstringview_t path);
bool    mfile_create_directories(mstringview_t path);
bool    mfile_remove(mstringview_t path);
size_t  mfile_remove_all(mstringview_t path);
bool    mfile_rename(mstringview_t from, mstringview_t to);
bool    mfile_exists(mstringview_t path);
bool    mfile_status(mstringview_t path, mfilestatus_t* status);
bool    mfile_set_status(mstringview_t path, const mfilestatus_t* status);