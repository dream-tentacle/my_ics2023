#include "fs.h"
#include "ramdisk.h"

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);
typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *path, int flags, int mode) {
  int len = sizeof(file_table) / sizeof(Finfo);
  for (int i = 3; i < len; i++) {
    if (strcmp(file_table[i].name, path) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("no file name: \"%s\" found!\n", path);
}
int fs_write(int fd, void *buf, size_t count) {
  if (fd == 1 || fd == 2) {
    char *str = (char *)buf;
    for (int i = 0; i < count; i++) {
      putch(str[i]);
    }
    return count;
  } else if (fd != 0) {
    assert(file_table[fd].open_offset + count <= file_table[fd].size);
    ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
                  count);
    file_table[fd].open_offset += count;
    return count;
  }
  return -1;
}
int fs_read(int fd, void *buf, size_t count) {
  if (fd != 0) {
    assert(file_table[fd].open_offset + count <= file_table[fd].size);
    ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
                 count);
    file_table[fd].open_offset += count;
    return count;
  }
  return -1;
}
int fs_lseek(int fd, size_t offset, int whence) {
  if (fd <= 2) {
    printf("fd = %d in fs_lseek!\n", fd);
    return 0;
  }
  if (whence == 0) {
    file_table[fd].open_offset = offset;
  } else if (whence == 1) {
    file_table[fd].open_offset += offset;
  } else if (whence == 2) {
    file_table[fd].open_offset = file_table[fd].size + offset;
  } else {
    panic("wrong whence = %d in fs_lseek!\n", whence);
  }
  printf("fs_lseek: fd = %d, offset = %d, whence = %d\n", fd,
         file_table[fd].open_offset, whence);
  return file_table[fd].open_offset;
}
