cmake_minimum_required(VERSION 3.10)
project(LoadBalancer VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)

if (WIN32)
  set(OS_SERVER_FILE "src/platform/win32_tcp_server.c")
  set(OS_CLIENT_FILE "src/platform/win32_tcp_client.c")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(OS_SERVER_FILE "src/platform/linux_tcp_server.c")
  set(OS_CLIENT_FILE "src/platform/linux_tcp_client.c")
else()
  message(FATAL_ERROR "OS not suppor  # ${CMAKE_CURRENT_SOURCE_DIR}/src/round_robin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.h")
endif()

add_executable(balancer
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/http.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/http.c
  
  ${CMAKE_CURRENT_SOURCE_DIR}/src/round_robin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/round_robin.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.h
  ${OS_SERVER_FILE}
  ${OS_CLIENT_FILE}
)

# add_executable(balancer-tests
#   ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.c
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/http.h
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/http.c

#   ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.h
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.h
#   ${OS_SERVER_FILE}
#   ${OS_CLIENT_FILE}
# )
