// #define NETC_SOCKET_IMPL
#include <nc/socket.hpp>

#include <stdio.h>

int main() {
  nc::SocketSSL::init();
  
  nc::SocketSSL sock;

  sock.initIPv4();
  sock.openIPv4(80,"142.250.179.238"); // google.com (at the moment of writing update for your own test) (it is easy to get by 'ping google.com')

  if (sock == INVALID_SOCKET) { return 1; }
  if (!sock) { return 2; }

  sock.timeout(3,0); // timeout 1 second

  char sendMsg[] = 
    "GET https://google.com HTTP/1.1"
    "Host: https://google.com"
    "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; en-GB; rv:1.9.2.13) Gecko/20101203 Firefox/3.6.13"
    "Accept: text/html"
    "Accept-Language: en-gb,en;q=0.5"
    "Accept-Charset: ISO-8859-1,utf-8;"
  ;
  size_t bsent = sock.send(sendMsg, sizeof(sendMsg));

  char buffer[1024];
  size_t brecved = sock.recv(buffer,sizeof(buffer));

  fwrite(buffer,brecved,1,stdout);

  sock.close();

  nc::SocketSSL::clean();
}