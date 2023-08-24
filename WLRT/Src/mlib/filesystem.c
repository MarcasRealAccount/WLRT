#include "filesystem.h"
#include "build.h"
#include "mem.h"
#include "mat.h"

#include <stdlib.h>
#include <string.h>

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>
#elif BUILD_IS_SYSTEM_UNIX
	#include <fcntl.h>
	#include <utime.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <stdio.h>
	#if BUILD_IS_SYSTEM_MACOSX
		#include <copyfile.h>
		#include <mach-o/dyld.h>
	#else
		#include <sys/sendfile.h>
	#endif
#else
#endif

mpath_t mpathcstr(const char* str)
{
	return mpathv(mstringviewcstr(str));
}

mpath_t mpathv(mstringview_t view)
{
	mpath_t out;
	memset(&out, 0, sizeof(out));
	mpath_assign(&out, view);
	return out;
}

mpath_t mpaths(const mstring_t* string)
{
	return mpathv(mstringviews(string));
}

mpath_t mpath(size_t initialCapacity)
{
	mpath_t out;
	memset(&out, 0, sizeof(out));
	mpath_cstr(&out, initialCapacity);
	return out;
}

mpath_t* mpath_new(size_t initialCapacity)
{
	mpath_t* out = (mpath_t*) mmalloc(sizeof(mpath_t));
	mpath_cstr(out, initialCapacity);
	return out;
}

void mpath_del(mpath_t* self)
{
	if (!self)
		return;
	mpath_dstr(self);
	mfree(self);
}

bool mpath_cstr(mpath_t* self, size_t initialCapacity)
{
	return mstring_cstr((mstring_t*) self, initialCapacity);
}

void mpath_dstr(mpath_t* self)
{
	mstring_dstr((mstring_t*) self);
}

bool mpath_clear(mpath_t* self)
{
	return mstring_clear((mstring_t*) self);
}

bool mpath_resize(mpath_t* self, size_t newLength)
{
	return mstring_resize((mstring_t*) self, newLength);
}

bool mpath_reserve(mpath_t* self, size_t newCapacity)
{
	return mstring_reserve((mstring_t*) self, newCapacity);
}

bool mpath_shrink_to_fit(mpath_t* self)
{
	return mstring_shrink_to_fit((mstring_t*) self);
}

char* mpath_begin(mpath_t* self)
{
	return mstring_begin((mstring_t*) self);
}

char* mpath_end(mpath_t* self)
{
	return mstring_end((mstring_t*) self);
}

char* mpath_get(mpath_t* self, size_t index)
{
	return mstring_get((mstring_t*) self, index);
}

bool mpath_set(mpath_t* self, size_t index, char c)
{
	return mstring_set((mstring_t*) self, index, c);
}

mstringview_t mpath_substr(const mpath_t* self, size_t index, size_t length)
{
	return mstring_substr((const mstring_t*) self, index, length);
}

bool mpath_pushback(mpath_t* self, char c)
{
	return mstring_pushback((mstring_t*) self, c);
}

bool mpath_pushfront(mpath_t* self, char c)
{
	return mstring_pushfront((mstring_t*) self, c);
}

bool mpath_insert(mpath_t* self, size_t index, char c)
{
	return mstring_insert((mstring_t*) self, index, c);
}

bool mpath_popback(mpath_t* self)
{
	return mstring_popback((mstring_t*) self);
}

bool mpath_popfront(mpath_t* self)
{
	return mstring_popfront((mstring_t*) self);
}

bool mpath_assign(mpath_t* self, mstringview_t string)
{
	return mstring_assign((mstring_t*) self, string);
}

// TODO(MarcasRealAccount): Optimize mpath_append, mpath_prepend to a single operation
bool mpath_append(mpath_t* self, mstringview_t string)
{
	if (!self ||
		!mstring_pushback((mstring_t*) self, '/') ||
		!mstring_append((mstring_t*) self, string))
		return false;
	return true;
}

bool mpath_prepend(mpath_t* self, mstringview_t string)
{
	if (!self ||
		!mstring_pushfront((mstring_t*) self, '/') ||
		!mstring_prepend((mstring_t*) self, string))
		return false;
	return true;
}

bool mpath_suffix(mpath_t* self, mstringview_t string)
{
	return mstring_append((mstring_t*) self, string);
}

bool mpath_prefix(mpath_t* self, mstringview_t string)
{
	return mstring_prepend((mstring_t*) self, string);
}

bool mpath_replace(mpath_t* self, size_t index, size_t length, mstringview_t replacement)
{
	return mstring_replace((mstring_t*) self, index, length, replacement);
}

