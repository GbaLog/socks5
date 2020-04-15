#include "TcpServer.h"
#include "ExceptionStream.h"
#include <cstring>

#ifdef _WIN32
#include <ws2tcpip.h>
#include <winsock2.h>
typedef int socklen_t;
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif
//-----------------------------------------------------------------------------
TcpServer::TcpServer(ITcpServerUser & user, const IpAddressAndPort & host) :
  Traceable("TcpSrv"),
  _user(user),
  _host(host),
  _run(false)
{}
//-----------------------------------------------------------------------------
TcpServer::~TcpServer()
{
  _run = false;
  for (auto & it  : _clientSocks)
    delete it;
}
//-----------------------------------------------------------------------------
void TcpServer::runInThisThread()
{
  _run = true;
  runServerInternalStatic(this);
}
//-----------------------------------------------------------------------------
void TcpServer::runServerInternalStatic(TcpServer * serv)
{
  serv->runServerInternal();
}
//-----------------------------------------------------------------------------
void TcpServer::runServerInternal()
{
  TRACE(DBG) << "Tcp server try to start on " << _host;
  if (_serverSock.bind(_host._ip, _host._port) == false)
  {
    TRACE(ERR) << "Can't bind server socket to " << _host;
    return;
  }

  if (_serverSock.listen() == false)
  {
    TRACE(ERR) << "Can't start listen server socket";
    return;
  }

  _serverSock.setNonBlock(false);

  fd_set readSocks;
  fd_set writeSocks;
  fd_set exceptSocks;

  timeval tv;
  tv.tv_sec = SelectTimeoutSec;
  tv.tv_usec = 0;

  while (_run)
  {
    //Clean all sets
    FD_ZERO(&readSocks);
    FD_ZERO(&writeSocks);
    FD_ZERO(&exceptSocks);

    //Set up them again
    FD_SET(_serverSock.getFileDescr(), &readSocks);

    for (auto it : _clientSocks)
    {
      FD_SET(it->getFileDescr(), &readSocks);
      FD_SET(it->getFileDescr(), &writeSocks);
    }

    //Select on it
    int ret = select(0, &readSocks, &writeSocks, &exceptSocks, &tv);

    //Timeout
    if (ret == 0)
    {
      TRACE(DBG) << "Timeout on select, continue";
      tv.tv_sec = SelectTimeoutSec; // 3 min
      tv.tv_usec = 0;
      continue;
    }

    //Select returns error
    if (ret == SOCKET_ERROR)
    {
      TRACE(ERR) << "Something went wrong: " << errno;
      return;
    }

    //Check new connections
    if (FD_ISSET(_serverSock.getFileDescr(), &readSocks))
    {
      struct sockaddr_in clientAddr;
      TcpSocket * client = _serverSock.acceptSock(clientAddr);

      if (client == nullptr)
      {
        TRACE(ERR) << "Can't accept client: " << errno;
        continue;
      }

      client->setNonBlock(true);
      _clientSocks.push_back(client);

      TRACE(DBG) << "Client accepted";

      IpAddressAndPort host;
      host._ip = clientAddr.sin_addr.s_addr;
      host._port = clientAddr.sin_port;
      _user.onNewConnection(host, client);
    }

    //Check clients
    handleClients(readSocks, writeSocks);
  }
}
//-----------------------------------------------------------------------------
void TcpServer::removeClient(TcpSocket * client, fd_set & socksSet)
{
  FD_CLR(client->getFileDescr(), &socksSet);
  TRACE(DBG) << "Client removed";

  _user.onCloseConnection(client);
}
//-----------------------------------------------------------------------------
void TcpServer::handleClients(fd_set & readSocks, fd_set & writeSocks)
{
  const size_t bufLen = 1024;
  char buf[bufLen] = {};

  for (auto it = _clientSocks.begin(); it != _clientSocks.end(); )
  {
    TcpSocket * client = *it;

    if (FD_ISSET(client->getFileDescr(), &readSocks))
    {
      if (handleClientRead(client, readSocks, buf, bufLen) == false)
      {
        it = _clientSocks.erase(it);
        continue;
      }
    }

    if (FD_ISSET(client->getFileDescr(), &writeSocks))
    {
      _user.onDataCanBeSend(client);
    }

    ++it;
  }
}
//-----------------------------------------------------------------------------
bool TcpServer::handleClientRead(TcpSocket * client, fd_set & readSocks, char * buf, int bufLen)
{
  int recvLen = client->recv(buf, bufLen);
  if (recvLen < 0)
  {
    if (errno != EWOULDBLOCK)
    {
      TRACE(DBG) << "Recv failed: received: " << recvLen << ", error: " << errno;
      removeClient(client, readSocks);
    }
    return false;
  }

  if (recvLen == 0)
  {
    TRACE(DBG) << "Connection closed";
    removeClient(client, readSocks);
    return false;
  }

  TRACE(DBG) << "Data received: size: " <<  recvLen << ", buf: " << buf;
  _user.onDataReceived(client, buf, recvLen);

  memset(buf, 0, bufLen);
  return true;
}
//-----------------------------------------------------------------------------

