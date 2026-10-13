# OS Lab 7 Report
**Manav Raj (2401MC33)**

## Q1: Kernel Threads (clone / join)

**Design.** `struct proc` got an `isthread` flag. `clone(fcn, arg, stack)` allocates a new proc, shares the parent's `pgdir` and `sz`, gives the thread its own kernel stack, and copies the trapframe. It places `arg` and a fake return address on the user-provided stack and sets `eip = fcn` and `esp` to that stack, so the thread starts running `fcn(arg)`. `join()` waits for a child with `isthread` set, frees only its kernel stack, and sets `pgdir = 0` without freeing it, because the address space is shared. `wait()` was changed to skip thread children, so it never frees a shared address space.

**Result.** Three threads each increment a shared counter 100,000 times with no synchronization. Final values from two runs were **264162** and **238781**, both below 300000. Threads read the counter, get preempted, and write back stale values (a lost-update race). The counter being shared proves the threads run in one address space.

## Q2: Semaphores

**Design.** The kernel keeps a fixed array of 10 semaphores, `struct semaphore { int value; int inited; struct spinlock lk; }`, with a global spinlock protecting initialization. `sem_wait` sleeps on the semaphore while `value <= 0`, then decrements. `sem_signal` increments and calls `wakeup`. Blocking uses xv6's `sleep()`/`wakeup()`. Because the process sleeps until `value > 0`, the value never goes negative. This differs from the textbook form (decrement first, block if negative) but behaves the same for callers. System calls: `sem_init`, `sem_wait`, `sem_signal`.

**Before vs after.**

| Program | Synchronization | Final counter |
|---|---|---|
| threadtest (Q1) | none | 264162 / 238781 (wrong) |
| synctest (Q2) | semaphore around the increment | **300000** (exact) |

**Producer-Consumer.** Buffer size 5 with three semaphores: `mutex = 1`, `empty = 5`, `full = 0`. One producer thread and one consumer thread handled items 1 to 10. The log shows all 10 items produced and consumed in order, with the buffer never overflowing or underflowing.

## Q3: Deadlock Detection and Banker's Algorithm

**Design.** `deadlock.c` is a user program using static arrays `Allocation`, `Request`, `Max` and `Available`. `detect_deadlock()` repeatedly finds an unfinished process whose Request fits in Work, adds its allocation back, and repeats. Anything left unfinished is deadlocked. `is_safe_state()` runs the Banker's safety check using Need = Max - Allocation, and `request_resources()` tentatively grants a request, checks safety, and rolls back if unsafe.

| Test | Scenario | Result |
|---|---|---|
| 1 | Deadlocked matrix, Available = 0 0 0 | DEADLOCK: P1 P2 P3 P4 |
| 2 | Banker's, initial state | SAFE, sequence P1 P3 P4 P0 P2 |
| 3 | P1 requests [1,0,2], then P0 requests [0,2,0] | P1 granted (safe); P0 denied (unsafe) |
| 4 | Two real xv6 processes blocked on two pipes | Circular wait detected: A and B deadlocked |

In test 4, process A holds the write end of Pipe1 and reads Pipe2, while B holds Pipe2's write end and reads Pipe1. Both block forever, the detector reports the cycle, and the processes are then killed and cleaned up.

## Q4: Priority Scheduling with Aging

**Design.** `struct proc` got `priority` (1 = highest, 20 = lowest, default 10) and `wait_ticks`. The syscalls are `set_priority` and `get_priority`. `scheduler()` scans the table and picks the RUNNABLE process with the lowest priority number. The scan starts from a rotating index (`rrnext`), so equal priorities are served round-robin. Preemption comes from xv6's timer interrupt, which already calls `yield()` every tick. `aging_tick()` runs on every timer tick. It increments `wait_ticks` of each RUNNABLE process, and when a process has waited more than 30 ticks its priority number drops by 1 and the counter resets.

**Result (times in ticks from program start).**

| PID | Initial priority | Final priority | Finished at tick |
|---|---|---|---|
| 4 | 3 | 3 | 53 |
| 5 | 8 | 8 | 112 |
| 6 | 14 | 13 | 170 |
| 7 | 20 | 17 | 226 |

Completion order matches priority order. The lowest-priority process (PID 7) was raised from 20 to 17 while waiting and still finished, so it did not starve. The `[aging]` lines in the output show the priority changes.

## Problems faced and fixes
- **Build errors on a modern GCC.** `-Werror` turned new warnings into errors, so I added `-Wno-error=...` flags for infinite-recursion, array-bounds, stringop-overflow and maybe-uninitialized in the Makefile.
- **`exec: fail` when running priotest.** A Makefile edit did not match (wrong escaping), so the program was never added to the disk image. Fixed the match and rebuilt.
- **Garbled console output.** xv6's `printf` makes separate writes, so when the timer preempts a process mid-print, lines from different processes interleave. This is expected and does not affect correctness.
- **Race condition setup.** An empty delay loop between the read and write of the counter makes the race show up reliably in Q1.

## Limitations
Threads do not support safe `malloc`/`sbrk` from inside a thread, and the semaphore table has a fixed size of 10.
