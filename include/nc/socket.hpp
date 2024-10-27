#pragma once
#ifdef _WIN32
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef long ssize_t;
  #define NC_OPT_TYPE(val) ((const char *)val)
#else
  #include <sys/socket.h>
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define INVALID_SOCKET ((SOCKET)-1)
  #define SOCKET_ERROR -1
  #define closesocket close
  typedef int SOCKET;
  #define NC_OPT_TYPE(val) val
#endif

namespace nc {

enum status {
  // fatal
  BIND_FAILED = -4,
  CONNECT_FAILED = -3,
  SOCKET_CREATION_FAILED = -2,
  OS_CTX_START_FAILED = -1,
  // ok
  good = 0,
  // warning
  ACCEPT_FAILED = 1,
  SOCKET_OPT_TIMEOUT_FAILED = 2,
  LISTEN_FAILED = 3,
  ERR_SETTING_REUSEADDR = 4
};

const char *errString(status errcode);

class SocketTCP {
public: // private (but private is bad for program development, why close your api off?)
  struct sockaddr_in m_sockaddr;
  socklen_t m_sockaddrlen; // set only on accept
  SOCKET m_sock;
public:
  status err;

  bool operator==(SOCKET s) { return this->m_sock == s; } // e.g if (sock == INVALID_SOCKET)
  bool operator!() { return this->m_sock == INVALID_SOCKET; }

  SocketTCP() = default;

  SocketTCP(const SocketTCP& socket) = delete; // can't have two exact same sockets
  SocketTCP(SocketTCP&& socket) : m_sockaddr(socket.m_sockaddr), m_sock(socket.m_sock) { socket.m_sock = INVALID_SOCKET; }

  ~SocketTCP() { this->__deconstruct(); }

  void __deconstruct(); // in case relying on deconstructor is unreliable

  // socket init && deinit
  status initIPv4(); // WSAStartup
  status openIPv4(unsigned int port, const char *ipaddr = "127.0.0.1"); // open connection to port at ip address
  status bindIPv4(unsigned int port); // bind socket to port
  status close();

  // socket interaction
  ssize_t write(const char *buf, size_t size);
  ssize_t read(char *buf, size_t size);
  ssize_t send(const char *buf, size_t size) { return this->write(buf, size); }
  ssize_t recv(char *buf, size_t size) { return this->read(buf, size); }

  status accept(nc::SocketTCP *client);

  // socket parameters
  status listen(int maxNumberConnections);
  status timeout(time_t sec, time_t us); // us is micro seconds
  status set_reuseaddr();
};

};

#if defined(NETC_IMPL) || defined(NETC_SOCKET_IMPL)
  // auxiliary
  const char *nc::errString(status errcode) {
    switch (errcode) {
      case nc::status::good: return "Success";
      // WARNING
      case nc::status::ACCEPT_FAILED: return "WARNING - Accept failed";
      case nc::status::SOCKET_OPT_TIMEOUT_FAILED: return "WARNING - Setting Socket Option Timeout Failed";
      case nc::status::LISTEN_FAILED: return "WARNING - Socket Couldn't Start Listening for max connections given";
      case nc::status::ERR_SETTING_REUSEADDR: return "WARNING - socket failed to set option reuse port address";
      // FATAL
      case nc::status::OS_CTX_START_FAILED: return "FATAL - OS couldn't create socket context (normally relating to windows api WSAStartup)";
      case nc::status::SOCKET_CREATION_FAILED: return "FATAL - socket function failed returning INVALID_SOCKET";
      case nc::status::CONNECT_FAILED: return "FATAL - socket couldn't connect to given ip address and port";
      case nc::status::BIND_FAILED: return "FATAL - socket couldn't bind to given port";
      default: return "Status Code is Invalid";
    };
  }

  // TCP Socket Implementation
  void nc::SocketTCP::__deconstruct() {
    #ifdef _WIN32
      WSACleanup();
    #endif
    this->close();
  }

  nc::status nc::SocketTCP::initIPv4() {
  #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      this->m_sock = INVALID_SOCKET;
      return nc::status::OS_CTX_START_FAILED;
    }
  #endif
    // AF_INET tells api to use ipv4
    // SOCK_STREAM tells api to use tcp
    this->m_sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (this->m_sock == INVALID_SOCKET) {
      return nc::status::SOCKET_CREATION_FAILED;
    }
    return nc::status::good;
  }
  nc::status nc::SocketTCP::openIPv4(unsigned int port, const char *ipaddr) {
    // config socket
    this->m_sockaddr.sin_family = AF_INET; // ipv4
    this->m_sockaddr.sin_port = htons(port);
    this->m_sockaddr.sin_addr.s_addr = inet_addr(ipaddr);
    
    // connect socket to port and ip addr
    if (::connect(this->m_sock, (struct sockaddr*)&this->m_sockaddr, sizeof(this->m_sockaddr)) == SOCKET_ERROR) {
      this->m_sock = INVALID_SOCKET;
      return nc::status::CONNECT_FAILED;
    }
    return nc::status::good;
  }
  nc::status nc::SocketTCP::bindIPv4(unsigned int port) {
    // config socket
    this->m_sockaddr.sin_family = AF_INET; // ipv4
    this->m_sockaddr.sin_addr.s_addr = INADDR_ANY; // accept any incomming connections (this should be changed by the user)
    this->m_sockaddr.sin_port = htons(port);

    // bind socket to port
    if (::bind(this->m_sock, (struct sockaddr *)&this->m_sockaddr, sizeof(this->m_sockaddr)) < 0) {
      this->m_sock = INVALID_SOCKET;
      return nc::status::BIND_FAILED;
    }
    return nc::status::good;
  }
  nc::status nc::SocketTCP::close() {
    if (this->m_sock == INVALID_SOCKET) return nc::status::good;
    ::closesocket(this->m_sock);
    return nc::status::good;
  }

  // socket interaction
  ssize_t nc::SocketTCP::write(const char *buf, size_t size) {
    return ::send(this->m_sock, buf, size, 0);
  }
  ssize_t nc::SocketTCP::read(char *buf, size_t size) {
    return ::recv(this->m_sock, buf, size, 0);
  }
  nc::status nc::SocketTCP::accept(nc::SocketTCP *client) {
    // accept client connection
    if (
      (client->m_sock = ::accept(this->m_sock, 
        (struct sockaddr *)&client->m_sockaddr, &client->m_sockaddrlen
      )) < 0) {
      return nc::status::ACCEPT_FAILED;
    }

    return nc::status::good;
  }

  // socket parameters
  nc::status nc::SocketTCP::timeout(time_t sec, time_t us) {
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = us;
    if (setsockopt(this->m_sock, SOL_SOCKET, SO_RCVTIMEO, NC_OPT_TYPE((&timeout)), sizeof(timeout)) == SOCKET_ERROR) {
      return nc::status::SOCKET_OPT_TIMEOUT_FAILED;
    }
    return nc::status::good;
  }
  nc::status nc::SocketTCP::set_reuseaddr() {
    int opt = 1;
    if (setsockopt(this->m_sock, SOL_SOCKET, SO_REUSEADDR, NC_OPT_TYPE((&opt)), sizeof(opt)) < 0) {
      this->m_sock = INVALID_SOCKET;
      return nc::status::ERR_SETTING_REUSEADDR;
    }
    return nc::status::good;
  }
  nc::status nc::SocketTCP::listen(int maxNumberConnections) {
    if (::listen(this->m_sock, maxNumberConnections) == SOCKET_ERROR) {
      return nc::status::LISTEN_FAILED;
    }
    return nc::status::good;
  }
#endif