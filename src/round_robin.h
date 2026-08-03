#ifndef _round_robin_h_
#define _round_robin_h_

#include "config.h"

typedef struct RoundRobin {
  int servers_count;
  int current_index;
} RoundRobin;

Server* get_next_server(RoundRobin* rr, Server* servers, int servers_count);

#endif // _round_robin_h_
