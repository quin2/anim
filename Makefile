CXX=g++
SDL2CFLAGS=-I/usr/local/include/SDL2 -D_THREAD_SAFE

CXXFLAGS=-O2 -c --std=c++14 -Wall $(SDL2CFLAGS)
LDFLAGS=-L/usr/local/lib -lSDL2

exec: main.o
	$(CXX) $(LDFLAGS) -o exec main.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp