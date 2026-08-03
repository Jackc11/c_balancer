#ifndef _tcp_server_h_
#define _tcp_server_h_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "round_robin.h"

typedef struct TcpServer {
  int fd;
  int epfd;
  bool running;
  RoundRobin rr;
} TcpServer;

typedef void(*OnConnectionHandle)(TcpServer* s, int fd, Server* servers, int servers_count);

TcpServer tcp_server_listen(const char* ip, uint16_t port, bool reuse_address);
void run_tcp_server(TcpServer* s, OnConnectionHandle func);
void close_connection(int fd);

#endif // _tcp_server_h_
