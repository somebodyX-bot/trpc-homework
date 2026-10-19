ARG BUILD_BASE=ubuntu:22.04
FROM ${BUILD_BASE} AS builder
ARG BUILD_JOBS=4
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git python3 ca-certificates pkg-config \
    libcurl4-openssl-dev libssl-dev zlib1g-dev && rm -rf /var/lib/apt/lists/*
COPY vendor /tmp/vendor
COPY scripts/prepare_vendor.py /tmp/prepare_vendor.py
RUN python3 /tmp/prepare_vendor.py /tmp/vendor /opt/trpc \
    && cmake -S /opt/trpc -B /opt/trpc/build -C /opt/trpc/offline.cmake -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /opt/trpc/build --target trpc -j${BUILD_JOBS}
WORKDIR /src
COPY CMakeLists.txt ./
COPY proto ./proto
COPY src ./src
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j${BUILD_JOBS} \
    && mkdir -p /runtime-libs \
    && ldd build/rpc_server build/rpc_client | awk '/=> \// {print $3} /^\// {print $1}' | sort -u | xargs -r -I '{}' cp -L '{}' /runtime-libs/

FROM ubuntu:22.04 AS runtime
WORKDIR /app
COPY --from=builder /runtime-libs/ /usr/local/lib/
RUN ldconfig && groupadd -g 10001 rpc && useradd -u 10001 -g rpc -M rpc \
    && mkdir -p /app/logs && chown rpc:rpc /app/logs
COPY --from=builder /src/build/rpc_server /src/build/rpc_client ./
COPY conf ./conf
USER rpc
CMD ["./rpc_server", "--config=conf/server.yaml"]
