// #define NETC_SOCKET_IMPL
#include <nc/socket.hpp>

#include <stdio.h>

#define CheckError \
  if (s != nc::status::good) {  \
    printf("%s %c", nc::errString(s), '\n'); \
    return 1;  \
  }

int main() {
  nc::SocketSSL::init();
  
  nc::SocketSSL sock;
  nc::status s;

  s = sock.initIPv4(); CheckError
  s = sock.openIPv4(443,"142.250.179.238"); CheckError // google.com (at the moment of writing update for your own test) (it is easy to get by 'ping google.com')

  s = sock.timeout(3,0); CheckError // timeout 1 second

  char sendMsg[] = 
    "GET https://google.com HTTP/1.1\n"
    "Host: https://google.com\n"
    "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; en-GB; rv:1.9.2.13) Gecko/20101203 Firefox/3.6.13\n"
    "Accept: text/html\n"
    "Accept-Language: en-gb,en;q=0.5\n"
    "Accept-Charset: ISO-8859-1,utf-8;\n"
    "\n\n"
  ;
  ssize_t bsent = sock.send(sendMsg, sizeof(sendMsg));
  printf("bytes sent: %zd\n", bsent);

  char buffer[1024];
  ssize_t brecved = sock.recv(buffer,sizeof(buffer));
  printf("bytes recieved: %zd\n", brecved);

  fwrite(buffer,brecved,1,stdout);

  s = sock.close(); CheckError

  nc::SocketSSL::clean();
}