#ifndef _tcp_client_h_
#define _tcp_client_h_

#include <stdint.h>
#include <stdbool.h>

typedef struct TcpClient {
  int fd;
} TcpClient;

bool tcp_client_connect(TcpClient* c, const char* ip, uint16_t port);

#endif // _tcp_client_h_
