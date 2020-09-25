Operating Systems


Run:

./hw1 int num_peers, int num_iterations, int num_entries, int read_ratio

#Directories:

*main.c*: This is practically the coordinator of the program, which by using fork() creates Peers. In this directory semaphores and shared memory are created and respectively released.

*Peer.c*: In this directory is the implementation of the services of peers read/write. It extracts the results/statistics needed.

*Peer.h*: Definition of functions Peer, Semaphores Operations (SemOperation), as well as the function that produces random exponential distribution numbers (ran_expo). In order to decrease the time of the exponential distribution, LAMBDA must be increased.

*Globals.h*: The definition of global variables of the program.

*Entry.h*: Definition of struct Entry, which contains the variables necessary to operate the Entry.
Example: reads/writes as counters of reading/writing records in that Entry and write_queue_num which indicates how many Peers expect to write in that Entry.

#Shared Memory:

Implemented through the following functions:

shmget: This function creates and initializes the shared memory to 0. Its arguments are:

-   The key,
-   the size of the memory which is the product of sizeof (struct Entry) on the number of entries we give to the program,
-   with permissions IPC_CREAT | 0600.

shmat: The arguments are:

-   the id of the shared memory created via shmget,
-   the address of the shared memory that we have set as NULL so that the operating system chooses the address,
-   a flag that defines whether the memory is only for read or not. Putting 0 ensures that it is for both read and write.

shmdt: Its argument is:

-   The pointer to the address returned by the shmat function.

It disconnects the shared memory from the process.

shmctl: Its arguments are:

-   the process id,
-   the command that will be executed on the shared memory which in our case is the IPC_RMID that deletes the segment of the shared memory and finally,
-   the buffer that is set to 0.

#Semaphores:

sem_id: They control access to the Entry, every time we want to gain access to members of the struct Entry, for example, to increase a counter. The size of the semaphore *set* is as large as the number of Entries. That is, for every Enrty we have a semaphore. They are initialized at semval = 1.

write_sem_id: Imposes that at any given time there can be at most one writer in a specific Entry. The size of the semaphore set is as large as the number of Entries. They are initialized in semval = 1.

read_sem_id: Used by Peers when attempting to read in order to check for pending writes. If a write is pending and we tried to read, or in case the information we would read would be out-of-date, we give priority to the writers, before we allow a reader to read. This guarantees the reliability and accuracy of our data, but may delay readings. However, given that in our system and in general, the number of writers is much smaller than the readers, the impact is mitigated.

The implementation is done with the help of the special case of the semop function when given argument for op being the number 0. Some process that will call semop (sem_id, 0, index) will wait until this semaphore gets the value 0.

Initially, the semaphores of the set read_sem_id have the value 0. When the first writer takes over, the semval for the specific Entry changes to 1, so the next readers will be blocked until the last writer returns the semaphore value to 0 again.

#Ratio of Readers/Writers:

When a Peer is asked to decide whether to read or write:

if(rand() % READER_RATIO)

    read()

else

    write()

Therefore we can increase the number of reads by increasing the variable READER_RATIO. Eg READER_RATIO = 4 we will have about 3 read for each write, given that rand () returns numbers with uniform distribution.

After completing the repetitions of each process, the semaphores and the shared memory are released, while we also have all the statistics required by the exercise. These statistics are:

-    The average waiting time to commit an entry to write mode. That is, how long a writer waited (in line) until he undertook the registration.
-    The sum of readings that were made in the specific process.
-   The sum of writes that were made in the specific process.

When all the Peers are finished, the coordinator is activated again who waited () for all his children. Then all the reads and writes are printed in each Entry.
