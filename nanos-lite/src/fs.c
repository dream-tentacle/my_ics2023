#include "fs.h"
#include "ramdisk.h"

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO };
// FD_EVENTS: keyboard
// FD_FB: frame buffer, the VGA device
size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}
extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);
/* This is the information about all files in disk. */
Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
    [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
    [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size =
      gpu_config.width * gpu_config.height * sizeof(uint32_t);
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
  if (file_table[fd].write == NULL) {
    assert(file_table[fd].open_offset + count <= file_table[fd].size);
    ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
                  count);
    file_table[fd].open_offset += count;
    return count;
  } else
    return file_table[fd].write(buf, file_table[fd].open_offset, count);
  return -1;
}
int fs_read(int fd, void *buf, size_t count) {
  if (file_table[fd].read == NULL) {
    if (file_table[fd].open_offset > file_table[fd].size)
      panic("fs_read: file_table[%d].open_offset = %d, "
            "file_table[fd].size = %d\n",
            fd, file_table[fd].open_offset, file_table[fd].size);
    if (count + file_table[fd].open_offset > file_table[fd].size) {
      count = file_table[fd].size - file_table[fd].open_offset;
    }
    ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
                 count);
    file_table[fd].open_offset += count;
    return count;
  } else
    return file_table[fd].read(buf, file_table[fd].open_offset, count);
  return -1;
}
int fs_lseek(int fd, size_t offset, int whence) {
  if (fd <= 2) {
    panic("fd = %d in fs_lseek!\n", fd);
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
  return file_table[fd].open_offset;
}
