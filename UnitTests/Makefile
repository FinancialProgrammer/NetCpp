CXX = g++
LIBS = -lnetcpp

tcp: tcpclient.out tcpserver.out

%.out: %.cpp
	${CXX} $< -o $@ ${LIBS}

clean:
	rm *.out