BaseSocket::BaseSocket(int type) :
  _type(type),
  _sock(INVALID_SOCKET)
{
  switch (_type)
  {
  case SocketType_Tcp:
  case SocketType_TcpServer:
  case SocketType_TcpConnected:
    _sock = ::socket(PF_INET, SOCK_STREAM, 0);
    break;
  case SocketType_Udp:
    _sock = ::socket(PF_INET, SOCK_DGRAM, 0);
    break;
  default:
    RATEL_THROW(std::runtime_error) << "Unknown type of socket";
  }
}

BaseSocket::BaseSocket(int type, SOCKET sock) :
  _type(type),
  _sock(sock)
{}

BaseSocket::~BaseSocket()
{
  shutdown();
  close();
}

void BaseSocket::shutdown()
{
  if (_type == SocketType_Tcp) return;
#ifdef _WIN32
  ::shutdown(_sock, SD_BOTH);
#else
  ::shutdown(_sock, SHUT_RDWR);
#endif
}

void BaseSocket::close()
{
#ifdef _WIN32
  ::closesocket(_sock);
#else
  ::close(_sock);
#endif
}

void BaseSocket::setNonBlock(bool block)
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

SOCKET BaseSocket::getSocketFD()
{
  return _sock;
}

bool BaseSocket::connect(uint32_t ip, uint16_t port)
{
  RATEL_THROW_IF(_type != SocketType_TcpConnected, std::runtime_error) << "Can't connect non-TCP socket";

  sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(ip);
  sa.sin_port = htons(port);
  return ::connect(_sock, (const sockaddr *)&sa, sizeof(sa));
}

bool BaseSocket::bind(uint32_t ip, uint16_t port)
{
  sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = ip;
  sa.sin_port = port;
  return ::bind(_sock, (sockaddr *)&sa, sizeof(sa)) == 0;
}

bool BaseSocket::listen(int backlog)
{
  RATEL_THROW_IF(_type != SocketType_TcpServer, std::runtime_error) << "Can't listen non-TCP socket";
  return ::listen(_sock, backlog) == 0;
}

int BaseSocket::recv(char * buf, int len, int flags)
{
  return ::recv(_sock, buf, len, flags);
}

int BaseSocket::send(const char * buf, int len, int flags)
{
  return ::send(_sock, buf, len, flags);
}

int BaseSocket::getReadSize()
{
#ifdef _WIN32
  unsigned long readSize = 0;
  ::ioctlsocket(_sock, FIONREAD, &readSize);
  return readSize;
#else
  int n;
  unsigned int m = sizeof(n);
  getsockopt(_sock, SOL_SOCKET, SO_RCVBUF, (void *)&n, &m);
  return n;
#endif
}

