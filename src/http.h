#ifndef _http_h_
#define _http_h_

#include <stdint.h>
#include <stdbool.h>

#define MAX_HEADERS 128
#define STR_FROM(X) (String) { (X), (int) sizeof(X)-1 }
#define UNPACK(X) (X).len, (X).ptr
#define COUNT(X) (sizeof(X) / sizeof((X)[0]))

typedef struct String {
  char* ptr;
  int len;
} String;

typedef enum HttpVersion {
  HTTP_VER_UNKNOWN,
  HTTP_VER_1_0,
  HTTP_VER_1_1,
} HttpVersion;

const char* http_ver_to_str(HttpVersion ver);
HttpVersion str_to_http_ver(const char* str);

typedef enum HttpMethod {
  HTTP_UNKNOWN,
  HTTP_GET,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE,
  HTTP_PATCH,
  HTTP_HEAD,
  HTTP_OPTIONS,
  HTTP_CONNECT,
  HTTP_TRACE,
} HttpMethod;

const char* method_to_str(HttpMethod m);
HttpMethod str_to_method(const char* str);

typedef struct HttpHeader {
  char name[1024];
  char value[1024];
} HttpHeader;

typedef struct HttpRequest {
  HttpMethod method;
  char target[1024];
  HttpVersion version;

  int num_headers;
  HttpHeader headers[MAX_HEADERS];
} HttpRequest;

bool parse_http_request(HttpRequest* req, char* buf, int len);

typedef enum HttpStatusCode {
  HTTP_CODE_UNKNOWN,
  HTTP_CODE_200_OK,
  HTTP_CODE_403_FORBIDDEN,
  HTTP_CODE_404_NOT_FOUND,
} HttpStatusCode;

const char* http_code_to_str(HttpStatusCode code);
HttpStatusCode str_to_http_code(const char* str);

typedef struct HttpResponse {
  HttpVersion version;
  HttpStatusCode status_code;

  int num_headers;
  HttpHeader headers[MAX_HEADERS];
} HttpResponse;

bool parse_http_response(HttpResponse* res, char* buf, int len);

#endif // _http_h_
