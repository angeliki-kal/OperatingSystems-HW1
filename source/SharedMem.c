#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include "SharedMem.h"

struct my_object {
  float base;
  int exponent;
};

int key = 99999;
int segment_id;

if(segment_id == -1 )  {
  error("shmget: ");
}

my_object* ptr = (my_object*);
shmat(segment_id, NULL, 0);

if(ptr == (void*)-1){
  error("shmat: ");
}

int detach_return = shmdt(ptr);
if(detach_return == -1)  {
  error("shmdt: ");
}

if(delete_shm == 1) {
    int ctl_return = shmctl(segment_id, IPC_RMID, NULL);
    if(ctl_return == -1) {
      error("shmctl for removal: ");
    }
}