void BaseSocket::convertStringToHost(const std::string & ip, uint16_t port, sockaddr_in & sa)
{
  sa.sin_family = AF_INET;
  sa.sin_port = port;
  inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);
}

SOCKET IExSocket::getFileDescr()
{
  return _sock;
}

static int sockid = 0;

ExSocket::ExSocket(IExSocketHandler & handler, IExSocketUser & user, int type) :
  Traceable("ExSock", ++sockid),
  _handler(handler),
  _user(user),
  _type(type),
  _connected(false),
  _sock(type)
{
  setSocket(_sock.getSocketFD());
  _handler.onAddSocket(this);
}

ExSocket::ExSocket(IExSocketHandler & handler, IExSocketUser & user, int type, SOCKET sock) :
  Traceable("ExSock", ++sockid),
  _handler(handler),
  _user(user),
  _type(type),
  _connected(true),
  _sock(type, sock)
{
  setSocket(_sock.getSocketFD());
  _handler.onAddSocket(this);
}

ExSocket::~ExSocket()
{
  TRACE(DBG) << "Desctructor of ExSocket";
  _handler.onDelSocket(this);
  _sock.close();
}

void ExSocket::shutdown()
{
  _sock.shutdown();
}

int ExSocket::send(const char * buf, int len, int flags)
{
  return _sock.send(buf, len, flags);
}

void ExSocket::readMessages(DequeVecByte & buf)
{
  _msgRecvBuffer.swap(buf);
}

void ExSocket::bind(uint32_t ip, uint16_t port)
{
  TRACE(DBG) << "Try to bind socket to: " << IpAddressAndPort{ip, port};
  if (_sock.bind(ip, port) == false)
  {
    TRACE(ERR) << "Bind failed";
  }
}

void ExSocket::listen()
{
  TRACE(DBG) << "Start to listen";
  _sock.listen();
}

void ExSocket::onRead()
{
  TRACE(DBG) << "onRead";
  switch (_type)
  {
  case BaseSocket::SocketType_TcpServer:
    {
      sockaddr_in addr;
      socklen_t addrlen = sizeof(addr);
      SOCKET clientSock = ::accept(getSocketFD(), (sockaddr *)&addr, &addrlen);
      if (clientSock == INVALID_SOCKET)
      {
        TRACE_SINGLE(ERR, "ExSock") << "Invalid socket accepted";
        break;
      }
      TRACE_SINGLE(ERR, "ExSock") << "Valid sock accepted";
      ExSocket * sock = new ExSocket(_handler, _user, BaseSocket::SocketType_TcpConnected, clientSock);
      TRACE_SINGLE(ERR, "ExSock") << "New ExSocket created";
      _user.onNewConnection(sock, convertToAddr(addr));
      TRACE_SINGLE(ERR, "ExSock") << "OnNewConnection called";
      break;
    }
  case BaseSocket::SocketType_TcpConnected:
    {
      int readSize = _sock.getReadSize();
      TRACE_SINGLE(DBG, "ExSock") << "Ex read size: " << readSize;

      if (readSize > 0)
      {
        _msgRecvBuffer.push_back({});
        VecByte buf;
        buf.resize(readSize);

        _sock.recv((char *)buf.data(), buf.size());
        _msgRecvBuffer.back().swap(buf);
        _user.onRead();
      }
      else if (_connected && readSize == 0)
      {
        _sock.shutdown();
        TRACE_SINGLE(DBG, "ExSock") << "close conn";
        _connected = false;
        _user.onConnectionClosed();
        _handler.onDelSocket(this);
      }
      break;
    }
  default:
    break;
  }
}

void ExSocket::onWrite()
{
  TRACE(DBG) << "onWrite: " << _type << ", connected: " << _connected;
  switch (_type)
  {
  case BaseSocket::SocketType_TcpConnected:
    if (_connected == false)
    {
      _connected = true;
      _user.onConnected();
    }
    else
    {
      if (_msgSendBuffer.empty()) return;
      VecByte & msg = _msgSendBuffer.front();
      int sendSize = _sock.send((char *)msg.data(), msg.size());
      TRACE(DBG) << "msg size: " << msg.size() << ", snd size: " << sendSize;
      if (sendSize != msg.size())
        msg.erase(msg.begin(), msg.begin() + sendSize);
      else
        _msgSendBuffer.pop_front();
    }
    break;
  default:
    break;
  }
}

