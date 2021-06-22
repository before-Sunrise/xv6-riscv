// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.
//引用计数
int refcount[PHYSTOP/PGSIZE];
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    int pn = (uint64)p /PGSIZE;
    refcount[pn] = 1;
    kfree(p);
  }
    
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int tem;
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  //对引用计数的操作必须加锁，否则多个进程对同一个页面的引用计数进行操作
  //比如refconut[pn]=2;p1:decref(pn), p2:decref(pn),p1:getref(pn), p2:getref(pn)
  //会kfree两次
  acquire(&kmem.lock);
  int pn = (uint64)pa / PGSIZE;
  if(refcount[pn] < 1){
    printf("before panic:refcount[%d] is %d\n", pn, refcount[pn]);
    panic("kfree:kfree a page with refcount < 1");
  }
  refcount[pn] -= 1;
  tem = refcount[pn];
  release(&kmem.lock);
  //如果引用计数此时大于0，说明至少有一个进程在使用该页面，所以不需要释放
  if(tem > 0){
    return;
  }
  // Fill with junk to catch dangling refs.
  memset(pa, 0, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    int pn = (uint64)r / PGSIZE;
    if(refcount[pn] != 0){
      panic("kalloc:reference not equal to zero");
    }
    //引用计数设为1
    refcount[pn] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void increref(uint64 pa){
  int pn = pa / PGSIZE;
  acquire(&kmem.lock);
  if((char*)pa < end || pa >= PHYSTOP || refcount[pn] < 1){
    panic("increref");
  }
  refcount[pn] += 1;
 // printf("after increref:refcount[%d] is %d\n", pn, refcount[pn]);
  release(&kmem.lock);
}