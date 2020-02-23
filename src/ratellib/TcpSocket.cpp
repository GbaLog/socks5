#include "TcpSocket.h"
#ifdef _WIN32
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <fcntl.h>
#include <unistd.h>
#endif
#include <stdexcept>
#include <cstring>

TcpSocket::TcpSocket()
{
  _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (_sock == INVALID_SOCKET)
    throw std::runtime_error("Can't create TCP socket");
}

TcpSocket::TcpSocket(SOCKET sock) :
  _sock(sock)
{}

TcpSocket::~TcpSocket()
{
  shutdown();
  close();
}

void TcpSocket::shutdown()
{
#ifdef _WIN32
  ::shutdown(_sock, SD_BOTH);
#else
  ::shutdown(_sock, SHUT_RDWR);
#endif
}

void TcpSocket::close()
{
#ifdef _WIN32
  ::closesocket(_sock);
#else
  ::close(_sock);
#endif
}

void TcpSocket::setNonBlock(bool block)
{
#ifdef _WIN32
  ioctlsocket(_sock, FIONBIO, (unsigned long *)&block);
#else
  int flags = fcntl(_sock, F_GETFL, 0);
  if (flags == -1) return;
  flags &= block ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
  fcntl(_sock, F_SETFL, flags);
#endif
}

bool TcpSocket::connect(const std::string & ip, uint16_t port)
{
  sockaddr_in sa;

  convertStringToHost(ip, port, sa);

  return ::connect(_sock, (sockaddr *)&sa, sizeof(sa)) == 0;
}

bool TcpSocket::bind(uint32_t ip, uint16_t port)
{
  sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = ip;
  sa.sin_port = port;
  return ::bind(_sock, (sockaddr *)&sa, sizeof(sa)) == 0;
}

bool TcpSocket::bind(const std::string & ip, uint16_t port)
{
  sockaddr_in sa;

  convertStringToHost(ip, port, sa);

  return ::bind(_sock, (sockaddr *)&sa, sizeof(sa)) == 0;
}

bool TcpSocket::listen(int backlog)
{
  return ::listen(_sock, backlog) == 0;
}

TcpSocket * TcpSocket::acceptSock(sockaddr_in & sa)
{
  memset(&sa, 0, sizeof(sa));
  socklen_t addrlen = sizeof(sa);
  SOCKET sock = ::accept(_sock, (struct sockaddr *)&sa, &addrlen);
  if (sock == INVALID_SOCKET)
    return nullptr;
  return new TcpSocket(sock);
}

int TcpSocket::recv(char * buf, int len)
{
  return ::recv(_sock, buf, len, 0);
}

int TcpSocket::send(const char * buf, int len)
{
  return ::send(_sock, buf, len, 0);
}

SOCKET TcpSocket::getFileDescr()
{
  return _sock;
}

void TcpSocket::convertStringToHost(const std::string & ip, uint16_t port, sockaddr_in & sa)
{
  sa.sin_family = AF_INET;
  sa.sin_port = port;
  inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);
}

