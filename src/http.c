#include "http.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* http_ver_to_str(HttpVersion ver) {
  switch (ver) {
  case HTTP_VER_1_0:
    return "HTTP/1.0";
  case HTTP_VER_1_1:
    return "HTTP/1.1";
  }
  return NULL;
}

HttpVersion str_to_http_ver(const char* str) {
  if (strncmp("HTTP/1.0", str, 8) == 0) {
    return HTTP_VER_1_0;
  } else if (strncmp("HTTP/1.1", str, 8) == 0) {
    return HTTP_VER_1_1;
  }
  return HTTP_VER_UNKNOWN;
}

const char* method_to_str(HttpMethod m) {
  switch (m) {
  case HTTP_GET:     return "GET";
  case HTTP_POST:    return "POST";
  case HTTP_PUT:     return "PUT";
  case HTTP_DELETE:  return "DELETE";
  case HTTP_PATCH:   return "PATCH";
  case HTTP_HEAD:    return "HEAD";
  case HTTP_OPTIONS: return "OPTIONS";
  case HTTP_CONNECT: return "CONNECT";
  case HTTP_TRACE:   return "TRACE";
  }
  return "UNKNOWN";
}

HttpMethod str_to_method(const char* str) {
  if (strncmp(str, "GET", 3) == 0)     return HTTP_GET;
  if (strncmp(str, "POST", 4) == 0)    return HTTP_POST;
  if (strncmp(str, "PUT", 3) == 0)     return HTTP_PUT;
  if (strncmp(str, "DELETE", 6) == 0)  return HTTP_DELETE;
  if (strncmp(str, "PATCH", 5) == 0)   return HTTP_PATCH;
  if (strncmp(str, "HEAD", 4) == 0)    return HTTP_HEAD;
  if (strncmp(str, "OPTIONS", 7) == 0) return HTTP_OPTIONS;
  if (strncmp(str, "CONNECT", 7) == 0) return HTTP_CONNECT;
  if (strncmp(str, "TRACE", 5) == 0)   return HTTP_TRACE;
  return HTTP_UNKNOWN;
}

bool parse_http_request(HttpRequest* req, char* request, int len) {
  if (!req || len <= 0) return false;
  memset(req, 0, sizeof(HttpRequest));
  
  char* outer_ptr = NULL;
  char* inner_ptr = NULL;
  
  char* lines = strtok_r(request, "\r\n", &outer_ptr);
  bool is_first_line = true;
  while (lines != NULL) {
    if (is_first_line) {
      char* method = strtok_r(lines, " ", &inner_ptr);
      char* path = strtok_r(NULL, " ", &inner_ptr);
      char* protocol = strtok_r(NULL, " ", &inner_ptr);

      req->method = str_to_method(method);
      memcpy(req->target, path, strlen(path));
      req->version = str_to_http_ver(protocol);

      is_first_line = false;
    } else {
      char* key = strtok_r(lines, ":", &inner_ptr);
      char* value = strtok_r(NULL, "", &inner_ptr);

      if (key && value) {
        if (*value == ' ') ++value;
        HttpHeader* h = &req->headers[req->num_headers];
        memcpy(h->name, key, strlen(key));
        memcpy(h->value, value, strlen(value));
        ++req->num_headers;
      }
    }

    lines = strtok_r(NULL, "\r\n", &outer_ptr);
  }

  return true;
}

const char* http_code_to_str(HttpStatusCode code) {
  switch (code) {
  case HTTP_CODE_200_OK:
    return "200 OK";
  case HTTP_CODE_403_FORBIDDEN:
    return "403 Forbiddent";
  case HTTP_CODE_404_NOT_FOUND:
    return "404 Not Found";
  default:
    return "UNKNOWN";
  }
}

HttpStatusCode str_to_http_code(const char* str) {
  if (strncmp(str, "200 OK", 6) == 0) {
    return HTTP_CODE_200_OK;
  } else if (strncmp(str, "403 Forbiddent", 14) == 0) {
    return HTTP_CODE_403_FORBIDDEN;
  } else if (strncmp(str, "404 Not Found", 13) == 0) {
    return HTTP_CODE_404_NOT_FOUND;
  }
  return HTTP_CODE_UNKNOWN;
}

bool parse_http_response(HttpResponse* res, char* buf, int len) {
  if (!buf || len <= 0) return false;

  memset(res, 0, sizeof(HttpResponse));

  char* outer_ptr = NULL;
  char* inner_ptr = NULL;
  
  char* lines = strtok_r(buf, "\r\n", &outer_ptr);
  bool is_first_line = true;
  while (lines != NULL) {
    if (is_first_line) {
      char* v = strtok_r(lines, " ", &inner_ptr);
      res->version = str_to_http_ver(v);
      char* sc = strtok_r(NULL, "", &inner_ptr);
      res->status_code = str_to_http_code(sc);
      is_first_line = false;
    } else {
      char* key = strtok_r(lines, ":", &inner_ptr);
      char* value = strtok_r(NULL, "", &inner_ptr);

      if (key && value) {
        if (*value == ' ') ++value;
        HttpHeader* h = &res->headers[res->num_headers];
        memcpy(h->name, key, strlen(key));
        memcpy(h->value, value, strlen(value));
        ++res->num_headers;
      }
    }

    lines = strtok_r(NULL, "\r\n", &outer_ptr);
  }

  return true;
}
