#ifndef TcpSocketH
#define TcpSocketH

#include "Common.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#endif

class TcpSocket
{
public:
  enum
  {
    somaxconn = SOMAXCONN
  };

public:
  TcpSocket();
  TcpSocket(SOCKET sock);
  ~TcpSocket();

  void shutdown();
  void close();
  void setNonBlock(bool block);
  bool connect(const std::string & ip, uint16_t port);
  bool bind(uint32_t ip, uint16_t port);
  bool bind(const std::string & ip, uint16_t port);
  bool listen(int count = somaxconn);
  TcpSocket * acceptSock(sockaddr_in & sa);
  bool acceptSock(TcpSocket & sock);

  int recv(char * buf, int len);
  int send(const char * buf, int len);

  SOCKET getFileDescr();

private:
  SOCKET _sock;

  void convertStringToHost(const std::string & ip, uint16_t port, sockaddr_in & sa);
};

#endif // TcpSocketH
