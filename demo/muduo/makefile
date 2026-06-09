CFLAG=-std=c++11 -I ../../build//release-install-cpp11/include/
LDFLAG=-L ../../build//release-install-cpp11/lib/  -lmuduo_net -lmuduo_base -pthread
all:server client
server:server.cpp
	g++  $(CFLAG) $^ -o $@ $(LDFLAG)
client:client.cpp
	g++  $(CFLAG) $^ -o $@ $(LDFLAG)