bool mpath_erase(mpath_t* self, size_t index, size_t length)
{
	return mstring_erase((mstring_t*) self, index, length);
}

bool mpath_replace_extension(mpath_t* self, mstringview_t newExtension)
{
	if (!self)
		return false;

	size_t dot   = self->length;
	char*  begin = mpath_begin(self);
	for (size_t i = self->length - 1; i != ~0ULL; --i)
	{
		char a = begin[i];
		if (a == '.')
		{
			dot = i;
			break;
		}
		else if (a == '/')
		{
			break;
		}
	}
	return mpath_replace(self, dot, self->length - dot, newExtension);
}

bool mpath_replace_filename(mpath_t* self, mstringview_t newFilename)
{
	if (!self)
		return false;

	size_t slash = mstring_find_last_of((const mstring_t*) self, mstringviewcstr("/"), 0);
	if (slash < self->length)
		++slash;
	return mpath_replace(self, slash, self->length - slash, newFilename);
}

bool mpath_remove_filename(mpath_t* self)
{
	return mpath_replace_filename(self, mstringviewcstr(NULL));
}

bool mpath_absolute(mpath_t* self)
{
	if (!self)
		return false;

	if (mpath_is_absolute(self))
		return true;

	mpath_t cwd = mfile_cwd();
	if (self->length > 0)
		mpath_prepend(self, mstringviewp(&cwd));
	else
		mpath_assign(self, mstringviewp(&cwd));
	mpath_dstr(&cwd);
	return true;
}

bool mpath_canonical(mpath_t* self)
{
	if (!self)
		return false;

	if (!mpath_absolute(self))
		return false;

#if BUILD_IS_SYSTEM_WINDOWS
	if (self->length > 32767)
		return false;

	wchar_t wbuffer[32768];
	if (!wbuffer)
		return false;
	int newLen      = MultiByteToWideChar(CP_UTF8, 0, mpath_begin(self), (int) self->length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';
	HANDLE handle   = CreateFileW(wbuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (handle)
	{
		DWORD len = GetFinalPathNameByHandleW(handle, wbuffer, 32767, 0);
		for (size_t i = 0; i < len; ++i)
		{
			if (wbuffer[i] == L'\\')
				wbuffer[i] = L'/';
		}
		newLen = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, NULL, 0, NULL, NULL);
		mpath_reserve(self, newLen);
		newLen = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, mpath_begin(self), newLen, NULL, NULL);
		mpath_resize(self, newLen);
		mpath_shrink_to_fit(self);
		CloseHandle(handle);
	}
#elif BUILD_IS_SYSTEM_UNIX
	char buf[4097];
	realpath(mpath_begin(self), buf);
	mpath_assign(self, mstringviewcstr(buf));
	mpath_shrink_to_fit(self);
#else
#endif
	return true;
}

bool mpath_weakly_canonical(mpath_t* self)
{
	if (!self)
		return false;

	if (!mpath_absolute(self))
		return false;

	mstringview_t parentPath = mstringviewp(self);
	while (parentPath.length > 0)
	{
		if (mfile_exists(parentPath))
			break;
		parentPath = mpathview_parent(parentPath);
	}
	if (parentPath.length == 0)
		return true;

	mpath_t copy = mpathv(parentPath);
	mpath_canonical(&copy);
	mpath_erase(self, 0, parentPath.length);
	mpath_normalize(self);
	mpath_prepend(self, mstringviewp(&copy));
	mpath_dstr(&copy);
	return true;
}

bool mpath_relative(mpath_t* self, mstringview_t root)
{
	if (!self ||
		!mpath_weakly_canonical(self))
		return false;

	mpath_t rootCopy = mpathv(mpathview_dir(root));
	if (!mpath_weakly_canonical(&rootCopy) ||
		mstringview_compare(mpath_rootpath(self), mpath_rootpath(&rootCopy)) != 0)
	{
		mpath_dstr(&rootCopy);
		return false;
	}
	size_t i      = 0;
	char*  begin1 = mpath_begin(self);
	char*  begin2 = mpath_begin(&rootCopy);
	for (; i < mmin(self->length, rootCopy.length); ++i)
	{
		if (begin1[i] != begin2[i])
			break;
	}
	if (i >= self->length)
	{
		mpath_dstr(&rootCopy);
		return false;
	}
	mpath_erase(self, 0, i);
	if (i >= rootCopy.length)
	{
		mpath_dstr(&rootCopy);
		return true;
	}
	size_t elements = mpathview_count_elements(mpath_substr(&rootCopy, i, rootCopy.length - i));
	mpath_dstr(&rootCopy);
	for (i = 0; i < elements; ++i)
		mpath_prefix(self, mstringviewcstr("../"));
	return true;
}

