#include "fs.h"
/*
in pa, the size and position of each file is unchangeable
the number of files is also a constant value
*/


typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;  //file's offset in ramdisk
  off_t open_offset;  //file has been opened, the position it has been read
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))  //calculate the number of files

extern void getScreen(int* width, int* height);

void init_fs() {
  // TODO: initialize the size of /dev/fb
  //screen?  ioe
  int width=0,height=0;
  getScreen(&width,&height);
  file_table[FD_FB].size=width*height*sizeof(uint32_t);
  Log("set /dev/fb's size: %d",fs_filesz(FD_FB));
}

void set_openoffset(int fd,off_t off)
{
  assert(fd>=0&&fd<=NR_FILES);
  assert(off>=0);
  if(off>=file_table[fd].size)
  {
    off=file_table[fd].size;
  }
  file_table[fd].open_offset=off;
}

off_t get_diskoffset(int fd)
{
  assert(fd >=0 && fd < NR_FILES);
  return file_table[fd].disk_offset;
}

off_t get_openoffset(int fd)
{
  assert(fd >=0 && fd < NR_FILES);
  return file_table[fd].open_offset;
}

size_t fs_filesz(int fd) 
{
  assert(fd >=0 && fd < NR_FILES);
  return file_table[fd].size;
}

extern void ramdisk_read(void* buf, off_t offset, size_t len);
extern void ramdisk_write(void* buf, off_t offset, size_t len);
//use these 2 functions to read/write

int fs_open(const char*pathname,int flags, int mode)
{
  for(int i=0;i<NR_FILES;i++)
  {
    if(strcmp(pathname,file_table[i].name)==0)
    {
      Log("open file %s successfully",pathname);
      set_openoffset(i,0);
      return i;
    }
  }
  panic("no such file or dictionary!\n");
  return -1;
}

size_t events_read(void *buf, size_t len);
void dispinfo_read(void* buf, off_t offset, size_t len);

ssize_t fs_read(int fd, void *buf, size_t len)
{
  assert(fd>=0&&fd<=NR_FILES);
  if(fd<3||fd==FD_FB)
  {
    //FD_FB means..
    Log("ur fd number is invalid, fd<3 or fd==FD_FB");
    return 0;
  }

  if (fd == FD_EVENTS) 
  {
    return events_read(buf, len);
  }

  int n=fs_filesz(fd)-get_openoffset(fd);
  if(n>len)
  {
    n=len;
  }

  if(fd==FD_DISPINFO)
  {
    dispinfo_read(buf, get_openoffset(fd), n);
  }
  else
  {
    ramdisk_read(buf,get_diskoffset(fd)+get_openoffset(fd),n);
  }

  set_openoffset(fd,get_openoffset(fd)+n);
  return n;
}

void fb_write(const void *buf, off_t offset, size_t len);

ssize_t fs_write(int fd, const void *buf, size_t len)
{
  assert(fd>=0&&fd<=NR_FILES);

  if(fd<3||fd==FD_DISPINFO)
  {
    //FD_FB means..
    Log("ur fd number is invalid, fd<3 or fd==FD_FB");
    return 0;
  }

  int n=fs_filesz(fd)-get_openoffset(fd);
  if(n>len)
  {
    n=len;
  }

  if(fd==FD_FB)
  {
    fb_write(buf,get_openoffset(fd),n);
  }
  else
  {
    ramdisk_write(buf,get_diskoffset(fd)+get_openoffset(fd),n);
  }

  set_openoffset(fd,get_openoffset(fd)+n);

  return n;

}
/*
lseek()  repositions the file offset of the open file description asso‐
       ciated with the file descriptor fd to the argument offset according  to
       the directive whence as follows:

       SEEK_SET
              The file offset is set to offset bytes.

       SEEK_CUR
              The  file  offset  is  set  to  its current location plus offset
              bytes.

       SEEK_END
              The file offset is set to the  size  of  the  file  plus  offset
              bytes.


*/


off_t fs_lseek(int fd, off_t offset, int whence)
{

  switch(whence)
  {
    case SEEK_SET1:
      set_openoffset(fd,offset);
      return get_openoffset(fd);
    case SEEK_CUR1:
      set_openoffset(fd,get_openoffset(fd)+offset);
      return get_openoffset(fd);
    case SEEK_END1:
      set_openoffset(fd,fs_filesz(fd)+offset);
      return get_openoffset(fd);
    default:
      panic("incorrect whence ID = %d!",whence);
      return -1;
  }
}

int fs_close(int fd) {
    assert(fd >= 0 && fd < NR_FILES);
    return 0;
}
