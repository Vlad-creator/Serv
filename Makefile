CXX = g++
CXXFLAGS = -lpthread

.PHONY: clean

all:
	@ $(CXX) servers/server.cpp -o server $(CXXFLAGS)
	@ $(CXX) servers/server_thread.cpp -o server_thread $(CXXFLAGS)
	@ $(CXX) clients/client.cpp -o client $(CXXFLAGS)

server_o:
	@ $(CXX) servers/server.cpp -o server $(CXXFLAGS)

server_thread_o:
	@ $(CXX) servers/server_thread.cpp -o server_thread $(CXXFLAGS)

client_o:
	@ $(CXX) clients/client.cpp -o client $(CXXFLAGS)

clean:
	rm -rf client server_thread server