void ExSocket::onError()
{
  TRACE(DBG) << "onError";
  switch (_type)
  {
  case BaseSocket::SocketType_TcpConnected:
    _user.onConnectionFailed();
    break;
  default:
    break;
  }
}

SOCKET ExSocket::getSocketFD()
{
  return _sock.getSocketFD();
}

ExSocketHandler::ExSocketHandler() :
  Traceable("ExSockH"),
  _run(false)
{}

ExSocketHandler::~ExSocketHandler()
{
  _run = false;
}

void ExSocketHandler::runInThisThread()
{
  _run = true;
  runServerInternalStatic(this);
}

void ExSocketHandler::runServerInternalStatic(ExSocketHandler * serv)
{
  serv->runServerInternal();
}

void ExSocketHandler::runServerInternal()
{
  TRACE(DBG) << "Start socket handler";

  fd_set readSocks;
  fd_set writeSocks;
  fd_set errSocks;

  timeval tv;
  tv.tv_sec = SelectTimeoutSec;
  tv.tv_usec = SelectTimeoutMSec;

  //Event ev;

  while (_run)
  {
    /*
    if (_events.tryPop(ev))
    {
      handleEvent(ev);
    }
    */

    //Clean all sets
    FD_ZERO(&readSocks);
    FD_ZERO(&writeSocks);
    FD_ZERO(&errSocks);

    auto sockets = _flSockets.read();

    for (auto it : sockets)
    {
      FD_SET(it->getFileDescr(), &readSocks);
      FD_SET(it->getFileDescr(), &writeSocks);
      FD_SET(it->getFileDescr(), &errSocks);
    }

    //Select on it
    int ret = select(0, &readSocks, &writeSocks, &errSocks, &tv);

    //Timeout
    if (ret == 0)
    {
      tv.tv_sec = SelectTimeoutSec;
      tv.tv_usec = SelectTimeoutMSec;
      continue;
    }

    //Select returns error
    if (ret == SOCKET_ERROR)
    {
      TRACE(ERR) << "Something went wrong: " << errno;
      return;
    }

    for (auto & it : sockets)
    {
      if (FD_ISSET(it->getFileDescr(), &readSocks))
        it->onRead();
      if (FD_ISSET(it->getFileDescr(), &writeSocks))
        it->onWrite();
      if (FD_ISSET(it->getFileDescr(), &errSocks))
        it->onError();
    }
  }
  TRACE(DBG) << "Loop is ended, handler stopped";
}

void ExSocketHandler::handleEvent(const ExSocketHandler::Event & ev)
{
  TRACE(DBG) << "Event received: " << ev._type;
  switch (ev._type)
  {
  case Event::EventAddSocket:
    _flSockets.insert(ev._sock);
    //_sockets.insert(ev._sock);
    break;
  case Event::EventDelSocket:
    _flSockets.erase(ev._sock);
    //_sockets.erase(ev._sock);
    break;
  default:
    RATEL_THROW(std::invalid_argument) << "Unknown event received: " << ev._type;
    break;
  }
}

void ExSocketHandler::onAddSocket(IExSocket * sock)
{
  TRACE(DBG) << "Try to add sock";
  pushEvent(Event::EventAddSocket, sock);
}

void ExSocketHandler::onDelSocket(IExSocket * sock)
{
  TRACE(DBG) << "Try to del sock";
  pushEvent(Event::EventDelSocket, sock);
}

void ExSocketHandler::pushEvent(int type, IExSocket * sock)
{
  Event ev;
  ev._type = type;
  ev._sock = sock;
  handleEvent(ev);
}
