#!/bin/bash

echo "Starting Freelance Service..."
set -e

echo ">> Initial build..."
cmake -S . -B build
cmake --build build

./build/FreelanceService &
PID=$!

echo ">> Watching for source changes..."
while true; do
    inotifywait -e modify,create,delete -r \
        ./db \
        ./models \
        ./repositories \
        ./controllers \
        ./main.cpp \
        ./CMakeLists.txt

    echo ">> Change detected. Rebuilding..."
    
    if [ -n "$PID" ]; then
        kill $PID 2>/dev/null || true
    fi

    cmake --build build

    echo ">> Restarting..."
    ./build/FreelanceService &
    PID=$!
done
