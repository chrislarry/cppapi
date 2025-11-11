FROM gcc:12-bookworm AS builder
# Set the working directory
WORKDIR /app
# Copy your source code
COPY . .
COPY yahoo_crumb.txt /app/yahoo_crumb.txt 
COPY yahoo_cookies.txt /app/yahoo_cookies.txt 
# Compile your C++ code with the C++17 standard flag
RUN g++ -o yfapi yfapi.cpp -lcurl -std=c++17
# --- Stage 2: Create a minimal runtime image ---
# Use a modern, slim Debian image (Bookworm/Debian 12) to match the required GLIBC version (avoiding the GLIBC_2.38 error)
FROM debian:bookworm-slim
# Install necessary runtime dependencies
# apt-get update and install commands are simple and clean on modern images.
RUN apt-get update && \
    apt-get install -y libcurl4 && \
    rm -rf /var/lib/apt/lists/*
# Set the working directory
WORKDIR /app
# Copy the compiled binary from the builder stage
COPY --from=builder /app/yfapi /app/yfapi
# Define the default command to run the application
# Your C++ code must read the PORT environment variable.
CMD ["./getCookieAndCrubFiles.sh"]
CMD ["./yfapi"]
