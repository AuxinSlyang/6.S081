// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  uint64 freepg;
} kmem[NCPU];

void
kinit()
{
  int id = cpuid();
  initlock(&kmem[id].lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  kmem[id].freepg = freesize(id);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int id = cpuid();

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[id].lock);
  r->next = kmem[id].freelist;
  kmem[id].freelist = r;
  kmem[id].freepg ++;
  release(&kmem[id].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int id = cpuid();

  acquire(&kmem[id].lock);
  r = kmem[id].freelist;

  // donot have any page but steal page
  // or just have free page.
  // alloc free page.
  if((!r && steal_page()) || r) {
    r = kmem[id].freelist;
    kmem[id].freelist = r->next;
    kmem[id].freepg --;
  }

  release(&kmem[id].lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// before calling this, must hold own kmem lock.
uint64
steal_page()
{
  int myid = cpuid();
  int steal = 0;
  struct run *r;

  for (int i = 0; i < NCPU; i ++) {
    if (i == myid)
      continue;
    acquire(&kmem[i].lock);
    steal = kmem[i].freepg / 2;
    if (kmem[i].freepg > 0 && steal > 0) {
      // just steal half.
      kmem[i].freepg = kmem[i].freepg - steal;
      kmem[myid].freepg = steal;
      r = kmem[i].freelist;

      for (int j = 0; j < steal - 1; j++) {
        r = r->next;
      }

      kmem[myid].freelist = kmem[i].freelist;
      kmem[i].freelist = r->next;
      r->next = 0;
      release(&kmem[i].lock);
      break;
    } else {
      release(&kmem[i].lock);
    }
  }

  return steal;
}

uint64
freesize(int id)
{
  struct run *r;
  uint64 res = 0;

  acquire(&kmem[id].lock);
  r = kmem[id].freelist;
  while (r) {
    res += 1;
    r = r->next;
  }
  release(&kmem[id].lock);

  return res;
}