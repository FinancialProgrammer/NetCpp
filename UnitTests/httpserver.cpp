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

/*
* NotImplementedYet
*
*
*/

int main() {
  nc::SocketSSL::init();
  
  nc::SocketSSL sock;
  nc::status s;

  s = sock.initIPv4(); CheckError
  s = sock.bindIPv4(60000); CheckError
  
  s = sock.timeout(3,0); CheckError // timeout 1 second

  s = sock.listen(SIMCONNNUM); CheckError // 10 simulatneous connections
  nc::SocketSSL client;

  while (true) {
    s = sock.accept(&client); CheckErrorLoop
    
    char exampleMsg[] = 
      "1.0 400 Bad Request\n"
      "Content-Length: 54\n"
      "Content-Type: text/html; charset=UTF-8\n"
      "Date: Sun, 27 Oct 2024 14:01:10 GMT\n"
      "\n"
      "<html><title>Error 400 (Bad Request)!!1</title></html>"
    ;
    client.send(exampleMsg, sizeof(exampleMsg));
  }

  sock.close();

  nc::SocketSSL::clean();
}