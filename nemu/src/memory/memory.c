#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

//get the pte
#define PTE_ADDR(pte)  ((uint32_t)(pte)&~0xfff)

//va higher 10 bits-->pd  middle 10 bits-->pt  last 10 bits-->offset
#define PDX(va) (((uint32_t)(va)>>22)&0x3ff)
#define PTX(va) (((uint32_t)(va)>>12)&0x3ff)
#define OFF(va) ((uint32_t)(va)&0xfff)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

//get the physical address

/*
format of PT index
 31                                  12 11                      0
      +--------------------------------------+-------+---+-+-+---+-+-+-+
      |                                      |       |   | | |   |U|R| |
      |      PAGE FRAME ADDRESS 31..12       | AVAIL |0 0|D|A|0 0|/|/|P|
      |                                      |       |   | | |   |S|W| |
      +--------------------------------------+-------+---+-+-+---+-+-+-+

typedef union CR0 {
  struct {
    uint32_t protect_enable      : 1;
    uint32_t dont_care           : 30;
    uint32_t paging              : 1;  //if paging is 1, enable the paging
  };
  uint32_t val;
} CR0;
the Control Register 3 (physical address of page directory)
typedef union CR3 {
  struct {
    uint32_t pad0                : 3;
    uint32_t page_write_through  : 1;
    uint32_t page_cache_disable  : 1;
    uint32_t pad1                : 7;
    uint32_t page_directory_base : 20;
  };
  uint32_t val;
} CR3;
      

*/
//higher 10 bits find pd, middle 10 bits find pt


paddr_t page_translate(vaddr_t addr, bool write)
{
  CR0 cr0=(CR0)cpu.CR0;
  if(cr0.paging&&cr0.protect_enable)//paging is abled and in protect verision
  {
    CR3 cr3=(CR3)cpu.CR3;
    //get the loacation of PDE-->now is base addr
    PDE* pgdir=(PDE*)PTE_ADDR(cr3.val); 

    //pa = (pg_table[va>>10]&~0x3ff) | (va & 0x3ff)
    PDE pde=(PDE)paddr_read((uint32_t)(pgdir+PDX(addr)),4);
    //make sure it is availiable
    Assert(pde.present,"addr=0x%x",addr);

    PTE* ptes=(PTE*)PTE_ADDR(pde.val);//get the pd
    PTE pte=(PTE)paddr_read((uint32_t)(ptes+PTX(addr)),4);
    Assert(pte.present,"addr=0x%x",addr);

    //access bits and dirty bits
    pde.accessed=1;
    pte.accessed=1;

    if(write)
    {
      pte.dirty=1;
    }

    paddr_t paddr=PTE_ADDR(pte.val)|OFF(addr);//value of pte(high 20 bits) and offset(12 bits)
    return paddr;
  }

  return addr;  
}


uint32_t paddr_read(paddr_t addr, int len) {
  int mid=is_mmio(addr);
  if(mid==-1)
  {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
  else
  {
    return mmio_read(addr,len,mid);
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mid=is_mmio(addr);
  if(mid==-1)
  {
    memcpy(guest_to_host(addr), &data, len);
  } 
  else
  {
    mmio_write(addr,len,data,mid);
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if (PTE_ADDR(addr)!=PTE_ADDR(addr+len-1))
  {
    //do nothing by now.
    //data cross the page boundary-->go to the different page
    int firstLen=0x1000-OFF(addr); //the length that will be written on the first page
    int restLen=len-firstLen;

    paddr_t addr1=page_translate(addr,false);
    paddr_t addr2=page_translate(addr+firstLen,false);
    //ATTENTION!!!  SMALL ENDING!!!!  THE HIGHER BITS ARE SAVED IN THE HIGHER ADDR
    uint32_t low=paddr_read(addr1,firstLen);
    uint32_t high=paddr_read(addr2,restLen);

    uint32_t result=high<<(firstLen*8)|low;
    return result;
  }
  else
  {
    //printf("read转换addr=0x%x \n",addr);
    paddr_t paddr=page_translate(addr,false);
    //printf("read转换addr=0x%x sucess\n",addr);
    return paddr_read(paddr,len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(PTE_ADDR(addr)!=PTE_ADDR(addr+len-1))
  {
    //do nothing by now.
    //data cross the page boundary-->go to the different page
    int firstLen=0x1000-OFF(addr); //the length that will be written on the first page
    int restLen=len-firstLen;

    paddr_t addr1=page_translate(addr,true);
    paddr_t addr2=page_translate(addr+firstLen,true);
    //get the lower bits of data
    uint32_t low=data&(~0u>>((4-firstLen)*8));
    //higher bits of data
    uint32_t high=data>>((4-restLen)*8);

    paddr_write(addr1,firstLen,low);
    paddr_write(addr2,restLen,high);

    return;
  }
  else
  {
    paddr_t paddr=page_translate(addr,true);
    paddr_write(paddr, len, data);
  }
}