bool mpath_normalize(mpath_t* self)
{
	if (!self)
		return false;

	if (self->length == 0)
		return true;

	size_t offset = mpath_rootpath(self).length;
	char*  begin  = mpath_begin(self);
	while (offset < self->length)
	{
		size_t end = mstring_find_first_of((const mstring_t*) self, mstringviewcstr("/"), offset);
		if (end >= self->length)
			break;
		size_t len = end - offset;
		switch (len)
		{
		case 0: // Can only be /
			mpath_erase(self, offset, 1);
			break;
		case 1: // Can be ./
			if (begin[offset] == '.')
				mpath_erase(self, offset, 2);
			else
				offset = end + 1;
			break;
		case 2: // Can be ../
			if (offset > 1 && begin[offset] == '.' && begin[offset + 1] == '.')
			{
				size_t previousOffset = mstring_find_last_of((const mstring_t*) self, mstringviewcstr("/"), offset - 2);
				if (previousOffset < self->length)
					mpath_erase(self, previousOffset + 1, 3);
				else
					offset = end + 1;
			}
			else
			{
				offset = end + 1;
			}
			break;
		default:
			offset = end + 1;
			break;
		}
	}
	switch (self->length - offset)
	{
	case 0: // Can only be /
		mpath_erase(self, offset, 1);
		break;
	case 1: // Can be ./
		if (begin[offset] == '.')
			mpath_erase(self, offset, 2);
		break;
	case 2: // Can be ../
		if (offset > 1 && begin[offset] == '.' && begin[offset + 1] == '.')
		{
			size_t previousOffset = mstring_find_last_of((const mstring_t*) self, mstringviewcstr("/"), offset - 2);
			if (previousOffset < self->length)
				mpath_erase(self, previousOffset, 3);
		}
		break;
	}
	if (self->length == 0)
		mpath_assign(self, mstringviewcstr("./"));
	return true;
}

mstringview_t mpath_rootname(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_rootname(mstringviewp(self));
}

mstringview_t mpath_rootdir(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_rootdir(mstringviewp(self));
}

mstringview_t mpath_rootpath(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_rootpath(mstringviewp(self));
}

mstringview_t mpath_dir(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_dir(mstringviewp(self));
}

mstringview_t mpath_filename(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_filename(mstringviewp(self));
}

mstringview_t mpath_stem(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_stem(mstringviewp(self));
}

mstringview_t mpath_extension(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_extension(mstringviewp(self));
}

mstringview_t mpath_parent(mpath_t* self)
{
	if (!self)
		return mstringviewcstr(NULL);
	return mpathview_parent(mstringviewp(self));
}

bool mpath_is_absolute(mpath_t* self)
{
	if (!self)
		return false;

	return mpath_rootpath(self).length > 0;
}

bool mpath_is_relative(mpath_t* self)
{
	return self && !mpath_is_absolute(self);
}

size_t mpath_count_elements(mpath_t* self)
{
	return self ? mpathview_count_elements(mstringviewp(self)) : 0;
}

int mpath_compare(const mpath_t* lhs, const mpath_t* rhs)
{
	return mstring_compare((const mstring_t*) lhs, mstringviewp(rhs));
}

mstringview_t mstringviewp(const mpath_t* path)
{
	return mstringviews((const mstring_t*) path);
}

mstringview_t mpathview_rootname(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);
#if BUILD_IS_SYSTEM_WINDOWS
	if (path.length < 2)
		return mstringviewcstr(NULL);
	const char* begin = mstringview_begin(path);
	if (begin[0] == '/' || begin[1] == '/')
	{
		return mstringview_substr(path, 0, mstringview_find_first_of(path, mstringviewcstr("/"), 2));
	}
	else if (begin[1] == ':' &&
			 ((begin[0] >= 'A' && begin[0] <= 'Z') ||
			  (begin[0] >= 'a' && begin[0] <= 'z')))
	{
		return mstringview_substr(path, 0, 2);
	}
	return mstringviewcstr(NULL);
#else
	return mstringviewcstr(NULL);
#endif
}

