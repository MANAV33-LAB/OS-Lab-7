#include "types.h"
#include "stat.h"
#include "user.h"

volatile int counter = 0;

void worker(void *arg)
{
  int i;
  volatile int j;
  int tmp;

  for(i = 0; i < 100000; i++){
    sem_wait(0);
    tmp = counter;
    for(j = 0; j < 20; j++)
      ;
    counter = tmp + 1;
    sem_signal(0);
  }
  exit();
}

int main(int argc, char *argv[])
{
  int i;
  void *stacks[3];

  printf(1, "=== Q2: Synchronized Counter (with semaphore) ===\n");
  sem_init(0, 1);

  for(i = 0; i < 3; i++){
    stacks[i] = malloc(4096);
    if(stacks[i] == 0){
      printf(1, "malloc failed\n");
      exit();
    }
    if(clone(worker, (void*)i, stacks[i]) < 0){
      printf(1, "clone failed\n");
      exit();
    }
    printf(1, "Created thread %d\n", i + 1);
  }

  for(i = 0; i < 3; i++){
    join();
    printf(1, "Joined thread %d\n", i + 1);
  }

  for(i = 0; i < 3; i++)
    free(stacks[i]);

  printf(1, "Final counter = %d (expected 300000)\n", counter);
  exit();
}
