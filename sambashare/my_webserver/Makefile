CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ./log/*.cpp ./pool/*.cpp ./heaptimer/*.cpp \
       ./httprequest/*.cpp  ./httpresponse/*.cpp ./webserver/*.cpp \
       ./buffer/*.cpp ./epoller/*.cpp ./main/*.cpp 

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
