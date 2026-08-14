#ifndef _config_h_
#define _config_h_

#include <stdint.h>

typedef struct Server {
  char ip[32];
  int port;
} Server;

typedef struct DomainConfig {
  const char* domain;
  Server servers[3];
} DomainConfig;

static struct Config {
  // Address of balancer
  const char* ip;
  uint16_t port;
  // Domain addresses and settings
  DomainConfig domains[2];
} CONFIG = {
  .ip = "127.0.0.1",
  .port = 80,
  .domains = {
    (DomainConfig) {
      .domain = "test.dev",
      .servers = {
        (Server){"127.0.0.1", 8080},
        (Server){"127.0.0.1", 8081},
        (Server){"127.0.0.1", 8082}
      },
    },
    // (DomainConfig) {
    //   .servers = {
    //     (Server){"127.0.0.1", 3000},
    //   },
    // },
  }
};

#endif // _config_h_
