#ifndef OS_GLOBALS_H
#define OS_GLOBALS_H

int NUM_PEERS , NUM_ITERATIONS, NUM_ENTRIES;

/*Controls the probability of a Peer performing a read opeartion.
i.e if the READER_RATIO is 4, then it is 3 times more likely to have a reader
that it is to have a writer. if the READER_RATIO is 2 it is equality likely.
reader if: rand() % READER_RATIO > 0 */
int READER_RATIO;

#endif
