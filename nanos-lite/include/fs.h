#ifndef __FS_H__
#define __FS_H__

#include <common.h>

#ifndef SEEK_SET
enum { SEEK_SET, SEEK_CUR, SEEK_END };
#endif

int fs_open(const char *path, int flags, int mode);
int fs_write(int fd, void *buf, size_t count);
int fs_read(int fd, void *buf, size_t count);
int fs_lseek(int fd, size_t offset, int whence);

#endif