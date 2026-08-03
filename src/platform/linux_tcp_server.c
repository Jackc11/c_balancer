#include "../tcp_server.h"

#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define MAX_EVENTS 32

static bool set_non_blocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return false;

  flags |= O_NONBLOCK;
  int s = fcntl(fd, F_SETFL, flags);
  if (s == -1) return false;

  return true;
}

static bool epoll_ctl_add(int epfd, int fd, uint32_t events) {
  struct epoll_event ev;
  ev.events = events;
  ev.data.fd = fd;
  int res = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
  if (res == -1) {
    printf("Epoll ctl add error %s\n", strerror(errno));
    return false;
  }
  return true;
}

static void accept_connection(TcpServer* s) {
  struct sockaddr_in cli_addr;
  socklen_t socklen = sizeof(cli_addr);

  int conn_sock = accept(s->fd, (struct sockaddr*)&cli_addr, &socklen);
  char addr[16];
  inet_ntop(AF_INET, (char *)&(cli_addr.sin_addr), addr, sizeof(cli_addr));
  printf("Client %s:%d connected\n", addr, ntohs(cli_addr.sin_port));

  // set_non_blocking(conn_sock);
  epoll_ctl_add(s->epfd, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
}

TcpServer tcp_server_listen(const char* ip, uint16_t port, bool reuse_address) {
  TcpServer s;
  s.running = true;

  s.fd = socket(AF_INET, SOCK_STREAM, 0);
  if (s.fd == -1) {
    printf("Failed to create socket %s\n", strerror(errno));
    return (TcpServer){};
  }

  if (reuse_address) {
    int opt = 1;
    int res = setsockopt(s.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (res == -1) {
      printf("Failed to set reuse address %s\n", strerror(errno));
      close(s.fd);
      return (TcpServer){};
    }
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  int res = inet_pton(AF_INET, ip, &server_addr.sin_addr);
  if (res == -1) {
    printf("Failed to set ip address %s\n", ip);
    close(s.fd);
    return (TcpServer){};
  }

  res = bind(s.fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (res == -1) {
    printf("Failed to bind address %s\n", strerror(errno));
    close(s.fd);
    return (TcpServer){};
  }

  set_non_blocking(s.fd);

  res = listen(s.fd, SOMAXCONN);
  if (res == -1) {
    printf("Failed to start listen %s\n", strerror(errno));
    close(s.fd);
    return (TcpServer){};
  }

  s.epfd = epoll_create1(0);
  if (s.epfd == -1) {
    printf("Failed to create epoll %s\n", strerror(errno));
    close(s.fd);
    return (TcpServer){};
  }
  epoll_ctl_add(s.epfd, s.fd, EPOLLIN | EPOLLOUT | EPOLLET);
  
  printf("Server started at %s:%d\n", ip, port);

  return s;
}

void run_tcp_server(TcpServer* s, OnConnectionHandle func) {
  int nfds = 0;
  struct epoll_event events[MAX_EVENTS];
  while (s->running) {
    nfds = epoll_wait(s->epfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == s->fd) {
        accept_connection(s);
      } else if (events[i].events & EPOLLIN) {
        // func(s, events[i].data.fd, CONFIG.domains[0].servers, 3);
        func(s, events[i].data.fd, CONFIG.domains[1].servers, 1);
      } else {
        printf("Unexpected disconnect\n");
      }

      if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
        printf("Connection closed\n");
        epoll_ctl(s->epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
        close(events[i].data.fd);
      }
    }
  }
}

void close_connection(int fd) {
  close(fd);
}
