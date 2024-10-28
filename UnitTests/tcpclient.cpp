// #define NETC_SOCKET_IMPL
#include <nc/socket.hpp>

#include <stdio.h>

#define CheckError \
  if (s != nc::status::good) {  \
    printf("%s %c", nc::errString(s), '\n'); \
    return 1;  \
  }

int main() {
  nc::SocketTCP sock;

  nc::status s;

  s = sock.initIPv4(); CheckError
  s = sock.openIPv4(80,"142.250.179.238"); CheckError

  s = sock.timeout(3,0); CheckError

  char sendMsg[] = "Sorry Google, Not Really";
  ssize_t bsent = sock.send(sendMsg, sizeof(sendMsg));

  char buffer[1024];
  size_t brecved = sock.recv(buffer,sizeof(buffer)); CheckError

  fwrite(buffer,brecved,1,stdout);

  s = sock.close(); CheckError
}