FROM ubuntu:22.04

# Install build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake git curl pkg-config \
    meson ninja-build \
    libsqlite3-dev libssl-dev libcurl4-openssl-dev \
    nlohmann-json3-dev \
    inotify-tools

# Clone and install Pistache (Meson based)
WORKDIR /opt
RUN git clone https://github.com/pistacheio/pistache.git && \
    cd pistache && \
    git checkout master && \
    meson setup build && \
    meson compile -C build && \
    meson install -C build

# Workdir for your app
WORKDIR /app

# Copy entrypoint that watches files
COPY entrypoint.sh .

# Run the watcher script
ENTRYPOINT ["./entrypoint.sh"]
