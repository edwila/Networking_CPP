#!/bin/bash

read -p "Build destination: " BUILD_DEST

echo "< Building at $BUILD_DEST... >"

if ! cmake --build $BUILD_DEST; then
    echo "< Failed to build! >"
    wait 5
    exit 1
fi

echo "< Built! >"

read -p "Client count: " NUM_CLIENTS

if ! [[ "$NUM_CLIENTS" =~ ^[0-9]+$ ]]; then
    echo "Please enter a valid number for client count."
    exit 1
fi

echo "Starting server..."
#start ./build/bin/server.exe
mintty -t "Server" -s 100,20 -e ./$BUILD_DEST/bin/server.exe &
echo "> Server started. <"

echo ""

echo "Starting $NUM_CLIENTS client(s)..."

for (( i=1; i<=NUM_CLIENTS; i++ ))
do
    #start ./build/bin/client.exe
    mintty -t "Client $i" -s 50,15 -e ./$BUILD_DEST/bin/client.exe &
    echo "-> Client $i started."
done