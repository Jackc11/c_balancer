# c_balancer

<strong>A blazingly fast load balancer written in C without any heap allocation!!!</strong><br>
This load balancer is used for reverse proxy traffic from incoming connection to the one of the server that's were choosen by round robin algorithm<br>
<strong>This project is work in progress. Purpose of this is to learn optimization(a challenge to write C code without any heap allocation) and how a reverse proxy works</strong>

# How to build it
To build this balancer use this commands
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
# How to use this load balancer
<strong>First edit the config.h file in src directory then build it</strong><br>
To run enter this
```
./balancer
```

# Work in progress
It will support other algorithm for http traffic balancing and a working url parsing