mstringview_t mpathview_rootdir(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);
#if BUILD_IS_SYSTEM_WINDOWS
	if (path.length < 3)
		return mstringviewcstr(NULL);
	const char* begin = mstringview_begin(path);
	if (begin[0] == '/' || begin[1] == '/')
	{
		size_t end = mstringview_find_first_of(path, mstringviewcstr("/"), 2);
		return mstringview_substr(path, end, 1);
	}
	else if (begin[1] == ':' &&
			 ((begin[0] >= 'A' && begin[0] <= 'Z') ||
			  (begin[0] >= 'a' && begin[0] <= 'z')))
	{
		return mstringview_substr(path, 3, 1);
	}
	return mstringviewcstr(NULL);
#else
	if (path.length == 0 || path.string[0] != '/')
		return mstringviewcstr(NULL);
	return mstringview_substr(path, 0, 1);
#endif
}

mstringview_t mpathview_rootpath(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);
#if BUILD_IS_SYSTEM_WINDOWS
	if (path.length < 2)
		return mstringviewcstr(NULL);
	const char* begin = mstringview_begin(path);
	if (begin[0] == '/' || begin[1] == '/')
	{
		size_t end = mstringview_find_first_of(path, mstringviewcstr("/"), 2);
		if (end < path.length)
			++end;
		return mstringview_substr(path, 0, end);
	}
	else if (begin[1] == ':' &&
			 ((begin[0] >= 'A' && begin[0] <= 'Z') ||
			  (begin[0] >= 'a' && begin[0] <= 'z')))
	{
		if (path.length > 2 && begin[2] == '/')
			return mstringview_substr(path, 0, 3);
		else
			return mstringview_substr(path, 0, 2);
	}
	return mstringviewcstr(NULL);
#else
	if (path.length == 0 || path.string[0] != '/')
		return mstringviewcstr(NULL);
	return mstringview_substr(path, 0, 1);
#endif
}

mstringview_t mpathview_dir(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);

	size_t slash = mstringview_find_last_of(path, mstringviewcstr("/"), 0);
	return mstringview_substr(path, 0, slash + 1);
}

mstringview_t mpathview_filename(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);

	size_t slash = mstringview_find_last_of(path, mstringviewcstr("/"), 0);
	if (slash < path.length)
		++slash;
	return mstringview_substr(path, slash, path.length - slash);
}

mstringview_t mpathview_stem(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);

	size_t slash = mstringview_find_last_of(path, mstringviewcstr("/"), 0);
	size_t dot   = mstringview_find_last_of(path, mstringviewcstr("."), 0);
	if (slash < path.length)
		++slash;
	if (dot < slash)
		dot = slash;
	return mstringview_substr(path, slash, dot - slash);
}

mstringview_t mpathview_extension(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);

	size_t      dot   = path.length;
	const char* begin = mstringview_begin(path);
	for (size_t i = path.length - 1; i != ~0ULL; --i)
	{
		char a = begin[i];
		if (a == '.')
		{
			dot = i;
			break;
		}
		else if (a == '/')
		{
			break;
		}
	}
	return mstringview_substr(path, dot, path.length - dot);
}

mstringview_t mpathview_parent(mstringview_t path)
{
	if (!path.string)
		return mstringviewcstr(NULL);

	size_t slash = mstringview_find_last_of(path, mstringviewcstr("/"), 1);
	return mstringview_substr(path, 0, slash);
}

size_t mpathview_count_elements(mstringview_t path)
{
	if (!path.string)
		return 0;

	size_t count  = 0;
	size_t offset = 0;
	while (offset < path.length)
	{
		// TODO(MarcasRealAccount): mpathview_count_elements handle // and /./ == /
		size_t i = mstringview_find_first_of(path, mstringviewcstr("/"), offset);
		if (i >= path.length)
			break;
		++count;
		offset = i + 1;
	}
	return count;
}

#if BUILD_IS_SYSTEM_WINDOWS
mpath_t mfile_exec()
{
	wchar_t wbuffer[32768];
	DWORD   len       = GetModuleFileNameW(NULL, wbuffer, 32767);
	size_t  lastSlash = 0;
	for (size_t i = 0; i < len; ++i)
	{
		if (wbuffer[i] == L'\\')
		{
			wbuffer[i] = L'/';
			lastSlash  = i;
		}
	}
	int   newLen = WideCharToMultiByte(CP_UTF8, 0, wbuffer, (int) lastSlash, NULL, 0, NULL, NULL);
	char* buffer = (char*) mmalloc(newLen);
	if (!buffer)
		return mpathcstr(NULL);
	newLen       = WideCharToMultiByte(CP_UTF8, 0, wbuffer, (int) lastSlash, buffer, newLen, NULL, NULL);
	mpath_t path = mpathcstr(buffer);
	mfree(buffer);
	return path;
}

