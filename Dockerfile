FROM ubuntu:24.04 AS c_builder
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    # libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build_src
COPY . .
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release

FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    libcurl4 \
    curl \
    && rm -rf /var/lib/apt/lists/*

COPY test/dns.py ./dns.py
COPY test/server.py ./server.py

COPY --from=c_builder /build_src/build/balancer ./balancer
