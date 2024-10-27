CXX = g++
LIBS = -lnetcpp

all: tcp http

tcp: tcpclient.out tcpserver.out
http: httpclient.out httpserver.out

%.out: %.cpp
	${CXX} $< -o $@ ${LIBS}

clean:
	rm *.out