mpath_t mfile_cwd()
{
	wchar_t wbuffer[32768];
	DWORD   len = GetCurrentDirectoryW(32767, wbuffer);
	for (size_t i = 0; i < len; ++i)
	{
		if (wbuffer[i] == L'\\')
			wbuffer[i] = L'/';
	}
	int   newLen = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, NULL, 0, NULL, NULL);
	char* buffer = (char*) mmalloc(newLen);
	if (!buffer)
		return mpathcstr(NULL);
	newLen       = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, buffer, newLen, NULL, NULL);
	mpath_t path = mpathcstr(buffer);
	mfree(buffer);
	return path;
}

mpath_t mfile_temp()
{
	wchar_t wbuffer[32768];
	DWORD   len = GetTempPathW(32767, wbuffer);
	for (size_t i = 0; i < len; ++i)
	{
		if (wbuffer[i] == L'\\')
			wbuffer[i] = L'/';
	}
	int   newLen = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, NULL, 0, NULL, NULL);
	char* buffer = (char*) mmalloc(newLen);
	if (!buffer)
		return mpathcstr(NULL);
	newLen       = WideCharToMultiByte(CP_UTF8, 0, wbuffer, len, buffer, newLen, NULL, NULL);
	mpath_t path = mpathcstr(buffer);
	mfree(buffer);
	return path;
}

void mfile_set_cwd(mstringview_t path)
{
	if (!path.string)
		return;
	wchar_t wbuffer[32768];
	int     newLen  = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';
	SetCurrentDirectoryW(wbuffer);
}

static bool mfile_copy_directory(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;
	return false;
}

bool mfile_copy(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;

	mfilestatus_t status;
	if (!mfile_status(from, &status))
		return false;

	switch (status.type)
	{
	case mfiletype_symlink:
	case mfiletype_file: return mfile_copy_file(from, to);
	case mfiletype_directory: return mfile_copy_directory(from, to);
	default: return false;
	}
}

bool mfile_copy_file(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;

	wchar_t wbuffer1[32768];
	wchar_t wbuffer2[32768];
	int     newLen1   = MultiByteToWideChar(CP_UTF8, 0, from.string, (int) from.length, wbuffer1, 32767);
	int     newLen2   = MultiByteToWideChar(CP_UTF8, 0, to.string, (int) to.length, wbuffer2, 32767);
	wbuffer1[newLen1] = L'\0';
	wbuffer2[newLen2] = L'\0';
	return CopyFileExW(wbuffer1, wbuffer2, NULL, NULL, NULL, COPY_FILE_COPY_SYMLINK | COPY_FILE_FAIL_IF_EXISTS);
}

bool mfile_create_directory(mstringview_t path)
{
	if (!path.string)
		return false;

	wchar_t wbuffer[32768];
	int     newLen  = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';
	return CreateDirectoryW(wbuffer, NULL);
}

bool mfile_create_directories(mstringview_t path)
{
	if (!path.string)
		return false;

	wchar_t wbuffer[32768];
	int     newLen     = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
	wbuffer[newLen]    = L'\0';
	size_t offset      = newLen - 1;
	bool   replaceBack = false;
	while (true)
	{
		if (replaceBack)
			wbuffer[offset] = L'/';
		while (offset != ~0ULL && wbuffer[offset] != L'/')
			--offset;
		if (offset == ~0ULL)
			break;
		wbuffer[offset] = L'\0';
		replaceBack     = true;
		if (!CreateDirectoryW(wbuffer, NULL))
		{
			DWORD err = GetLastError();
			if (err == ERROR_ALREADY_EXISTS)
				break;
			return false;
		}
	}
	return true;
}

bool mfile_remove(mstringview_t path)
{
	if (!path.string)
		return false;

	mfilestatus_t status;
	if (!mfile_status(path, &status))
		return false;

	wchar_t wbuffer[32768];
	int     newLen  = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';

	switch (status.type)
	{
	case mfiletype_symlink:
	case mfiletype_file: return DeleteFileW(wbuffer);
	case mfiletype_directory: return RemoveDirectoryW(wbuffer);
	default: return false;
	}
}

size_t mfile_remove_all(mstringview_t path)
{
	if (!path.string)
		return 0;

	mfilestatus_t status;
	if (!mfile_status(path, &status))
		return 0;

	switch (status.type)
	{
	case mfiletype_symlink:
	case mfiletype_file: return mfile_remove(path) ? 1 : 0;
	case mfiletype_directory:
	{
		wchar_t wbuffer[32768];
		int     newLen  = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
		wbuffer[newLen] = L'\0';
		// TODO(MarcasRealAccount): Windows mfile_remove_all implementation for directories
		size_t count = 0;
		return count;
	}
	default: return 0;
	}
}

