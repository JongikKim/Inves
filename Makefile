##
CC = g++
CFLAGS = -O3 -Wall -std=c++11 -I./include

##
SRC = $(wildcard src/*.cpp)

OBJS = $(SRC:.cpp=.o)

TARGET = inves

##
.suffixes:
	.cpp

%.o:%.cpp
	$(CC) -c $< $(CFLAGS) -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f *~ */*~ $(OBJS) $(TARGET)

