# --- Stage 1: Build the C++ executable (Fix for gcc:latest) ---
FROM debian:bookworm-slim # Or debian:bullseye-slim
# FIX: Add repository archive configuration for the gcc base image
RUN echo "deb http://archive.debian.org/debian buster main" > /etc/apt/sources.list && \
    echo "deb http://archive.debian.org/debian-security buster/updates main" >> /etc/apt/sources.list && \
    apt-get update

WORKDIR /app
COPY . .
RUN g++ -o yfapi yfapi.cpp -lcurl -std=c++17

# --- Stage 2: Create a minimal runtime image (Improved Base Image) ---
# FIX: Use a supported Debian image (Debian 12) for long-term stability
FROM debian:bookworm-slim

# Install necessary runtime dependencies (no archive fix needed for Bookworm)
RUN apt-get update && \
    apt-get install -y libcurl4 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/yfapi /app/yfapi
CMD ["./yfapi"]
