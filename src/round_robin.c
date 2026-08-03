#include "round_robin.h"

Server* get_next_server(RoundRobin* rr, Server* servers, int servers_count) {
  Server* next_server = &servers[rr->current_index];
  rr->current_index = (rr->current_index + 1) % servers_count;
  return next_server;
}
