// #define NETC_SOCKET_IMPL
#include <nc/socket.hpp>

#include <stdio.h>

#define CheckError \
  if (s != nc::status::good) {  \
    printf("%s %c", nc::errString(s), '\n'); \
    return 1;  \
  }
#define CheckErrorLoop \
  if (s != nc::status::good) {  \
    printf("%s %c", nc::errString(s), '\n'); \
    continue;  \
  }

#define SIMCONNNUM 10


int main() {
  nc::SocketTCP sock;
  nc::status s;

  s = sock.initIPv4(); CheckError
  s = sock.bindIPv4(60000); CheckError
  
  s = sock.timeout(3,0); CheckError // timeout 1 second

  s = sock.listen(SIMCONNNUM); CheckError // 10 simulatneous connections
  nc::SocketTCP client;

  while (true) {
    s = sock.accept(&client); CheckErrorLoop
    
    char exampleMsg[] = "FooBar";
    client.send(exampleMsg, sizeof(exampleMsg));
  }

  sock.close();
}