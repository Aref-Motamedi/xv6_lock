// Mutual exclusion spin locks.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "ticketlock.h"


void
initlockTicket(struct ticketlock *lk, char *name)
{
  lk->name = name;
  lk->proc = 0; 
  lk->ticket = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
acquireTicket(struct ticketlock *lk)
{ 
    uint currentTicket;
    pushcli(); // disable interrupts to avoid deadlock.

    if(holdingTicket(lk))
        panic("acquire");

    currentTicket = fetch_and_add(&lk->ticket , 1);
    cprintf("current ticket: %d\n",currentTicket);

  // The xchg is atomic.
  while(lk->turn != currentTicket);

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memorysys
  // references happen after the lock is acquired.
  __sync_synchronize();

  // Record info about lock acquisition for debugging.
  lk->cpu = mycpu();
  lk->proc = myproc();
  getcallerpcs(&lk, lk->pcs);

}

// Release the lock.
void
releaseTicket(struct ticketlock *lk)
{
  if(!holdingTicket(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->proc = 0;
  lk->cpu = 0;
  lk->turn++;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code can't use a C assignment, since it might
  // not be atomic. A real OS would use C atomics here.
//   asm volatile("movl $0, %0" : "+m" (lk->locked) : );

  popcli();
}

// Check whether this cpu is holding the lock.
int
holdingTicket(struct ticketlock *lock)
{
//   int r;
//   pushcli();
//   r = lock->locked && lock->cpu == mycpu();
//   popcli();
//   return r;
int check;
if(lock->ticket != lock->turn && (lock->proc == myproc()))
{
    check = 1;
}
else
{
    check = 0;
}

return check;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

// void
// pushcli(void)
// {
//   int eflags;

//   eflags = readeflags();
//   cli();
//   if(mycpu()->ncli == 0)
//     mycpu()->intena = eflags & FL_IF;
//   mycpu()->ncli += 1;
// }

// void
// popcli(void)
// {
//   if(readeflags()&FL_IF)
//     panic("popcli - interruptible");
//   if(--mycpu()->ncli < 0)
//     panic("popcli");
//   if(mycpu()->ncli == 0 && mycpu()->intena)
//     sti();
// }

// // 