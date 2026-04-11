#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

#define MAX_THREADS 8
#define STACK_SIZE  8192  

enum tstate { UNUSED, RUNNABLE, RUNNING, ZOMBIE };

struct thread {
  int tid;
  int state;
  void *stack;
  uint saved_esp;
  void (*func)(void*);
  void *arg;
};

static struct thread threads[MAX_THREADS];
static struct thread *current;
static int next_tid = 1;

extern void uswtch(uint *old_esp, uint new_esp);
static void thread_stub(void);

static struct thread* alloc_thread(void) {
  for (int i = 0; i < MAX_THREADS; i++) {
    if (threads[i].state == UNUSED)
      return &threads[i];
  }
  return 0;
}

void thread_init(void) {
  for (int i = 0; i < MAX_THREADS; i++) {
    threads[i].tid = 0;
    threads[i].state = UNUSED;
    threads[i].stack = 0;
    threads[i].saved_esp = 0;
  }
  current = &threads[0];
  current->tid = next_tid++;
  current->state = RUNNING;
  current->stack = 0;
  current->saved_esp = 0;  
  current->func = 0;
}

tid_t thread_create(void (*fn)(void*), void *arg) {
  struct thread *t = alloc_thread();
  if (!t) return -1;

  char *stack = malloc(STACK_SIZE);
  if (!stack) return -1;
  memset(stack, 0, STACK_SIZE);

  uint *sp = (uint*)(stack + STACK_SIZE);
  *--sp = (uint)thread_stub;   // eip
  *--sp = 0;                   // ebp
  *--sp = 0;                   // ebx
  *--sp = 0;                   // esi
  *--sp = 0;                   // edi

  t->tid = next_tid++;
  t->state = RUNNABLE;
  t->stack = stack;
  t->saved_esp = (uint)sp;
  t->func = fn;
  t->arg = arg;

  return t->tid;
}

static void thread_stub(void) {
  current->func(current->arg);
  current->state = ZOMBIE;
  thread_yield();   // give up CPU forever
  while(1);         // should never reach here
}

static struct thread* pick_next(void) {
  int start = (current - threads) + 1;
  for (int i = 0; i < MAX_THREADS; i++) {
    int idx = (start + i) % MAX_THREADS;
    if (threads[idx].state == RUNNABLE)
      return &threads[idx];
  }

  if (current->state == RUNNING)
    return current;
  return &threads[0];
}

void thread_yield(void) {
  struct thread *next = pick_next();
  if (next == current) return;

  struct thread *prev = current;
  current = next;

  if (prev->state == RUNNING)
    prev->state = RUNNABLE;
  current->state = RUNNING;

  uswtch(&prev->saved_esp, next->saved_esp);
}

int thread_join(tid_t tid) {
  struct thread *t = 0;
  for (int i = 0; i < MAX_THREADS; i++) {
    if (threads[i].tid == tid && threads[i].state != UNUSED) {
      t = &threads[i];
      break;
    }
  }
  if (!t) return -1;

  while (t->state != ZOMBIE)
    thread_yield();

  if (t->stack)
    free(t->stack);
  t->state = UNUSED;
  t->tid = 0;
  return tid;
}