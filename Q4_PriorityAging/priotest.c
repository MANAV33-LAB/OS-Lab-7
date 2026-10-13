#include "types.h"
#include "stat.h"
#include "user.h"

void work(int n)
{
  volatile int i;
  for(i = 0; i < n; i++)
    ;
}

int main(int argc, char *argv[])
{
  int prios[4] = {3, 8, 14, 20};
  int i, pid, start;

  printf(1, "=== Q4: Priority Scheduling with Aging ===\n");
  start = uptime();

  for(i = 0; i < 4; i++){
    pid = fork();
    if(pid < 0){
      printf(1, "fork failed\n");
      exit();
    }
    if(pid == 0){
      set_priority(getpid(), prios[i]);
      printf(1, "Child pid %d started with priority %d\n", getpid(), prios[i]);
      work(200000000);
      printf(1, "Child pid %d (initial priority %d) FINISHED, current priority %d, at tick %d\n",
             getpid(), prios[i], get_priority(getpid()), uptime() - start);
      exit();
    }
  }

  for(i = 0; i < 4; i++)
    wait();

  printf(1, "Priority test complete.\n");
  exit();
}
