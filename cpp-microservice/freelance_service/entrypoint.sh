#!/bin/bash


# build the project like this


# cd freelance_service
# docker build -t freelance-service .
# docker run -it --rm -v $(pwd):/app -p 9080:9080 freelance-service

echo "Starting Freelance Service..."
set -e

# First build
echo ">> Initial build..."
cmake -S . -B build
cmake --build build

# Run initial binary
./build/FreelanceService &
PID=$!

# Watch for changes
echo ">> Watching for source changes..."
while true; do
    # Watch all source files, headers, and build files
    inotifywait -e modify,create,delete -r \
        ./db \
        ./models \
        ./repositories \
        ./controllers \
        ./main.cpp \
        ./CMakeLists.txt

    echo ">> Change detected. Rebuilding..."
    
    # Kill the previous process if it exists
    if [ -n "$PID" ]; then
        kill $PID 2>/dev/null || true
    fi

    # Rebuild
    cmake --build build

    echo ">> Restarting..."
    ./build/FreelanceService &
    PID=$!
done