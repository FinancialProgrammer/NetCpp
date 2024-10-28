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
  FAILED_ENCRYPTED_HANDSHAKE = -6,
  FAILED_INIT_ENCRYP_CTX = -5,
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

/*
* Base TCP Socket with nothing special
*
*/
class SocketTCP {
public: // private (but private is bad for program development, why close your api off?)
  struct sockaddr_in m_sockaddr;
  socklen_t m_sockaddrlen = 0; // set only on accept
  SOCKET m_sock = INVALID_SOCKET;
public:
  status err;

  bool operator==(SOCKET s) { return this->m_sock == s; } // e.g if (sock == INVALID_SOCKET)
  bool operator!() { return this->m_sock == INVALID_SOCKET; }

  SocketTCP() = default;
  SocketTCP(SOCKET sock, struct sockaddr_in sockaddr, socklen_t sockaddrlen) 
    : m_sock(sock), m_sockaddr(sockaddr), m_sockaddrlen(sockaddrlen)
  {}

  SocketTCP(const SocketTCP& socket) = delete; // can't have two exact same sockets
  SocketTCP(SocketTCP&& socket) : m_sockaddr(socket.m_sockaddr), m_sock(socket.m_sock) { socket.m_sock = INVALID_SOCKET; }

  ~SocketTCP() { this->close(); this->__deconstruct(); }

  void __deconstruct(); // in case relying on deconstructor is unreliable

  // socket init && deinit
  status init(); // base init called by other inits (WSAStartup)
  status initIPv4(); // (easier encyption wrapper development)
  status openIPv4(unsigned int port, const char *ipaddr = "127.0.0.1"); // open connection to port at ip address
  status bindIPv4(unsigned int port); // bind socket to port
  status close();

  // socket interaction
  ssize_t write(const void *buf, size_t size);
  ssize_t read(void *buf, size_t size);
  ssize_t send(const void *buf, size_t size) { return this->write(buf, size); }
  ssize_t recv(void *buf, size_t size) { return this->read(buf, size); }

  status accept(nc::SocketTCP *client);

  // socket parameters
  status listen(int maxNumberConnections);
  status timeout(time_t sec, time_t us); // us is micro seconds
  status set_reuseaddr();
};

/*
* A TCP Socket Connection with SSL/TLS Encryption
* depends on OpenSSL
*/
class SocketSSL : public SocketTCP {
  using SocketTCP::openIPv4;
  using SocketTCP::close;
  using SocketTCP::write;
  using SocketTCP::read;
  using SocketTCP::send;
  using SocketTCP::recv;
public: // should be private
  void *m_ssl = nullptr; // typed in implementation (dodges openssl global include)
  void *m_ctx = nullptr;
public:
  SocketSSL() = default;
  SocketSSL(const SocketSSL&) = delete;
  SocketSSL(SocketSSL&& socket) : 
    SocketTCP(socket.m_sock, socket.m_sockaddr, socket.m_sockaddrlen),
    m_ssl(socket.m_ssl), m_ctx(socket.m_ctx)
  { socket.m_sock = INVALID_SOCKET; socket.m_ssl = nullptr; socket.m_ctx = nullptr; }
  ~SocketSSL() { this->close(); this->__deconstruct(); }

  static status init(); // initalize the global encryption context
  static status clean(); // deinitalize the global encyption context

  status openIPv4(unsigned int port, const char *ipaddr = "127.0.0.1", void *ctx = nullptr); // pass a SSL_CTX
  status close();
  
  ssize_t write(const void *buf, size_t size);
  ssize_t read(void *buf, size_t size);
  ssize_t send(const void *buf, size_t size) { return this->write(buf, size); }
  ssize_t recv(void *buf, size_t size) { return this->read(buf, size); }
};

};

