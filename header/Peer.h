#ifndef PEER_H
#define PEER_H

#define LAMBDA 0.5

int Peer(int counter_sem_key, int write_sem_key, int read_sem_key, int shmem_id);

void SemOperation(int sem_id, int op, int i);

/*exponential distrubution
https://stackoverflow.com/questions/34558230/generating-random-numbers-of-exponential-distribution/34558404*/
double ran_expo(double lambda);

#endif
