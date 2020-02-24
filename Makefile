all: server client

server: server.cpp
	g++ -pthread server.cpp -o server

.PHONY: client

client:
	echo "Please install Qt IDE and use it to build client.pro"
