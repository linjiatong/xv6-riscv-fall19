// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13

struct {
  struct spinlock lock[NBUCKETS];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf hashbucket[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;

  // 将hashbucket节点指向自己
  for(int i = 0; i < NBUCKETS ; i++) {
    initlock(&bcache.lock[i],"bcache.bucket");
    // b = &bcache.hashbucket[i];
    // b->prev = b;
    // b->next = b;
    bcache.hashbucket[i].prev = &bcache.hashbucket[i];
    bcache.hashbucket[i].next = &bcache.hashbucket[i];
  }


  //initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  //bcache.head.prev = &bcache.head;
  //bcache.head.next = &bcache.head;
  // next: n->...->3->2->1->buf[0]->head[0]->n
  // pre:  head[0]->buf[0]->1->2->3->..->n->head[0]
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.hashbucket[0].next;
    b->prev = &bcache.hashbucket[0];
    initsleeplock(&b->lock,"buffer");
    bcache.hashbucket[0].next->prev = b;
    bcache.hashbucket[0].next = b;
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // initsleeplock(&b->lock, "buffer");
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  // 得到哈希值，上锁
  int number = blockno % NBUCKETS;
  acquire(&bcache.lock[number]);

  // Is the block already cached?
  for(b = bcache.hashbucket[number].next; b != &bcache.hashbucket[number]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[number]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  // 找到空闲buffer，当cnumber = number时，说明所有buffer都busy
  int cnumber = (number + 1) % NBUCKETS;
  while(cnumber != number) {
    acquire(&bcache.lock[cnumber]);

    for(b = bcache.hashbucket[cnumber].prev ; b != &bcache.hashbucket[cnumber]; b = b->prev) {
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        
        // 将空闲buffer从原bucket链表断开
        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.lock[cnumber]);

        // 将空闲链表插到对应哈希值的bucket链表中
        b->next=bcache.hashbucket[number].next;
        b->prev=&bcache.hashbucket[number];
        bcache.hashbucket[number].next->prev=b;
        bcache.hashbucket[number].next=b;
        release(&bcache.lock[number]);
        acquiresleep(&b->lock);
        return b;

      }
    }
    release(&bcache.lock[cnumber]);
    cnumber = (cnumber + 1) % NBUCKETS;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int number = (b->blockno) % NBUCKETS;
  acquire(&bcache.lock[number]);
  b->refcnt--;

  
  if (b->refcnt == 0) {
    // no one is waiting for it.

    // 类似bget not cached
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.hashbucket[number].next;
    b->prev = &bcache.hashbucket[number];
    bcache.hashbucket[number].next->prev = b;
    bcache.hashbucket[number].next = b;
  }
  
  release(&bcache.lock[number]);
}

void
bpin(struct buf *b) {
  int number = (b->blockno) % NBUCKETS;
  acquire(&bcache.lock[number]);
  b->refcnt++;
  release(&bcache.lock[number]);
}

void
bunpin(struct buf *b) {
  int number = (b->blockno) % NBUCKETS;
  acquire(&bcache.lock[number]);
  b->refcnt--;
  release(&bcache.lock[number]);
}