#include "types.h"
#include "stat.h"
#include "user.h"

#define P 5
#define R 3

int n, m;
int Allocation[P][R];
int Request[P][R];
int Max[P][R];
int Available[R];

int dAlloc[P][R] = {{0,1,0},{2,0,0},{3,0,3},{2,1,1},{0,0,2}};
int dReq[P][R]   = {{0,0,0},{2,0,2},{0,0,1},{1,0,0},{0,0,2}};
int dAvail[R]    = {0,0,0};

int bAlloc[P][R] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
int bMax[P][R]   = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
int bAvail[R]    = {3,3,2};

void load_detect(void)
{
  int i, j;
  n = P; m = R;
  for(i = 0; i < n; i++)
    for(j = 0; j < m; j++){
      Allocation[i][j] = dAlloc[i][j];
      Request[i][j] = dReq[i][j];
      Max[i][j] = 0;
    }
  for(j = 0; j < m; j++)
    Available[j] = dAvail[j];
}

void load_banker(void)
{
  int i, j;
  n = P; m = R;
  for(i = 0; i < n; i++)
    for(j = 0; j < m; j++){
      Allocation[i][j] = bAlloc[i][j];
      Max[i][j] = bMax[i][j];
      Request[i][j] = 0;
    }
  for(j = 0; j < m; j++)
    Available[j] = bAvail[j];
}

void print_state(int useReq)
{
  int i, j;
  printf(1, "Available: ");
  for(j = 0; j < m; j++)
    printf(1, "%d ", Available[j]);
  printf(1, "\n");
  printf(1, "Proc  Allocation  %s\n", useReq ? "Request" : "Need");
  for(i = 0; i < n; i++){
    printf(1, "P%d    ", i);
    for(j = 0; j < m; j++)
      printf(1, "%d ", Allocation[i][j]);
    printf(1, "      ");
    for(j = 0; j < m; j++)
      printf(1, "%d ", useReq ? Request[i][j] : Max[i][j] - Allocation[i][j]);
    printf(1, "\n");
  }
}

// Deadlock detection. Fills dead[] with deadlocked process numbers, returns the count.
int detect_deadlock(int dead[])
{
  int work[R], finish[P];
  int i, j, k, ok, progress, cnt;

  for(j = 0; j < m; j++)
    work[j] = Available[j];
  for(i = 0; i < n; i++){
    finish[i] = 1;
    for(j = 0; j < m; j++)
      if(Allocation[i][j] != 0)
        finish[i] = 0;
  }
  do {
    progress = 0;
    for(i = 0; i < n; i++){
      if(finish[i])
        continue;
      ok = 1;
      for(j = 0; j < m; j++)
        if(Request[i][j] > work[j])
          ok = 0;
      if(ok){
        for(k = 0; k < m; k++)
          work[k] += Allocation[i][k];
        finish[i] = 1;
        progress = 1;
        printf(1, "  P%d can finish. Work becomes: ", i);
        for(k = 0; k < m; k++)
          printf(1, "%d ", work[k]);
        printf(1, "\n");
      }
    }
  } while(progress);

  cnt = 0;
  for(i = 0; i < n; i++)
    if(!finish[i])
      dead[cnt++] = i;
  return cnt;
}

void run_detect(void)
{
  int dead[P], cnt, i;

  print_state(1);
  cnt = detect_deadlock(dead);
  if(cnt == 0){
    printf(1, "Result: NO deadlock.\n");
  } else {
    printf(1, "Result: DEADLOCK detected. Deadlocked processes: ");
    for(i = 0; i < cnt; i++)
      printf(1, "P%d ", dead[i]);
    printf(1, "\n");
  }
}

// Banker's safety check using Max, Allocation and Available.
int is_safe_state(void)
{
  int work[R], finish[P], seq[P];
  int i, j, k, ok, progress, count;

  for(j = 0; j < m; j++)
    work[j] = Available[j];
  for(i = 0; i < n; i++)
    finish[i] = 0;
  count = 0;
  do {
    progress = 0;
    for(i = 0; i < n; i++){
      if(finish[i])
        continue;
      ok = 1;
      for(j = 0; j < m; j++)
        if(Max[i][j] - Allocation[i][j] > work[j])
          ok = 0;
      if(ok){
        for(k = 0; k < m; k++)
          work[k] += Allocation[i][k];
        finish[i] = 1;
        seq[count++] = i;
        progress = 1;
      }
    }
  } while(progress);

  if(count == n){
    printf(1, "State is SAFE. Safe sequence: ");
    for(i = 0; i < n; i++)
      printf(1, "P%d ", seq[i]);
    printf(1, "\n");
    return 1;
  }
  printf(1, "State is UNSAFE.\n");
  return 0;
}

