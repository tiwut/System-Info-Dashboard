FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y \
    g++ make wget ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
RUN wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

COPY main.cpp .

RUN g++ -O3 -std=c++11 -pthread main.cpp -o sysinfo_server

FROM debian:bookworm-slim

WORKDIR /app

COPY --from=builder /app/sysinfo_server .
COPY index.html .

EXPOSE 8080

CMD ["./sysinfo_server"]
