#include "Peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

int Peer(unsigned int num_iterations, int sem_key) {
  //create sem if it doesnt exist, else just get it
  int sem_id = semget(sem_key,1,IPC_CREAT | 0600);
  if(sem_id < 0) {
      perror("semget: ");
      exit(1);
  }

  printf("Peer %d started.\n", getpid());
  for(int i=0; i<num_iterations; i++) {
    printf("%d found sem with val %d\n", getpid(), semctl(sem_id, 0, GETVAL));
    SemOperation(sem_id, -1, 0); //TODO change 0 to entry_index
    //critical section
    printf("%d\n", getpid());
    sleep(5);
    SemOperation(sem_id, 1, 0); //TODO change 0 to entry_index
    printf("%d raised sem\n", getpid());
  }

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
