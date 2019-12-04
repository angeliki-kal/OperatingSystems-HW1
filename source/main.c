#include "Globals.h"


void GetArgs(int argc, char** argv);

int main(int argc, char** argv) {
  GetArgs(argc, argv);



  for(int i=0; i<NUM_PEERS; i++) {
    pid_t f = fork();
    if(f == 0) { //if child
      return Peer(NUM_ENTRIES);
    }
  }
}

void GetArgs(int argc, char** argv) {
  if(argc != 3) {
    printf("Not right number of arguments\n. Expected 3, got %d.\n", argc);
    exit(1);  //not enough args
  }
  NUM_PEERS = atoi(argv[1]);
  NUM_ITERATIONS = atoi(argv[2]);
  NUM_ENTRIES = atoi(argv[3]);
}
