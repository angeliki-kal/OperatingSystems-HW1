#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int Peer(unsigned int num_iterations) {
  printf("Peer %d started.\n", getpid());
  for(int i=0; i<num_iterations; i++) {
    printf("%d\n", getpid());

  }
  printf("Peer %d ended.\n", getpid());
  return 0;
}
