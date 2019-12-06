#include "Peer.h"
#include "Globals.h"

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>

int Peer(int sem_key, int write_sem_key, int shmem_id) {
  //init rand num generator
  srand(getpid());
  //create sem if it doesnt exist, else just get it
  int sem_id = semget(sem_key,NUM_ENTRIES,IPC_CREAT | 0600);
  if(sem_id < 0) {
      perror("semget in Peer: ");
      exit(1);
  }
  //get write semaphore set
  int write_sem_id = semget(write_sem_key,NUM_ENTRIES,IPC_CREAT | 0600);
  if(sem_id < 0) {
      perror("semget in Peer: ");
      exit(1);
  }
  //attach shared mem
  unsigned int* shm_ptr = shmat(shmem_id, NULL, 0);
  if (shm_ptr == (unsigned int*)-1) {
          perror("shmat: ");
          exit(1);
  }
  printf("Peer %d started.\n", getpid());
  for(int i=0; i<NUM_ITERATIONS; i++) {
    //choose random entry
    int entry_index = rand() % NUM_ENTRIES;
    //choose weather to read or write
    if(rand() % 2) {  //read entry
      double read_time = ran_expo(0.1);
      printf("%d reading from entry %d for %f seconds\n", getpid(), entry_index, read_time);
      sleep(read_time);
      //counter semaphore down
      SemOperation(sem_id, -1, entry_index);
      //critical section for counter
      shm_ptr[entry_index]++;
      //counter sempahore up
      SemOperation(sem_id, 1, entry_index);
      printf("%d finished reading from entry %d\n", getpid(), entry_index);
    }
    else {       //write entry
      double write_time = ran_expo(0.05);
      //write semaphore down
      SemOperation(write_sem_id, -1, entry_index);
      printf("%d writing to entry %d for %f seconds\n", getpid(), entry_index, write_time);
      sleep(write_time);  //fake write time
      //counter semaphore down
      SemOperation(sem_id, -1, entry_index);
      //critical section for counter
      shm_ptr[entry_index]++;
      //counter sempahore up
      SemOperation(sem_id, 1, entry_index);
      //write semaphore up
      SemOperation(write_sem_id, 1, entry_index);
      printf("%d done writing to %d\n", getpid(), entry_index);

    }
  }
  //detach shared memory
  shmdt((void*)shm_ptr);
  printf("Peer %d ended.\n", getpid());
  return 0;
}

void SemOperation(int sem_id, int op, int i) {
  //semaphor up or down
  struct sembuf sops;
  sops.sem_num = i;
  sops.sem_op = op;
  sops.sem_flg = 0;
  if(semop(sem_id, &sops, 1) !=0){
    perror("Semaphor Operation Failed. ");
    exit(1);
  }
}

/*https://stackoverflow.com/questions/34558230/generating-random-numbers-of-exponential-distribution/34558404*/
double ran_expo(double lambda){
    double u;
    u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}
