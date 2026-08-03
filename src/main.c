#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

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

void handle_client_connection(TcpServer* s, int fd, Server* servers, int servers_count) {
  char buf[2048];
  int res = recv(fd, buf, sizeof(buf), 0);
  if (res >= 0) {
    Server* server = get_next_server(&s->rr, servers, servers_count);
    TcpClient c;
    bool is_connected = tcp_client_connect(&c, server->ip, server->port);
    if (!is_connected) {
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
      return;
    }

    size_t n = strlen(buf);
    size_t max_size = sizeof(buf);

    char *keep_alive_pos = strstr(buf, "Connection: keep-alive");
    if (!keep_alive_pos) {
      keep_alive_pos = strstr(buf, "connection: keep-alive");
    }

    // if (keep_alive_pos) {
    //   memcpy(keep_alive_pos, "Connection: close     ", 22);
    // } else {
    //   // char *header_end = strstr(buf, "\r\n\r\n");
    //   // if (header_end) {
    //   //   size_t pos = header_end - buf;
    //   //   // const char *insert_text = "\r\nConnection: close";
        
    //   //   // c_str_insert(buf, pos, insert_text, n, max_size);
    //   //   // n += strlen(insert_text);
    //   // }
    // }

    char *header_end = strstr(buf, "\r\n\r\n");
    if (header_end) {
      size_t pos = header_end - buf;
      const char *insert_text = "\r\nX-Forwarded-Host: 127.0.0.1:3030";
      
      c_str_insert(buf, pos, insert_text, n, max_size);
      n += strlen(insert_text);
    }
    printf("\n%s\n", buf);

    send(c.fd, buf, sizeof(buf), 0);

    char buf2[2048];
    int bytes_received = 0;
    while ((bytes_received = recv(c.fd, buf2, sizeof(buf2), 0)) > 0) {
      int n = send(fd, buf2, bytes_received, 0);
      // printf("Send response:\n%s\n", buf2);
      if (n <= 0) {
        printf("Connection to client was broken!\n");
        break;
      }
    }

    epoll_ctl(fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    close_connection(c.fd);
  }
}

int main(int argc, char** argv) {
  print_logo();

  TcpServer server = tcp_server_listen("127.0.0.1", 3030, true);
  server.rr.current_index = 0;
  run_tcp_server(&server, handle_client_connection);
  return 0;
}

// int main(int argc, char** argv) {
//   char http_request[] = 
//     "GET /index.html HTTP/1.1\r\n"
//     "Host: www.example.com\r\n"
//     "User-Agent: C-Program/1.0\r\n"
//     "Accept: text/html\r\n"
//     "Connection: close\r\n"
//     "\r\n";

//   HttpRequest req = {
//     .num_headers = 0,
//   };
//   bool is_succ = parse_http_request(&req, http_request, strlen(http_request));
//   if (is_succ) {
//     printf("Method: %s\n", method_to_str(req.method));
//     printf("Target: %s\n", req.target);
//     printf("Version: %s\n", http_ver_to_str(req.version));

//     for (int i = 0; i < req.num_headers; i++) {
//       HttpHeader* h = &req.headers[i];
//       printf("%s: %s\n", h->name, h->value);
//     }
//   } else {
//     printf("Failed to parse http request header\n");
//   }

//   printf("\n");
//   printf("\n");
//   printf("\n");

//   char raw_response[] = 
//     "HTTP/1.1 200 OK\r\n"
//     "Server: CustomCServer/1.0\r\n"
//     "Content-Type: text/html; charset=UTF-8\r\n"
//     "Content-Length: 137\r\n"
//     "Connection: close\r\n"
//     "\r\n"
//     "<!DOCTYPE html>\n"
//     "<html>\n"
//     "<head><title>C Server</title></head>\n"
//     "<body><h1>Erfolg!</h1><p>Die HTML-Seite wurde geladen.</p></body>\n"
//     "</html>";

//   HttpResponse res;
//   is_succ = parse_http_response(&res, raw_response, strlen(raw_response));
//   if (is_succ) {
//     printf("Version: %s\n", http_ver_to_str(res.version));
//     printf("StatusCode: %s\n", http_code_to_str(res.status_code));
//     for (int i = 0; i < res.num_headers; i++) {
//       HttpHeader* h = &res.headers[i];
//       printf("%s: %s\n", h->name, h->value);
//     }
//   } else {
//     printf("Failed to parse http response header\n");
//   }

//   return 0;
// }
