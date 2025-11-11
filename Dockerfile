# --- Stage 1: Build the C++ executable ---
# Use a standard C++ build environment base image
FROM gcc:latest AS builder

# Set the working directory
WORKDIR /app

# Copy your source code
COPY . .

# Compile your C++ code
# Adjust 'your_server.cpp' and 'your_server' to match your actual files
# The -o flag names the final executable
RUN g++ -o yfapi yfapi.cpp -lcurl -std=c++17

# --- Stage 2: Create a minimal runtime image ---
# Use a minimal base image to reduce size (better security and faster deploy)
FROM debian:buster-slim

# Install necessary runtime dependencies (e.g., if you use libcurl)
# The 'libcurl4' package is often required at runtime for C++ libcurl
RUN apt-get update && apt-get install -y libcurl4 && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the compiled binary from the builder stage
COPY --from=builder /app/your_server /app/your_server

# Define the default command to run the application
# The server should bind to 0.0.0.0 and listen on the port provided by Railway
# Railway automatically sets the PORT environment variable.
# IMPORTANT: Your C++ code MUST read and use the PORT environment variable.
CMD ["./yfapi"]
