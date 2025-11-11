FROM gcc:12-bookworm AS builder # Use the modern GCC image matching bookworm
WORKDIR /app
COPY . .
RUN g++ -o yfapi yfapi.cpp -lcurl -std=c++17

# --- Stage 2: Create a minimal runtime image ---
FROM debian:bookworm-slim # Use modern runtime image (Debian 12)

# Install necessary runtime dependencies
# NOTE: No archive fix needed for bookworm. The simple command works.
RUN apt-get update && \
    apt-get install -y libcurl4 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=builder /app/yfapi /app/yfapi
CMD ["./yfapi"]
