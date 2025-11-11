# --- Stage 1: Build the C++ executable ---
FROM gcc:latest AS builder
WORKDIR /app
COPY . .
# Corrected: Executable name is 'yfapi'
RUN g++ -o yfapi yfapi.cpp -lcurl -std=c++17

# --- Stage 2: Create a minimal runtime image ---
FROM debian:buster-slim

# FIX: Add repository archive configuration to fix the 404/exit code 100 error.
# Debian Buster is EOL, so we must use the archive mirror.
RUN echo "deb http://archive.debian.org/debian buster main" > /etc/apt/sources.list && \
    echo "deb http://archive.debian.org/debian-security buster/updates main" >> /etc/apt/sources.list

# Install necessary runtime dependencies
RUN apt-get update
RUN apt-get install -y libcurl4
# 3. Clean up the cache to keep the image small
RUN rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# FIX: The binary copied must be 'yfapi', not 'your_server'.
COPY --from=builder /app/yfapi /app/yfapi

# FIX: The CMD must execute 'yfapi', not 'your_server'.
CMD ["./yfapi"]
