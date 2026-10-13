#include "types.h"
#include "stat.h"
#include "user.h"

#define N 5
#define ITEMS 10
#define MUTEX 0
#define EMPTY 1
#define FULL  2

int buf[N];
int in = 0;
int out = 0;

void producer(void *arg)
{
  int i;

  for(i = 1; i <= ITEMS; i++){
    sem_wait(EMPTY);
    sem_wait(MUTEX);
    buf[in] = i;
    printf(1, "Produced item %d at slot %d\n", i, in);
    in = (in + 1) % N;
    sem_signal(MUTEX);
    sem_signal(FULL);
  }
  exit();
}

void consumer(void *arg)
{
  int i, item;

  for(i = 1; i <= ITEMS; i++){
    sem_wait(FULL);
    sem_wait(MUTEX);
    item = buf[out];
    printf(1, "Consumed item %d from slot %d\n", item, out);
    out = (out + 1) % N;
    sem_signal(MUTEX);
    sem_signal(EMPTY);
  }
  exit();
}

int main(int argc, char *argv[])
{
  void *s1;
  void *s2;

  printf(1, "=== Q2: Producer-Consumer (buffer size %d) ===\n", N);
  sem_init(MUTEX, 1);
  sem_init(EMPTY, N);
  sem_init(FULL, 0);

  s1 = malloc(4096);
  s2 = malloc(4096);
  if(s1 == 0 || s2 == 0){
    printf(1, "malloc failed\n");
    exit();
  }

  clone(producer, 0, s1);
  clone(consumer, 0, s2);
  join();
  join();

  free(s1);
  free(s2);
  printf(1, "Producer-Consumer finished: %d items produced and consumed.\n", ITEMS);
  exit();
}
