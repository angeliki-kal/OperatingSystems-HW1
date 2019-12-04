#include "Globals.h"
#include "SharedMem.h"
#include "Peer.h"

#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void GetArgs(int argc, char** argv);

int main(int argc, char** argv) {
  GetArgs(argc, argv);

  //create shared mem (initialized to 0)
  key_t shm_key =  ftok("./source/main.c", 0x0001);
  printf("Got shared mem key %d.\n", shm_key);
  int shmem_id = shmget(shm_key,sizeof(unsigned int)*NUM_ENTRIES,IPC_CREAT | 0600);
  if (shmem_id<0) {
    perror("shmget: ");
    exit(1);
  }
  //attach shared mem
  unsigned int* shm_ptr = (unsigned int *) shmat(shmem_id, NULL, 0);
  if (shm_ptr == (unsigned int*)-1) {
    perror("shmat: ");
    exit(1);
  }

  for(int i=0; i<NUM_PEERS; i++) {
    if(fork() == 0) { //if child
      return Peer(NUM_ITERATIONS);
      exit(0);
    }
  }

  //wait for children to finish
  int status = 0;
  while (wait(&status) > 0);
  printf("All children done\n");

  //detach shared mem
  shmdt((void*)shm_ptr);

  //delete shared memory
  shmctl(shmem_id, IPC_RMID, 0);

}

void GetArgs(int argc, char** argv) {
  if(argc-1 != 3) {
    printf("Not right number of arguments\n. Expected 3, got %d.\n", argc-1);
    exit(1);  //not enough args
  }
  NUM_PEERS = atoi(argv[1]);
  NUM_ITERATIONS = atoi(argv[2]);
  NUM_ENTRIES = atoi(argv[3]);
}
