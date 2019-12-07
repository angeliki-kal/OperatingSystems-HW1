#include "Peer.h"
#include "Globals.h"
#include "Entry.h"

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

int Peer(int sem_key, int write_sem_key, int read_sem_key, int shmem_id) {
        //init rand num generator
        srand(getpid());
        //create Entry sem if it doesnt exist, else just get it
        int sem_id = semget(sem_key,NUM_ENTRIES,IPC_CREAT | 0600);
        if(sem_id < 0) {
                perror("semget in Peer from Entry semaphore set: ");
                exit(1);
        }
        //get write semaphore set
        int write_sem_id = semget(write_sem_key,NUM_ENTRIES,IPC_CREAT | 0600);
        if(write_sem_id < 0) {
                perror("semget in Peer for write semaphore set: ");
                exit(1);
        }
        //get read semaphore set
        int read_sem_id = semget(read_sem_key,NUM_ENTRIES,IPC_CREAT | 0600);
        if(read_sem_id < 0) {
                perror("semget in Peer for read semaphore set: ");
                exit(1);
        }
        //attach shared mem
        struct Entry* shm_ptr = shmat(shmem_id, NULL, 0);
        if (shm_ptr == (struct Entry*)-1) {
                perror("shmat: ");
                exit(1);
        }
        int sum_read = 0, sum_write = 0; //process num of reads/writes
        double sum_wait_t = 0;  //process write queue wait time
        printf("Peer %d started.\n", getpid());
        for(int i=0; i<NUM_ITERATIONS; i++) {
                //choose random entry
                int entry_index = rand() % NUM_ENTRIES;
                //choose weather to read or write

                if(rand() % READER_RATIO) { //read entry
                        double read_time = ran_expo(LAMBDA);
                        //check if someone is writing, if yes: wait
                        SemOperation(read_sem_id, 0, entry_index);

                        printf("%d reading from entry %d for %f seconds\n", getpid(), entry_index, read_time);
                        sleep(read_time);
                        //critical section for counter
                        SemOperation(sem_id, -1, entry_index);
                        shm_ptr[entry_index].reads++;
                        SemOperation(sem_id, 1, entry_index);

                        printf("%d finished reading from entry %d\n", getpid(), entry_index);
                        sum_read++;
                }
                else { //write entry
                        double write_time = ran_expo(LAMBDA);
                        struct timespec start, end;
                        //add to writer queue
                        clock_gettime(CLOCK_REALTIME, &start); //stopwatch for wait time in queue - start
                        SemOperation(sem_id, -1, entry_index);
                        if(++(shm_ptr[entry_index].writer_queue_num) == 1) {
                          //first writer in queue, blocks readers
                          printf("%d is the first writer in queue, queue now has %d writers\n", getpid(), shm_ptr[entry_index].writer_queue_num);
                          SemOperation(read_sem_id, 1, entry_index);
                        }
                        SemOperation(sem_id, 1, entry_index);

                        //start writing
                        SemOperation(write_sem_id, -1, entry_index);
                        clock_gettime(CLOCK_REALTIME, &end); //stopwatch for wait time in queue - end
                        sum_wait_t += (double)(end.tv_sec - start.tv_sec) +
                                      (double)(end.tv_nsec - start.tv_nsec)/1.0e9;
                        printf("%d writing to entry %d for %f seconds\n", getpid(), entry_index, write_time);
                        sleep(write_time); //fake write time
                        //writing done
                        SemOperation(write_sem_id, 1, entry_index);

                        SemOperation(sem_id, -1, entry_index);
                        //critical section for counters
                        shm_ptr[entry_index].writes++;
                        SemOperation(sem_id, 1, entry_index);

                        //remove writer from queue
                        SemOperation(sem_id, -1, entry_index);
                        if(--(shm_ptr[entry_index].writer_queue_num) == 0) { //last writer in queue frees readers
                          SemOperation(read_sem_id, -1, entry_index); //bring read_sem back to 0, wake up readers
                        }
                        SemOperation(sem_id, 1, entry_index);

                        printf("%d done writing to %d\n", getpid(), entry_index);
                        sum_write++;
                }
        }
        //print stats
        if(sum_write == 0) {
                printf("%d's read sum : %d, write sum: %d, average_wait_time: -no writes performed-\n", getpid(), sum_read, sum_write);
        }
        else {
                printf("%d's read sum : %d, write sum: %d, average_wait_time: %.4f\n", getpid(), sum_read, sum_write, sum_wait_t / (double)sum_write);
        }
        //detach shared memory
        shmdt((void*)shm_ptr);
        return 0;
}

void SemOperation(int sem_id, int op, int i) {
        //semaphor up or down
        struct sembuf sops;
        sops.sem_num = i;
        sops.sem_op = op;
        sops.sem_flg = 0;
        if(semop(sem_id, &sops, 1) !=0) {
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
