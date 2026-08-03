#include "../tcp_client.h"

#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

bool tcp_client_connect(TcpClient* c, const char* ip, uint16_t port) {
  c->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (c->fd == -1) {
    printf("Failed to create socket %s\n", strerror(errno));
    return false;
  }

  int flags = fcntl(c->fd, F_GETFL, 0);
  if (flags != -1) {
    fcntl(c->fd, F_SETFL, flags & ~O_NONBLOCK);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
    printf("Failed to set ip %s %s\n", ip, strerror(errno));
    return false;
  }

  int res = connect(c->fd, (struct sockaddr*)&addr, sizeof(addr));
  if (res == -1) {
    printf("Failed to connect %s:%d %s\n", ip, port, strerror(errno));
    return false;
  }

  return true;
}
