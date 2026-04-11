USER-SPACE THREADING LIBRARY



CONTEXT-SWITCHING

We implement cooperative context switching using a custom assembly routine `uswtch.S`.  
- Each thread has its own stack (8 KiB).  
- `uswtch` saves the callee‑saved registers (`edi, esi, ebx, ebp`) on the current stack, stores the current stack pointer in the thread’s `saved_esp`, then loads the new thread’s stack pointer and restores its registers.  
- Scheduling is round‑robin: `thread_yield()` picks the next runnable thread and switches to it.  
- Thread creation sets up an initial stack frame so that the first `uswtch` into the thread returns to `thread_stub`, which calls the user function and then marks the thread as `ZOMBIE`.



MUTEX IMPLEMENTATION

The mutex is a simple spin‑lock that calls `thread_yield()` while the lock is held. Because the scheduler is cooperative, there is no race condition – only one thread runs at a time. `mutex_lock` busy‑waits (yielding after each check) until the lock is free, then sets `locked = 1`. `mutex_unlock` clears the flag.



DEMONSTRATION

`test_pc` creates two producers (100 items each) and one consumer (200 items total). The bounded buffer of size 8 is protected by the mutex. The producers use busy‑wait loops that yield when the buffer is full, guaranteeing that all 200 items are eventually produced. The consumer similarly yields when the buffer is empty. The program runs to completion without deadlock or data corruption. Example output (exact last values may vary):



LIMITATIONS

- **Maximum threads:** 8 (including the main thread). This is a compile‑time constant (`MAX_THREADS`).  
- **Stack size:** Fixed at 8 KiB per thread. Very deep recursion or large stack variables may overflow.  
- **Cooperative only:** Threads must call `thread_yield()` explicitly; a thread that never yields will starve others.  
- **No preemption:** A thread holding a mutex can be preempted only at `thread_yield()` calls, which is safe because other threads will yield until the lock is released.  
- **No timed waits** or condition variables.  
- **Joining a thread that never finishes** will cause an infinite loop (the joiner yields forever).