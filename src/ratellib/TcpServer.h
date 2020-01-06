#ifndef TcpServerH
#define TcpServerH
//-----------------------------------------------------------------------------
#include <atomic>
#include <deque>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <set>
#include "Tracer.h"
#include "InetUtils.h"
#include "Common.h"
#include "TcpSocket.h"
//-----------------------------------------------------------------------------
//TODO: This file needs refactoring or should be deleted instead
// or NQSignals module have to be written instead
// This code remains only because it could be reused in other place

struct ITcpServerUser
{
  virtual ~ITcpServerUser() {}

  virtual void onNewConnection(IpAddressAndPort & addr, TcpSocket * sock) = 0;
  virtual void onCloseConnection(TcpSocket * sock) = 0;
  virtual void onDataReceived(TcpSocket * sock, char * buf, int len) = 0;
  virtual void onDataCanBeSend(TcpSocket * sock) = 0;
};
//-----------------------------------------------------------------------------
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#endif

class BaseSocket
{
public:
  enum
  {
    SocketType_Tcp,
    SocketType_Udp,
    SocketType_TcpServer,
    SocketType_TcpConnected
  };

public:
  BaseSocket(int type);
  BaseSocket(int type, SOCKET sock);
  ~BaseSocket();

  void close();
  void setNonBlock(bool block);

  SOCKET getSocketFD();
  int getType() const;

  bool bind(uint32_t ip, uint16_t port);
  bool listen(int backlog = SOMAXCONN);
  int recv(char * buf, int len, int flags = 0);
  int send(const char * buf, int len, int flags = 0);
  void shutdown();
  bool connect(uint32_t ip, uint16_t port);
  int getReadSize();

private:
  int _type;
  SOCKET _sock;
  struct event_base *base;

  void convertStringToHost(const std::string & ip, uint16_t port, sockaddr_in & sa);
};

class IExSocket
{
public:
  IExSocket() : _sock(INVALID_SOCKET) {}
  virtual ~IExSocket() {}

  SOCKET getFileDescr();

  virtual void onRead() = 0;
  virtual void onWrite() = 0;
  virtual void onError() = 0;

protected:
  void setSocket(SOCKET sock) { TRACE_SINGLE(DBG, "IExSock") << "Set socket"; _sock = sock; }

private:
  SOCKET _sock;
};

struct IExSocketHandler
{
  virtual ~IExSocketHandler() {}

  virtual void onAddSocket(IExSocket * sock) = 0;
  virtual void onDelSocket(IExSocket * sock) = 0;
};

struct IExSocketUser
{
  virtual ~IExSocketUser() {}

  // Connected
  virtual void onRead() { TRACE_SINGLE(DBG, "IExSockUser") << "OnRead"; }
  virtual void onConnected() { TRACE_SINGLE(DBG, "IExSockUser") << "OnConnected"; }
  virtual void onConnectionClosed() { TRACE_SINGLE(DBG, "IExSockUser") << "OnConnectionClosed"; }
  virtual void onConnectionFailed() { TRACE_SINGLE(DBG, "IExSockUser") << "OnConnectionFailed"; }

  // Server
  virtual void onNewConnection(class ExSocket * sock, const IpAddressAndPort & addr) { TRACE_SINGLE(DBG, "IExSockUser") << "OnNewConnection"; }
};

template<class T>
class FlipBuffer
{
public:
  typedef std::set<T> BufferType;

  void insert(const T & val)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _write.insert(val);
  }

  void erase(const T & val)
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _write.erase(val);
  }

  BufferType & read()
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _read = _write;
    }
    return _read;
  }

private:
  std::mutex _mutex;
  BufferType _write;
  BufferType _read;
};

class ExSocket : public IExSocket, private Traceable
{
public:
  typedef std::deque<VecByte> DequeVecByte;

public:
  ExSocket(IExSocketHandler & handler, IExSocketUser & user, int type);
  ExSocket(IExSocketHandler & handler, IExSocketUser & user, int type, SOCKET sock);
  ~ExSocket();

  void shutdown();

  int send(const char * buf, int len, int flags = 0);
  void sendMessage(VecByte & buf);
  void readMessages(DequeVecByte & buf);

  void bind(uint32_t ip, uint16_t port);
  void listen();

private:
  IExSocketHandler & _handler;
  IExSocketUser & _user;
  int _type;
  bool _connected;
  BaseSocket _sock;

  DequeVecByte _msgRecvBuffer;
  DequeVecByte _msgSendBuffer;

  virtual void onRead() override;
  virtual void onWrite() override;
  virtual void onError() override;

  SOCKET getSocketFD();
};

template<class T>
class ConcurrentQueue
{
public:
  void push(const T & val)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push_back(val);
    lock.unlock();
    _cv.notify_one();
  }

  bool tryPop(T & val)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_queue.empty()) return false;
    val = _queue.front();
    _queue.pop_front();
    return true;
  }

  T pop()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_queue.empty())
    {
      _cv.wait(lock);
    }
    T val = _queue.front();
    _queue.pop_front();
    return val;
  }

private:
  std::condition_variable _cv;
  std::mutex _mutex;
  std::deque<T> _queue;
};

class ExSocketHandler : public IExSocketHandler, private Traceable
{
public:
  ExSocketHandler();
  ~ExSocketHandler();

  void runInThisThread();

  enum
  {
    SelectTimeoutSec = 0,
    SelectTimeoutMSec = 500
  };

private:
  struct Event
  {
    enum
    {
      EventAddSocket,
      EventDelSocket
    };
    int _type;
    IExSocket * _sock;
  };

  FlipBuffer<IExSocket *> _flSockets;
  std::atomic<bool> _run;

  static void runServerInternalStatic(ExSocketHandler * serv);

  void runServerInternal();
  void handleEvent(const Event & ev);

  //IExSocketHandler
  virtual void onAddSocket(IExSocket * sock) override;
  virtual void onDelSocket(IExSocket * sock) override;

  void pushEvent(int type, IExSocket * sock);
};

//-----------------------------------------------------------------------------
class TcpServer : private Traceable
{
public:
  TcpServer(ITcpServerUser & user, const IpAddressAndPort & host);
  ~TcpServer();

  void runInThisThread();

  enum
  {
    SelectTimeoutSec = 3 * 60 // 3 min
  };

private:
  ITcpServerUser & _user;
  IpAddressAndPort _host;
  TcpSocket _serverSock;
  std::vector<TcpSocket *> _clientSocks;
  std::atomic<bool> _run;

  static void runServerInternalStatic(TcpServer * serv);

  void runServerInternal();
  void removeClient(TcpSocket * client, fd_set & socksSet);
  void handleClients(fd_set & readSocks, fd_set & writeSocks);
  bool handleClientRead(TcpSocket * client, fd_set & readSocks, char * buf, int bufLen);
};
//-----------------------------------------------------------------------------
#endif // TcpServerH
//-----------------------------------------------------------------------------
