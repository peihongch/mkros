#include "buf.h"
#include "defs.h"
#include "fs.h"
#include "param.h"
#include "riscv.h"
#include "sleeplock.h"
#include "spinlock.h"
#include "types.h"

// Simple logging that allows concurrent FS system calls.
//
// A log transaction contains the updates of multiple FS system
// calls. The logging system only commits when there are
// no FS system calls active. Thus there is never
// any reasoning required about whether a commit might
// write an uncommitted system call's updates to disk.
//
// A system call should call begin_op()/end_op() to mark
// its start and end. Usually begin_op() just increments
// the count of in-progress FS system calls and returns.
// But if it thinks the log is close to running out, it
// sleeps until the last outstanding end_op() commits.
//
// The log is a physical re-do log containing disk blocks.
// The on-disk log format:
//   header block, containing block #s for block A, B, C, ...
//   block A
//   block B
//   block C
//   ...
// Log appends are synchronous.

// Contents of the header block, used for both the on-disk header block
// and to keep track in memory of logged block# before commit.
struct logheader {
    int n;
    int block[LOGSIZE];
};

struct log {
    struct spinlock lock;
    int start;
    int size;
    int outstanding;  // how many FS sys calls are executing.
    int committing;   // in commit(), please wait.
    int dev;
    struct logheader lh;
};

struct log klog;

static void recover_from_log(void);
static void commit();

void initlog(int dev, struct superblock* sb) {
    if (sizeof(struct logheader) >= BSIZE)
        panic("initlog: too big logheader");

    initlock(&klog.lock, "log");
    klog.start = sb->logstart;
    klog.size = sb->nlog;
    klog.dev = dev;
    recover_from_log();
}

// Copy committed blocks from log to their home location
static void install_trans(int recovering) {
    int tail;

    for (tail = 0; tail < klog.lh.n; tail++) {
        struct buf* lbuf =
            bread(klog.dev, klog.start + tail + 1);  // read log block
        struct buf* dbuf = bread(klog.dev, klog.lh.block[tail]);  // read dst
        memmove(dbuf->data, lbuf->data, BSIZE);  // copy block to dst
        bwrite(dbuf);                            // write dst to disk
        if (recovering == 0)
            bunpin(dbuf);
        brelse(lbuf);
        brelse(dbuf);
    }
}

// Read the log header from disk into the in-memory log header
static void read_head(void) {
    struct buf* buf = bread(klog.dev, klog.start);
    struct logheader* lh = (struct logheader*)(buf->data);
    int i;
    klog.lh.n = lh->n;
    for (i = 0; i < klog.lh.n; i++) {
        klog.lh.block[i] = lh->block[i];
    }
    brelse(buf);
}

// Write in-memory log header to disk.
// This is the true point at which the
// current transaction commits.
static void write_head(void) {
    struct buf* buf = bread(klog.dev, klog.start);
    struct logheader* hb = (struct logheader*)(buf->data);
    int i;
    hb->n = klog.lh.n;
    for (i = 0; i < klog.lh.n; i++) {
        hb->block[i] = klog.lh.block[i];
    }
    bwrite(buf);
    brelse(buf);
}

static void recover_from_log(void) {
    read_head();
    install_trans(1);  // if committed, copy from log to disk
    klog.lh.n = 0;
    write_head();  // clear the log
}

// called at the start of each FS system call.
void begin_op(void) {
    acquire(&klog.lock);
    while (1) {
        if (klog.committing) {
            sleep(&klog, &klog.lock);
        } else if (klog.lh.n + (klog.outstanding + 1) * MAXOPBLOCKS > LOGSIZE) {
            // this op might exhaust log space; wait for commit.
            sleep(&klog, &klog.lock);
        } else {
            klog.outstanding += 1;
            release(&klog.lock);
            break;
        }
    }
}

// called at the end of each FS system call.
// commits if this was the last outstanding operation.
void end_op(void) {
    int do_commit = 0;

    acquire(&klog.lock);
    klog.outstanding -= 1;
    if (klog.committing)
        panic("klog.committing");
    if (klog.outstanding == 0) {
        do_commit = 1;
        klog.committing = 1;
    } else {
        // begin_op() may be waiting for log space,
        // and decrementing klog.outstanding has decreased
        // the amount of reserved space.
        wakeup(&klog);
    }
    release(&klog.lock);

    if (do_commit) {
        // call commit w/o holding locks, since not allowed
        // to sleep with locks.
        commit();
        acquire(&klog.lock);
        klog.committing = 0;
        wakeup(&klog);
        release(&klog.lock);
    }
}

// Copy modified blocks from cache to klog.
static void write_log(void) {
    int tail;

    for (tail = 0; tail < klog.lh.n; tail++) {
        struct buf* to = bread(klog.dev, klog.start + tail + 1);  // log block
        struct buf* from = bread(klog.dev, klog.lh.block[tail]);  // cache block
        memmove(to->data, from->data, BSIZE);
        bwrite(to);  // write the log
        brelse(from);
        brelse(to);
    }
}

static void commit() {
    if (klog.lh.n > 0) {
        write_log();       // Write modified blocks from cache to log
        write_head();      // Write header to disk -- the real commit
        install_trans(0);  // Now install writes to home locations
        klog.lh.n = 0;
        write_head();  // Erase the transaction from the log
    }
}

// Caller has modified b->data and is done with the buffer.
// Record the block number and pin in the cache by increasing refcnt.
// commit()/write_log() will do the disk write.
//
// log_write() replaces bwrite(); a typical use is:
//   bp = bread(...)
//   modify bp->data[]
//   log_write(bp)
//   brelse(bp)
void log_write(struct buf* b) {
    int i;

    acquire(&klog.lock);
    if (klog.lh.n >= LOGSIZE || klog.lh.n >= klog.size - 1)
        panic("too big a transaction");
    if (klog.outstanding < 1)
        panic("log_write outside of trans");

    for (i = 0; i < klog.lh.n; i++) {
        if (klog.lh.block[i] == b->blockno)  // log absorption
            break;
    }
    klog.lh.block[i] = b->blockno;
    if (i == klog.lh.n) {  // Add new block to log?
        bpin(b);
        klog.lh.n++;
    }
    release(&klog.lock);
}
