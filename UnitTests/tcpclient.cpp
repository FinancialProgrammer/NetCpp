// #define NETC_SOCKET_IMPL
#include <nc/socket.hpp>

#include <stdio.h>

int main() {
  nc::SocketTCP sock;

  sock.initIPv4();
  sock.openIPv4(80,"127.0.0.1");

  if (sock == INVALID_SOCKET) { return 1; }
  if (!sock) { return 2; }

  sock.timeout(3,0); // timeout 1 second

  char buffer[1024];
  size_t brecved = sock.recv(buffer,sizeof(buffer));

  fwrite(buffer,brecved,1,stdout);

  char sendMsg[] = "Sorry Google, Not Really";
  sock.send(sendMsg, sizeof(sendMsg));

  sock.close();
}