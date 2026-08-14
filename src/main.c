#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"
#include "round_robin.h"
#include "tcp_client.h"
#include "tcp_server.h"

void c_str_insert(char* dst, size_t pos, const char* in, size_t len, size_t max_buf_size) {
  size_t in_len = strlen(in);

  if (len + in_len >= max_buf_size) {
    printf("The buffer is too small!\n");
    return;
  }

  memmove(dst + pos + in_len, dst + pos, len - pos + 1);
  memcpy(dst + pos, in, in_len);
}

void print_logo(void) {
  printf("                                    \n");
  printf("                                    \n");
  printf("⠀⠀⠀⠀⣀⣤⣴⣶⣶⣦⣄⡀⠀⠀⠀⠀⠀⠀⢀⣤⣶⣶⣶⣶⣤⣀⠀⠀⠀⠀\n");
  printf("⠀⠀⢀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠀⠀⠀⢀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠀⠀\n");
  printf("⠀⠀⣾⣿⠟⠋⠉⠀⠀⠉⠙⢿⣿⣿⡄⠀⢠⣿⣿⡿⠋⠉⠀⠀ ⠉⠻⣿⣷⠀⠀\n");
  printf("⠀⢸⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠙⣿⣿⣿⣿⣿⠏⠀⠀⠀⠀⠀⠀⠀ ⠸⣿⡇⠀\n");
  printf("⠀⢸⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢈⣿⣿⣿⣁⠀⠀⠀⠀⠀⠀⠀⠀⠀ ⣿⡇⠀\n");
  printf("⠀⢸⣿⡆⠀⠀⠀⠀⠀⠀⠀⠀⢀⣾⣿⣿⣿⣿⣄⠀⠀⠀⠀⠀⠀⠀ ⢰⣿⡇⠀\n");
  printf("⠀⠀⢿⣿⣄⡀⠀⠀⠀⢀⣀⣴⣿⣿⡿⠁⠻⣿⣿⣦⣀⠀⠀⠀⢀⣀⣴⣿⡿⠀⠀\n");
  printf("⠀⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⠀⠀⠀⠀⠙⢿⣿⣿⣿⣿⣿⣿⣿⡿⠁⠀⠀\n");
  printf("⠀⠀⠀⠀⠈⠛⠻⠿⠿⠿⠛⠉⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⠿⠿⠿⠿⠛⠁⠀⠀⠀\n");
  printf("                                    \n");
  printf("        Momentum Inc.               \n");
  printf("                                    \n");
}

void handle_502(int fd) {
  const char* http_502_response = 
    "HTTP/1.1 502 Bad Gateway\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 42\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Error 502: Backend server is unreachable.";
      
  send(fd, http_502_response, strlen(http_502_response), 0);
  epoll_ctl(fd, EPOLL_CTL_DEL, fd, NULL);
  close(fd);
}

void handle_404(int fd) {
  const char* http_404_response = 
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 8\r\n"
    "\r\n"
    "Page not found.";
  send(fd, http_404_response, strlen(http_404_response), 0);
}

void inster_x_forwarded_host_header(char* buf, size_t* buf_size, size_t max_size) {
  char* header_end = strstr(buf, "\r\n\r\n");
  if (header_end) {
    size_t pos = header_end - buf;
    char insert_text[64];
    snprintf(insert_text, sizeof(insert_text), "\r\nX-Forwarded-Host: %s:%d", CONFIG.ip, CONFIG.port);
    
    c_str_insert(buf, pos, insert_text, *buf_size, max_size);
    *buf_size += strlen(insert_text);
  }
}

bool receive_header_from_connection(int fd, char* buf, size_t buf_size) {
  int res = recv(fd, buf, buf_size, 0); 
  if (res <= 0) {
    close(fd);
    printf("Failed to receive header from connection");
    return false;
  }
  return true;
}

void forward_buffer_to_backend(TcpServer* s, Server* server, HttpRequest* req, int fd, char* buf, size_t buf_size, size_t max_size) {
  bool processed = false;
  HttpHeader* h = NULL;
  for (int i = 0; i < req->num_headers; i++) {
    h = &req->headers[i];
    if (strcasecmp(h->name, "Host") == 0) {
      if (strncmp(h->value, CONFIG.domains[0].domain, strlen(h->value)) == 0) {
        processed = true;

        TcpClient c;
        bool is_connected = tcp_client_connect(&c, server->ip, server->port);
        if (!is_connected) {
          handle_502(fd);
          return;
        }

        inster_x_forwarded_host_header(buf, &buf_size, max_size);
        printf("Sending request: %s\n", buf);

        // Send the received header from connection
        send(c.fd, buf, buf_size, 0);

        char buf2[2048];
        int bytes_received = 0;
        while ((bytes_received = recv(c.fd, buf2, sizeof(buf2), 0)) > 0) {
          int sent_bytes = send(fd, buf2, bytes_received, 0);
          if (sent_bytes <= 0) {
            printf("Connection to client was broken!\n");
            break;
          }
        }
        close_connection(c.fd);
      }
    }
  }

  if (!processed) {
    handle_404(fd);
  }

  epoll_ctl(fd, EPOLL_CTL_DEL, fd, NULL);
  close(fd);
}

void handle_client_connection(TcpServer* s, int fd, Server* servers, int servers_count) {
  char buf[2048];
  memset(buf, 0, sizeof(buf));

  bool res = receive_header_from_connection(fd, buf, sizeof(buf));
  if (!res) return;

  size_t n = strlen(buf);
  size_t max_size = sizeof(buf);

  char temp_buf[2048]; 
  memset(temp_buf, 0, sizeof(temp_buf));
  memcpy(temp_buf, buf, n);

  HttpRequest req;
  parse_http_request(&req, temp_buf, n);

  Server* server = get_next_server(&s->rr, servers, servers_count);
  forward_buffer_to_backend(s, server, &req, fd, buf, n, max_size);
}

int main(int argc, char** argv) {
  setvbuf(stdout, NULL, _IONBF, 0); 

  print_logo();

  TcpServer server = tcp_server_listen(CONFIG.ip, CONFIG.port, true);
  server.rr.current_index = 0;
  run_tcp_server(&server, handle_client_connection);
  return 0;
}
