#ifndef ENTRY_H
#define ENTRY_H

struct Entry {
  unsigned int reads, writes, writer_queue_num;
};

#endif
