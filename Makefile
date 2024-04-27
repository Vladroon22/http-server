CXX = g++
CFLAGS = -Wall -Wextra 
TARGET = my_server

SRCS = main.cpp server.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

image: 
	sudo docker build . -t server
image-rm:
	sudo docker rmi server

docker:
	sudo docker run --name=http-server -p 8000:8000 -d server
docker-rm:
	sudo docker stop http-server
	sudo docker rm http-server
