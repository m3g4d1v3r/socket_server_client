include config.txt
SHELL := /bin/bash
binaries := server client
.DEFAULT_GOAL := all

# ssh -o SendEnv=SERVER_PORT,ARRAY_BYTES,PREFIX ${SERVER_PORT}

server: server.cpp 
	g++ -std=c++20 -DSERVER_PORT=${SERVER_PORT} -DARRAY_BYTES=${ARRAY_BYTES} server.cpp -o server

client: client.cpp 
	g++ -std=c++20 -DSERVER_IP=\"${SERVER_IP}\" -DSERVER_PORT=${SERVER_PORT} -DARRAY_BYTES=${ARRAY_BYTES} client.cpp -o client

clean:
	rm -f $(binaries)

all: clean $(binaries)

.PHONY: clean all default
