#include "common.h"
#include "fs.h"
#include "memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

extern void ramdisk_read(void *buf, off_t* offset, size_t len);

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  /* read `len' bytes starting from `offset' of ramdisk into `buf' */
  //ramdisk_read(DEFAULT_ENTRY,(off_t*)0,RAMDISK_SIZE); 
  int fd=fs_open(filename,0,0);
  Log("filename: %s, fd: %d",filename,fd);

  int size=fs_filesz(fd);
  //get the num of pages
  int pgnum=size/PGSIZE;
  if(size%PGSIZE!=0)
  {
    pgnum++;
  }

  void*pa=NULL;
  void*va=DEFAULT_ENTRY;

  for(int i=0;i<pgnum;i++)
  {
    pa=new_page();
    _map(as,va,pa);
    fs_read(fd,pa,PGSIZE);
    va+=PGSIZE;
  }


  //fs_read(fd,DEFAULT_ENTRY,size);

  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