int request_resources(int p, int req[])
{
  int j;

  printf(1, "Process P%d requests [", p);
  for(j = 0; j < m; j++)
    printf(1, "%d%s", req[j], j < m - 1 ? ", " : "");
  printf(1, "]\n");

  for(j = 0; j < m; j++)
    if(req[j] > Max[p][j] - Allocation[p][j]){
      printf(1, "Error: request exceeds maximum claim.\n");
      return -1;
    }
  for(j = 0; j < m; j++)
    if(req[j] > Available[j]){
      printf(1, "P%d must wait: resources not available.\n", p);
      return -1;
    }

  for(j = 0; j < m; j++){
    Available[j] -= req[j];
    Allocation[p][j] += req[j];
  }
  if(is_safe_state()){
    printf(1, "Request GRANTED for P%d.\n", p);
    return 0;
  }
  for(j = 0; j < m; j++){
    Available[j] += req[j];
    Allocation[p][j] -= req[j];
  }
  printf(1, "Request DENIED for P%d (would lead to unsafe state).\n", p);
  return -1;
}

void test1(void)
{
  printf(1, "\n===== TEST 1: Deadlock detection (deadlocked matrix) =====\n");
  load_detect();
  run_detect();
}

void test2(void)
{
  printf(1, "\n===== TEST 2: Banker's Algorithm (safe state) =====\n");
  load_banker();
  print_state(0);
  is_safe_state();
}

void test3(void)
{
  int r1[3] = {1, 0, 2};
  int r2[3] = {0, 2, 0};

  printf(1, "\n===== TEST 3: State that becomes unsafe after a request =====\n");
  load_banker();
  print_state(0);
  is_safe_state();
  printf(1, "\n--- Step 1 ---\n");
  request_resources(1, r1);
  print_state(0);
  printf(1, "\n--- Step 2 ---\n");
  request_resources(0, r2);
  print_state(0);
}

void test4(void)
{
  int p1[2], p2[2], pids[2], dead[P], cnt, i;

  printf(1, "\n===== TEST 4: Real xv6 processes with pipes =====\n");
  pipe(p1);
  pipe(p2);

  pids[0] = fork();
  if(pids[0] == 0){
    char c;
    close(p1[0]);
    close(p2[1]);
    read(p2[0], &c, 1);
    exit();
  }
  pids[1] = fork();
  if(pids[1] == 0){
    char c;
    close(p2[0]);
    close(p1[1]);
    read(p1[0], &c, 1);
    exit();
  }
  close(p1[0]); close(p1[1]);
  close(p2[0]); close(p2[1]);

  sleep(50);
  printf(1, "Process A (pid %d) holds the write end of Pipe1 and waits to read Pipe2.\n", pids[0]);
  printf(1, "Process B (pid %d) holds the write end of Pipe2 and waits to read Pipe1.\n", pids[1]);
  printf(1, "Both are blocked. Running detection (R0 = Pipe1 write, R1 = Pipe2 write):\n");

  n = 2; m = 2;
  Allocation[0][0] = 1; Allocation[0][1] = 0;
  Allocation[1][0] = 0; Allocation[1][1] = 1;
  Request[0][0] = 0; Request[0][1] = 1;
  Request[1][0] = 1; Request[1][1] = 0;
  Available[0] = 0; Available[1] = 0;

  print_state(1);
  cnt = detect_deadlock(dead);
  if(cnt == 0){
    printf(1, "Result: NO deadlock.\n");
  } else {
    printf(1, "Result: DEADLOCK detected (circular wait). Deadlocked: ");
    for(i = 0; i < cnt; i++)
      printf(1, "%s ", dead[i] == 0 ? "A" : "B");
    printf(1, "\n");
  }

  kill(pids[0]);
  kill(pids[1]);
  wait();
  wait();
  printf(1, "Deadlocked processes killed and cleaned up.\n");
}

int main(int argc, char *argv[])
{
  int which = 0;

  if(argc > 1)
    which = atoi(argv[1]);

  printf(1, "=== Q3: Deadlock Detection and Banker's Algorithm ===\n");
  if(which == 0 || which == 1) test1();
  if(which == 0 || which == 2) test2();
  if(which == 0 || which == 3) test3();
  if(which == 0 || which == 4) test4();
  exit();
}
