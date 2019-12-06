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

        //create counter semaphore
        int sem_key = 0x1111;
        int sem_id = semget(sem_key,NUM_ENTRIES,IPC_CREAT | 0600);
        if(sem_id < 0) {
                perror("Failed to create sem in main: ");
                exit(1);
        }
        //create write semaphore set
        int write_sem_key = 0x2222;
        int write_sem_id = semget(write_sem_key,NUM_ENTRIES,IPC_CREAT | 0600);
        if(sem_id < 0) {
            perror("semget in Peer: ");
            exit(1);
        }

        //initialize counter semaphore set to 1
        union semun arg;
        unsigned short *sem_val_array = malloc(sizeof(unsigned short)*NUM_ENTRIES);
        for(int i=0; i<NUM_ENTRIES; i++) {
          sem_val_array[i] = 1;
        }
        arg.array = sem_val_array;
        if(semctl(sem_id, 0, SETALL, arg) == -1) {
                perror("failed to initialize semaphore. ");
                exit(1);
        }
        //initialize write semaphore set to 1
        if(semctl(write_sem_id, 0, SETALL, arg) == -1) {
                perror("failed to initialize semaphore. ");
                exit(1);
        }

        //create shared mem (initialized to 0)
        key_t shm_key =  ftok("./source/main.c", 0x0001);
        printf("Got shared mem key %d.\n", shm_key);
        int shmem_id = shmget(shm_key,sizeof(unsigned int)*NUM_ENTRIES,IPC_CREAT | 0600);
        if (shmem_id<0) {
                perror("Failed to create shared mem in coordinator: ");
                exit(1);
        }
        //attach shared mem
        unsigned int* shm_ptr = shmat(shmem_id, NULL, 0);
        if (shm_ptr == (unsigned int*)-1) {
                perror("shmat: ");
                exit(1);
        }

        for(int i=0; i<NUM_PEERS; i++) {
                if(fork() == 0) { //if child
                        return Peer(sem_key, write_sem_key, shmem_id);
                        exit(0);
                }
        }

        //wait for children to finish
        int status = 0;
        while (wait(&status) > 0);
        printf("All children done\n");

        for (int i=0; i<NUM_ENTRIES; i++){
          printf("Entry %d accessed %d times.\n", i, shm_ptr[i]);
        }
        //detach shared mem
        shmdt((void*)shm_ptr);
        //delete shared memory
        shmctl(shmem_id, IPC_RMID, 0);
        //delete counter semaphore
        if(semctl(sem_id,0,IPC_RMID)==-1) {
                perror("Not able to delete semaphore. ");
                exit(1);
        }
        //delete write semaphore
        if(semctl(write_sem_id,0,IPC_RMID)==-1) {
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
