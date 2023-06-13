#ifndef __FS_H__
#define __FS_H__

#include "common.h"

//So WHAT THE HELL IS IT?  another var named same as them are defined in other files..
enum {SEEK_SET1, SEEK_CUR1, SEEK_END1};

int fs_open(const char*pathname,int flags, int mode);

ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);

off_t fs_lseek(int fd, off_t offset, int whence);

int fs_close(int fd);

size_t fs_filesz(int fd);


#endif