#if defined(NETC_IMPL) || defined(NETC_SOCKET_IMPL)
  // auxiliary
  const char *nc::errString(nc::status errcode) {
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
      case nc::status::FAILED_INIT_ENCRYP_CTX: return "FATAL - failed to initalize encrypted context";
      case nc::status::FAILED_ENCRYPTED_HANDSHAKE: return "FATAL - failed to compelte the encrypted handshake with the third party";
      // DEFAULT
      default: return "Status Code is Invalid";
    };
  }

  // TCP Socket Implementation
  void nc::SocketTCP::__deconstruct() {
    #ifdef _WIN32
      WSACleanup();
    #endif
  }

  nc::status nc::SocketTCP::init() {
    #ifdef _WIN32
      WSADATA wsaData;
      if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        this->m_sock = INVALID_SOCKET;
        return nc::status::OS_CTX_START_FAILED;
      }
    #endif
  }

  nc::status nc::SocketTCP::initIPv4() {
    if (this->init() != nc::status::good) {
      return nc::status::OS_CTX_START_FAILED;
    }

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
  ssize_t nc::SocketTCP::write(const void *buf, size_t size) {
    return ::send(this->m_sock, buf, size, 0);
  }
  ssize_t nc::SocketTCP::read(void *buf, size_t size) {
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

  // TCP SSL Socket
  #include <openssl/ssl.h>
  #include <openssl/err.h>

  nc::status nc::SocketSSL::init() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();
    return nc::status::good;
  }
  nc::status nc::SocketSSL::clean() {
    EVP_cleanup();
    return nc::status::good;
  }

  nc::status nc::SocketSSL::openIPv4(unsigned int port, const char *ipaddr, void *ctx) {
    if (this->init() != nc::status::good) {
      return nc::status::OS_CTX_START_FAILED;
    }

    if (!this->m_ctx) {    
      const SSL_METHOD *method = TLS_client_method();  // Use TLS_client_method() for flexibility
      this->m_ctx = SSL_CTX_new(method);    
      if (!this->m_ctx) {
        return nc::status::FAILED_INIT_ENCRYP_CTX;    
      }  
    }
    if (!this->m_ssl) {    
      this->m_ssl = SSL_new((SSL_CTX*)this->m_ctx);      
      // set fd    
      this->m_sock = ::socket(AF_INET, SOCK_STREAM, 0);    
      // header    
      struct sockaddr_in server_addr;    
      server_addr.sin_family = AF_INET;    
      server_addr.sin_port = htons(port);    
      ::inet_pton(AF_INET, ipaddr, &server_addr.sin_addr); // ip_addr    
      // connect    
      if (connect(this->m_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {      
        ::close(this->m_sock);      
        return nc::status::SOCKET_CREATION_FAILED;
      }
      // bind ssl    
      SSL_set_fd((SSL*)this->m_ssl, this->m_sock);    
      // set the connection state
      SSL_set_connect_state((SSL*)this->m_ssl);
      // start ssl connection    
      if (SSL_connect((SSL*)this->m_ssl) <= 0) {         
        return nc::status::FAILED_ENCRYPTED_HANDSHAKE;
      }
    }
    return nc::status::good;
  }
  nc::status nc::SocketSSL::close() {
    if (this->m_ssl) {    
      SSL_shutdown((SSL*)this->m_ssl);    
      SSL_free((SSL*)this->m_ssl);    
      this->m_ssl = nullptr;  
    }  
    if (this->m_sock >= 0) {    
      ::closesocket(this->m_sock);    
      this->m_sock = INVALID_SOCKET;
    }  
    if (m_ctx) {    
      SSL_CTX_free((SSL_CTX*)this->m_ctx);    
      this->m_ctx = nullptr;  
    }
    return nc::status::good;
  }
  ssize_t nc::SocketSSL::write(const void *buf, size_t size) {
    return SSL_write((SSL*)this->m_ssl, buf, size);
  }
  ssize_t nc::SocketSSL::read(void *buf, size_t size) {
    return SSL_read((SSL*)this->m_ssl, buf, size);
  }
#endif