#include "Globals.h"
#include "SharedMem.h"
#include "Peer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/sem.h>

union semun {
        int val;                   /* Value for SETVAL */
        struct semid_ds *buf;           /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;         /* Array for GETALL, SETALL */
        struct seminfo  *__buf;         /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

void GetArgs(int argc, char** argv);

int main(int argc, char** argv) {
        GetArgs(argc, argv);

        //create semaphore
        int sem_key = 0x1234;
        int sem_id = semget(sem_key,1,IPC_CREAT | 0600);
        if(sem_id < 0) {
                perror("Failed to create sem in main: ");
                exit(1);
        }

        //initialize semaphore set to 1
        union semun arg;
        arg.val = 1;
        if(semctl(sem_id, 0, SETVAL, arg) == -1) {
                perror("failed to initialize semaphore. ");
                exit(1);
        }

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
                        return Peer(NUM_ITERATIONS, sem_key);
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
        //delete semaphore
        if(semctl(sem_id,0,IPC_RMID)==-1) {
                perror("Not able to delete semaphore. ");
                exit(1);
        }
        return 0;
}

void GetArgs(int argc, char** argv) {
        if(argc-1 != 3) {
                printf("Not right number of arguments\n. Expected 3, got %d.\n", argc-1);
                exit(1); //not enough args
        }


        NUM_PEERS = atoi(argv[1]);
        NUM_ITERATIONS = atoi(argv[2]);
        NUM_ENTRIES = atoi(argv[3]);
}
