#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

sem_t* track;
enum BankAccountTurn {BankAccount = 0, Turn =  1};

void  ClientProcess(int []);

// int  main(int  argc, char *argv[])
// {
//      int    ShmID;
//      int    *ShmPTR;
//      pid_t  pid;
//      int    status;

//      if (argc != 5) {
//           printf("Use: %s #1 #2 #3 #4\n", argv[0]);
//           exit(1);
//      }

//      ShmID = shmget(IPC_PRIVATE, 4*sizeof(int), IPC_CREAT | 0666);
//      if (ShmID < 0) {
//           printf("*** shmget error (server) ***\n");
//           exit(1);
//      }
//      printf("Server has received a shared memory of four integers...\n");

//      ShmPTR = (int *) shmat(ShmID, NULL, 0);
//      if (*ShmPTR == -1) {
//           printf("*** shmat error (server) ***\n");
//           exit(1);
//      }
//      printf("Server has attached the shared memory...\n");

//      ShmPTR[0] = atoi(argv[1]);
//      ShmPTR[1] = atoi(argv[2]);
//      ShmPTR[2] = atoi(argv[3]);
//      ShmPTR[3] = atoi(argv[4]);
//      printf("Server has filled %d %d %d %d in shared memory...\n",
//             ShmPTR[0], ShmPTR[1], ShmPTR[2], ShmPTR[3]);

//      printf("Server is about to fork a child process...\n");
//      pid = fork();
//      if (pid < 0) {
//           printf("*** fork error (server) ***\n");
//           exit(1);
//      }
//      else if (pid == 0) {
//           ClientProcess(ShmPTR);
//           exit(0);
//      }

//      wait(&status);
//      printf("Server has detected the completion of its child...\n");
//      shmdt((void *) ShmPTR);
//      printf("Server has detached its shared memory...\n");
//      shmctl(ShmID, IPC_RMID, NULL);
//      printf("Server has removed its shared memory...\n");
//      printf("Server exits...\n");
//      exit(0);
// }

// void  ClientProcess(int  SharedMem[])
// {
//      printf("   Client process started\n");
//      printf("   Client found %d %d %d %d in shared memory\n",
//                 SharedMem[0], SharedMem[1], SharedMem[2], SharedMem[3]);
//      printf("   Client is about to exit\n");
// }

int main(int argc, char * argv[]) {
  int ShmID;
  int *ShmPTR;
  pid_t pid;
  int status;
  int fd,nloop=10,zero=0,*counter_ptr;
  
  if ((track = sem_open("examplesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }
  
  time_t t;
  srandom((unsigned) time( & t));
  ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
  if (ShmID < 0) {
    printf("*** shmget error (server) ***\n");
    exit(1);
  }
  ShmPTR = (int * ) shmat(ShmID, NULL, 0);
  if ( * ShmPTR == -1) {
    printf("*** shmat error (server) ***\n");
    exit(1);
  }  
  ShmPTR[BankAccount] = 0;
  ShmPTR[Turn] = 0;
  pid = fork();
  if (pid < 0) {
    exit(1);
  } else if (pid == 0) {
    ClientProcess(ShmPTR);
    exit(0);
  }
  
  // parent process
  int account_num;
  int std_balance;
  int i;

  for (i = 0; i < 25; ++i) {
    sleep(random() % 6);
    account_num = ShmPTR[BankAccount];
    while (ShmPTR[Turn] != 0) {}

    if (account_num <= 100) {
      std_balance = random() % 101;

      if (std_balance % 2 == 0) {
        account_num += std_balance;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", std_balance, account_num);
      } else {
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
    } else {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account_num);
    }
    ShmPTR[BankAccount] = account_num;
    ShmPTR[Turn] = 1;
    sem_post(track);
  }

  wait( & status);
  printf("Server has detected the completion of its child...\n");
  shmdt((void * ) ShmPTR);
  printf("Server has detached its shared memory...\n");
  shmctl(ShmID, IPC_RMID, NULL);
  printf("Server has removed its shared memory...\n");
  printf("Server exits...\n");
  exit(0);
}

void ClientProcess(int SharedMem[]) {
  int account_num;
  int std_balance;
  int j;

  for (j = 0; j < 25; ++j) {
    sleep(random() % 6);
    account_num = SharedMem[BankAccount];
    sem_wait(track);
    while (SharedMem[Turn] != 1) {}
    std_balance = random() % 51;
    printf("Poor Student needs $%d\n", std_balance);

    if (std_balance <= account_num) {
      account_num -= std_balance;
      printf("Poor Student: Withdraws $%d / Balance = $%d\n", std_balance, account_num);
    } else {
      printf("Poor Student: Not Enough Cash ($%d)\n", account_num );
    }   
    SharedMem[BankAccount] = account_num;
    SharedMem[Turn] = 0;
    sem_post(track);
  }
}