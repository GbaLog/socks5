#include "TcpSocket.h"
#include <ws2tcpip.h>
#include <stdexcept>

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
#ifdef LINUX
  ::shutdown(_sock, SHUT_RDWR);
#else
  ::shutdown(_sock, SD_BOTH);
#endif
}

void TcpSocket::close()
{
#ifdef LINUX
  close(_sock);
#else
  closesocket(_sock);
#endif
}

void TcpSocket::setNonBlock(bool block)
{
#ifdef _solaris_
  int flags = 0
  flags = fcntl(_sd, F_GETFL, 0);
  flags &= ~O_NONBLOCK;
  fcntl(_sd, F_SETFL, flags);
#else
    ioctlsocket(_sock, FIONBIO, (unsigned long *)&block);
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

TcpSocket * TcpSocket::accept(sockaddr_in & sa)
{
  memset(&sa, 0, sizeof(sa));
  int addrlen = sizeof(sa);
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