bool mfile_rename(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;

	wchar_t wbuffer1[32768];
	wchar_t wbuffer2[32768];
	int     newLen1   = MultiByteToWideChar(CP_UTF8, 0, from.string, (int) from.length, wbuffer1, 32767);
	int     newLen2   = MultiByteToWideChar(CP_UTF8, 0, to.string, (int) to.length, wbuffer2, 32767);
	wbuffer1[newLen1] = L'\0';
	wbuffer2[newLen2] = L'\0';
	return MoveFileExW(wbuffer1, wbuffer2, MOVEFILE_COPY_ALLOWED);
}

bool mfile_exists(mstringview_t path)
{
	if (!path.string)
		return false;

	wchar_t wbuffer[32768];
	int     newLen  = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';
	HANDLE handle   = CreateFileW(wbuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	bool   res      = handle != NULL && handle != INVALID_HANDLE_VALUE;
	if (res)
		CloseHandle(handle);
	return res;
}

bool mfile_status(mstringview_t path, mfilestatus_t* status)
{
	if (!path.string || !status)
		return false;
	memset(status, 0, sizeof(mfilestatus_t));
	wchar_t wbuffer[32768];
	int     newLen  = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';
	HANDLE handle   = CreateFileW(wbuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (!handle || handle == INVALID_HANDLE_VALUE)
		return false;
	FILE_BASIC_INFO    basicInfo;
	FILE_STANDARD_INFO standardInfo;
	memset(&basicInfo, 0, sizeof(basicInfo));
	memset(&standardInfo, 0, sizeof(standardInfo));
	GetFileInformationByHandleEx(handle, FileBasicInfo, &basicInfo, sizeof(basicInfo));
	GetFileInformationByHandleEx(handle, FileStandardInfo, &standardInfo, sizeof(standardInfo));
	CloseHandle(handle);
	status->createTime     = basicInfo.CreationTime.QuadPart;
	status->lastWriteTime  = basicInfo.LastWriteTime.QuadPart;
	status->lastAccessTime = basicInfo.LastAccessTime.QuadPart;
	status->type           = basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY
								 ? mfiletype_directory
								 : (basicInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT
										? mfiletype_symlink
										: mfiletype_file);
	status->size           = standardInfo.EndOfFile.QuadPart;
	status->sizeOnDisk     = standardInfo.AllocationSize.QuadPart;
	return true;
}

bool mfile_set_status(mstringview_t path, const mfilestatus_t* status)
{
	if (!path.string || !status)
		return false;
	wchar_t wbuffer[32768];
	int     newLen  = MultiByteToWideChar(CP_UTF8, 0, path.string, (int) path.length, wbuffer, 32767);
	wbuffer[newLen] = L'\0';
	HANDLE handle   = CreateFileW(wbuffer, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (!handle || handle == INVALID_HANDLE_VALUE)
		return false;
	SetFileTime(handle,
				status->createTime != ~0ULL ? (const FILETIME*) &status->createTime : NULL,
				status->lastAccessTime != ~0ULL ? (const FILETIME*) &status->lastAccessTime : NULL,
				status->lastWriteTime != ~0ULL ? (const FILETIME*) &status->lastWriteTime : NULL);
	CloseHandle(handle);
	return true;
}

#elif BUILD_IS_SYSTEM_UNIX

mpath_t mfile_exec()
{
	#if BUILD_IS_SYSTEM_MACOSX
	char buf[4097];
	uint32_t len = 4097;
	_NSGetExecutablePath(buf, &len);
	mpath_t path = mpathv(mstringview(buf, len));
	mpath_t dir = mpathv(mpath_parent(&path));
	mpath_dstr(&path);
	return dir;
	#else
	mpath_t path = mpathcstr("/proc/self/exe");
	if (!mpath_canonical(&path))
	{
		mpath_dstr(&path);
		return mpathcstr(NULL);
	}
	mpath_t dir = mpathv(mpath_parent(&path));
	mpath_dstr(&path);
	return dir;
	#endif
}

mpath_t mfile_cwd()
{
	char buf[4097];
	getcwd(buf, 4097);
	return mpathcstr(buf);
}

mpath_t mfile_temp()
{
	const char* tempDir = getenv("TMPDIR");
	if (tempDir == NULL)
	{
		mpath_t temp = mpathcstr("tmp/");
		mpath_t cwd  = mfile_cwd();
		mpath_prepend(&temp, mstringviewp(&cwd));
		mpath_dstr(&cwd);
		return temp;
	}
	else
	{
		return mpathcstr(tempDir);
	}
}

void mfile_set_cwd(mstringview_t path)
{
	if (!path.string)
		return;
	mpath_t fp = mpathv(path);
	chdir(mpath_begin(&fp));
	mpath_dstr(&fp);
}

static bool mfile_copy_directory(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;

	return true;
}

bool mfile_copy(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;

	mfilestatus_t status;
	if (!mfile_status(from, &status))
		return false;

	switch (status.type)
	{
	case mfiletype_symlink:
	case mfiletype_file: return mfile_copy_file(from, to);
	case mfiletype_directory: return mfile_copy_directory(from, to);
	default: return false;
	}
}

bool mfile_copy_file(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;

	mpath_t fromfp = mpathv(from);
	mpath_t tofp   = mpathv(to);

	int fromfd = open(mpath_begin(&fromfp), O_RDONLY);
	mpath_dstr(&fromfp);
	if (fromfd == -1)
	{
		mpath_dstr(&tofp);
		return false;
	}
	int tofd = open(mpath_begin(&tofp), 0660);
	mpath_dstr(&tofp);
	if (tofd == -1)
	{
		close(fromfd);
		return false;
	}
#if BUILD_IS_SYSTEM_MACOSX
	bool result = fcopyfile(fromfd, tofd, 0, COPYFILE_ALL);
#else
	struct stat fileinfo;
	fstat(fromfd, &fileinfo);
	bool result = sendfile(tofd, fromfd, &bytesCopied, fileinfo.st_size);
#endif
	close(fromfd);
	close(tofd);
	return result;
}

bool mfile_create_directory(mstringview_t path)
{
	if (!path.string)
		return false;

	mpath_t fp = mpathv(path);
	bool result = mkdir(mpath_begin(&fp), 0660);
	mpath_dstr(&fp);
	return result;
}

bool mfile_create_directories(mstringview_t path)
{
	if (!path.string)
		return false;

	mpath_t fp = mpathv(path);
	bool result = true;
	size_t offset = 1;
	bool replaceBack = false;
	char* begin = mpath_begin(&fp);
	while (result && offset < path.length)
	{
		if (replaceBack)
			begin[offset] = '/';
		size_t end = mstringview_find_first_of(path, mstringviewcstr("/"), offset);
		if (end < path.length)
			begin[end] = '\0';
		if (!mkdir(begin, 0660))
			result = false;
		if (end >= path.length)
			break;
		offset = end + 1;
	}
	mpath_dstr(&fp);
	return result;
}

bool mfile_remove(mstringview_t path)
{
	if (!path.string)
		return false;

	mfilestatus_t status;
	if (!mfile_status(path, &status))
		return false;

	switch (status.type)
	{
	case mfiletype_symlink:
	case mfiletype_file:
	case mfiletype_directory:
	{
		mpath_t fp = mpathv(path);
		bool result = remove(mpath_begin(&fp));
		mpath_dstr(&fp);
		return result;
	}
	default: return false;
	}
}

size_t mfile_remove_all(mstringview_t path)
{
	if (!path.string)
		return 0;

	mfilestatus_t status;
	if (!mfile_status(path, &status))
		return 0;
	
	switch (status.type)
	{
	case mfiletype_symlink:
	case mfiletype_file:
	{
		mpath_t fp = mpathv(path);
		bool result = remove(mpath_begin(&fp));
		mpath_dstr(&fp);
		return result ? 1 : 0;
	}
	case mfiletype_directory:
	{
		// TODO(MarcasRealAccount): visit directory files
		size_t removed = 0;
		mpath_t fp = mpathv(path);
		bool result = remove(mpath_begin(&fp));
		mpath_dstr(&fp);
		return removed + (result ? 1 : 0);
	}
	default: return 0;
	}
}

bool mfile_rename(mstringview_t from, mstringview_t to)
{
	if (!from.string || !to.string)
		return false;

	mpath_t fromfp = mpathv(from);
	mpath_t tofp = mpathv(to);
	bool result = rename(mpath_begin(&fromfp), mpath_begin(&tofp));
	mpath_dstr(&fromfp);
	mpath_dstr(&tofp);
	return result;
}

bool mfile_exists(mstringview_t path)
{
	if (!path.string)
		return false;

	mpath_t fp = mpathv(path);
	int handle = open(mpath_begin(&fp), O_RDONLY);
	close(handle);
	mpath_dstr(&fp);
	return handle != -1;
}

static mtimestamp_t mfile_timespec_to_timestamp(struct timespec spec)
{
	return ((mtimestamp_t) spec.tv_sec) * 1000000 + ((mtimestamp_t) spec.tv_nsec) / 1000;
}

bool mfile_status(mstringview_t path, mfilestatus_t* status)
{
	if (!path.string || !status)
		return false;

	mpath_t fp = mpathv(path);
#if BUILD_IS_SYSTEM_MACOSX
	struct stat fs;
	stat(mpath_begin(&fp), &fs);
	mpath_dstr(&fp);
	status->createTime     = 0; // TODO(MarcasRealAccount): Find createTime somehow on MacOSX
	status->lastWriteTime  = mfile_timespec_to_timestamp(fs.st_mtimespec);
	status->lastAccessTime = mfile_timespec_to_timestamp(fs.st_atimespec);
	status->size           = fs.st_size;
	status->sizeOnDisk     = fs.st_blocks * 512;
	switch (fs.st_mode & S_IFMT)
	{
	case S_IFLNK: status->type = mfiletype_symlink; break;
	case S_IFREG: status->type = mfiletype_file; break;
	case S_IFDIR: status->type = mfiletype_directory; break;
	default: status->type = mfiletype_none; break;
	}
#else
	struct statx fs;
	int fd = open(mpath_begin(&fp), O_RDONLY);
	mpath_dstr(&fp);
	statx(fd, NULL, AT_EMPTY_PATH, STATX_TYPE | STATX_ATIME | STATX_MTIME | STATX_BTIME | STATX_SIZE | STATX_BLOCKS, &fs);
	close(fd);
	status->createTime     = mfile_timespec_to_timestamp(fs.stx_btime);
	status->lastWriteTime  = mfile_timespec_to_timestamp(fs.stx_mtime);
	status->lastAccessTime = mfile_timespec_to_timestamp(fs.stx_atime);
	status->size           = fs.stx_size;
	status->sizeOnDisk     = fs.stx_blocks * 512;
	switch (fs.stx_mode & S_IFMT)
	{
	case S_IFLNK: status->type = mfiletype_symlink; break;
	case S_IFREG: status->type = mfiletype_file; break;
	case S_IFDIR: status->type = mfiletype_directory; break;
	default: status->type = mfiletype_none; break;
	}
#endif
	return true;
}

bool mfile_set_status(mstringview_t path, const mfilestatus_t* status)
{
	if (!path.string || !status)
		return false;

	struct timeval times[2] = {
		{
			.tv_sec  = status->lastAccessTime / 1000000,
			.tv_usec = status->lastAccessTime % 1000000
		},
		{
			.tv_sec  = status->lastWriteTime / 1000000,
			.tv_usec = status->lastWriteTime % 1000000
		}
	};
	mpath_t fp = mpathv(path);
	bool result = utimes(mpath_begin(&fp), times);
	mpath_dstr(&fp);
	return result;
}

#else

mpath_t mfile_exec()
{
	return mpathcstr(NULL);
}

mpath_t mfile_cwd()
{
	return mpathcstr(NULL);
}

mpath_t mfile_temp()
{
	return mpathcstr(NULL);
}

void mfile_set_cwd(mstringview_t path)
{
	(void) path;
}

bool mfile_copy(mstringview_t from, mstringview_t to)
{
	(void) from;
	(void) to;
	return false;
}

bool mfile_copy_file(mstringview_t from, mstringview_t to)
{
	(void) from;
	(void) to;
	return false;
}

bool mfile_create_directory(mstringview_t path)
{
	(void) path;
	return false;
}

bool mfile_create_directories(mstringview_t path)
{
	(void) path;
	return false;
}

bool mfile_remove(mstringview_t path)
{
	(void) path;
	return false;
}

size_t mfile_remove_all(mstringview_t path)
{
	(void) path;
	return 0;
}

bool mfile_rename(mstringview_t from, mstringview_t to)
{
	(void) from;
	(void) to;
	return false;
}

bool mfile_exists(mstringview_t path)
{
	(void) path;
	return false;
}

bool mfile_status(mstringview_t path, mfilestatus_t* status)
{
	(void) path;
	(void) status;
	return false;
}

bool mfile_set_status(mstringview_t path, const mfilestatus_t* status)
{
	(void) path;
	(void) status;
	return false;
}

#endif
