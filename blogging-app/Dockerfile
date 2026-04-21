FROM ubuntu:22.04

# Avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git \
    cmake \
    g++ \
    build-essential \
    libssl-dev \
    uuid-dev \
    zlib1g-dev \
    libbsoncxx-dev \
    libmongocxx-dev \
    jsoncpp \
    pkg-config \
    libjsoncpp-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install Drogon
RUN git clone https://github.com/drogonframework/drogon.git /tmp/drogon && \
    cd /tmp/drogon && \
    git submodule update --init && \
    mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install && \
    cd / && \
    rm -rf /tmp/drogon

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Build the application
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make -j$(nproc)

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    uuid-runtime \
    zlib1g \
    libbsoncxx1.3.0 \
    libmongocxx1.3.0 \
    libjsoncpp1 \
    && rm -rf /var/lib/apt/lists/*

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/api/stats || exit 1

# Run the application
CMD ["./build/blogging_app"]
