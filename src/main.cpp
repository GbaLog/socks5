#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cassert>
#include <map>
#include <sstream>
#include <algorithm>
#include <atomic>
#include "TcpSocket.h"
#include "TcpServer.h"
#include "Tracer.h"
#include "Common.h"

struct Client
{
  enum
  {
    AUTH_NOAUTH,
    AUTH_LOGINPASS,
  };
  int _auth;
  uint32_t _dstIP;
  uint16_t _port;
  sockaddr _clientAddr;
  SOCKET _remoteSock;
};

typedef std::map<uint32_t, Client> MapIPToClient;

class Socks5ConnMng : public ITcpServerUser, private Traceable
{
public:
  Socks5ConnMng() :
    Traceable("S5ConnMng")
  {}

private:
  struct ClientData
  {
    IpAddressAndPort _addr;
    TcpSocket * _sock;
  };

  std::map<TcpSocket *, ClientData> _clients;

  virtual void onNewConnection(IpAddressAndPort & addr, TcpSocket * sock) override
  {
    TRACE(DBG) << "New connection from: " << addr;
    ClientData data;
    data._addr = std::move(addr);
    data._sock = sock;
    _clients.emplace(sock, data);
  }

  virtual void onCloseConnection(TcpSocket * sock) override
  {
    auto it = _clients.find(sock);
    if (it == _clients.end())
    {
      TRACE(WRN) << "Connection cannot be closed, because client is not found";
      return;
    }
    TRACE(DBG) << "Connection closed";
    delete it->first;
    _clients.erase(it);
  }

  virtual void onDataReceived(TcpSocket * sock, char * buf, int len) override
  {
    auto it = _clients.find(sock);
    if (it == _clients.end())
    {
      TRACE(WRN) << "Data can't be read, because clients is not found";
      return;
    }


    std::string hexData;
    hexData.resize(len * 3);
    for (int i = 0; i < len; ++i)
    {
      sprintf(&hexData[0] + (i * 3), "%02X ", buf[i]);
    }

    TRACE(DBG) << "Data from client: " << it->second._addr << ", buf: " << hexData;

    //sock->send(tmpBuf.data(), tmpBuf.size());
  }

  virtual void onDataCanBeSend(TcpSocket * sock) override
  {
    //TRACE(DBG) << "Data can be send";
  }
};

class TestConnection : public IExSocketUser, private Traceable
{
public:
  TestConnection(IExSocketHandler & handler) :
    Traceable("Test"),
    _srvSock(handler, *this, BaseSocket::SocketType_TcpServer),
    _client(nullptr)
  {
    _srvSock.bind(0, htons(35555));
    _srvSock.listen();
  }

private:
  ExSocket _srvSock;
  ExSocket * _client;

  virtual void onNewConnection(ExSocket * sock, const IpAddressAndPort & addr) override
  {
    TRACE(DBG) << "Test connection from: " << addr;
    sock->send("ololo\n", strlen("ololo\n"));
    TRACE(DBG) << "Client is: " << _client;
    if (_client)
    {
      delete _client;
    }
    _client = sock;
    TRACE(DBG) << "Client is: " << _client;
  }

  virtual void onRead() override
  {
    if (_client == nullptr)
    {
      TRACE(ERR) << "client is null";
      return;
    }

    ExSocket::DequeVecByte buf;
    _client->readMessages(buf);
    if (buf.empty())
    {
      TRACE(DBG) << "Buf is empty";
      return;
    }

    TRACE(DBG) << "Buf has size: " << buf.size();
    for (auto & it : buf)
    {
      std::string data((char *)it.data(), it.size());
      TRACE(DBG) << "Incoming msg: " << data;
    }

    delete _client;
    _client = nullptr;
  }
};

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

extern "C"
void eventLog(int severity, const char * msg)
{
  int lvl = ERR;
  switch (severity)
  {
  case EVENT_LOG_DEBUG: lvl = DBG; break;
  case EVENT_LOG_MSG:   lvl = INF; break;
  case EVENT_LOG_WARN:  lvl = WRN; break;
  case EVENT_LOG_ERR:   lvl = ERR; break;
  default:              lvl = ERR; break;
  }
  //TRACE_SINGLE(lvl, "EvLog") << msg;
}

struct Socks5Params
{
  bool _active = false;
  evutil_socket_t _fd = evutil_socket_t{};
  sockaddr _addr = {};

  enum
  {
    Unauthorized,
    Authorization,
    WaitForConnectRequest,
    Connected
  };
  int _state = Unauthorized;
  bufferevent * _clientBev = nullptr;
  bufferevent * _hostBev = nullptr;
  VecByte _outBuf;
};

typedef std::map<evutil_socket_t, Socks5Params> MapSocks5Params;
#include <iomanip>
std::ostream & operator << (std::ostream & strm, const VecByte & v)
{
  auto flags = strm.flags();
  strm << "Size: " << v.size() << ", data: ";
  strm << std::hex;
  for (const auto it : v)
    strm << std::setw(2) << std::setfill('0') << (int)it << " ";
  strm << std::setfill(' ');
  strm.flags(flags);
  return strm;
}

extern "C"
void onClientRead(bufferevent * bev, void * arg)
{
  TRACE_SINGLE(DBG, "EvListCli") << "Client read buffer";
  Socks5Params * user = (Socks5Params *)arg;
  evbuffer * input = bufferevent_get_input(bev);
  evbuffer * srvOutput = bufferevent_get_output(user->_clientBev);
  evbuffer_add_buffer(srvOutput, input);
}

extern "C"
void onClientEvent(bufferevent * bev, short events, void * arg)
{
  Socks5Params * user = (Socks5Params *)arg;

  if (user->_active == false)
  {
    TRACE_SINGLE(DBG, "EvListCli") << "Server is not active, close";
    bufferevent_free(bev);
  }

  if (events & BEV_EVENT_EOF)
  {
    TRACE_SINGLE(INF, "EvListCli") << "Buffer event got EOF, close";
    user->_active = false;
    bufferevent_free(bev);
    bufferevent_free(user->_clientBev);
  }

  if (events & BEV_EVENT_ERROR)
  {
    user->_active = false;
    auto fd = bufferevent_getfd(bev);
    auto err = evutil_socket_geterror(fd);
    TRACE_SINGLE(ERR, "EvListCli") << "Buffer event got error: " << err << ", text: '" << evutil_socket_error_to_string(err) << "', close";
    byte resp[] = { 0x05, 0x04 };
    auto output = bufferevent_get_output(user->_clientBev);
    evbuffer_add(output, (void *)resp, sizeof(resp));
  }

  if (events & BEV_EVENT_CONNECTED)
  {
    TRACE_SINGLE(INF, "EvListCli") << "Client connected";
    bufferevent_setcb(bev, onClientRead, NULL, onClientEvent, (void *)user);

    if (user->_outBuf.empty() == false)
    {
      evbuffer * output = bufferevent_get_output(user->_clientBev);
      if (evbuffer_add(output, (void *)user->_outBuf.data(), user->_outBuf.size()) != 0)
      {
        TRACE_SINGLE(ERR, "EvListCli") << "Error to add buffer: " << user->_outBuf;
      }

      TRACE_SINGLE(DBG, "EvListCli") << "Data added to server buffer: " << user->_outBuf;
    }
    bufferevent_enable(bev, EV_READ | EV_WRITE);
  }
}

extern "C"
void onEchoRead(bufferevent * bev, void * arg)
{
  Socks5Params * user = (Socks5Params *)arg;
  if (!user->_active)
  {
    TRACE_SINGLE(ERR, "EvList") << "Bufferevent is not active";
    bufferevent_free(bev);
  }

  /* This callback is invoked when there is data to read on bev. */
  evbuffer * input = bufferevent_get_input(bev);
  evbuffer * output = bufferevent_get_output(bev);

  auto bufSize = evbuffer_get_length(input);
  VecByte buf;
  buf.resize(bufSize);
  auto copied = evbuffer_remove(input, (void *)buf.data(), buf.size());

  if (copied != bufSize)
  {
    TRACE_SINGLE(ERR, "EvList") << "Copied size: " << copied << " is not equal with buffer length: " << bufSize;
    bufferevent_free(bev);
    return;
  }

  TRACE_SINGLE(DBG, "EvList") << "Got on read";
  TRACE_SINGLE(DBG, "EvList") << "Received data: " << buf;

  const std::string login = "asd";
  const std::string pass = "asd";

  VecByte outBuf;
  switch (user->_state)
  {
  case Socks5Params::Unauthorized:
    outBuf.push_back(0x05);
    outBuf.push_back(0x02);
    user->_state = Socks5Params::Authorization;
    break;
  case Socks5Params::Authorization:
    outBuf.push_back(0x05);
    outBuf.push_back(0x00);
    user->_state = Socks5Params::WaitForConnectRequest;
    break;
  case Socks5Params::WaitForConnectRequest:
  {
    outBuf.push_back(0x05);
    if (buf.size() < 4)
    {
      outBuf.push_back(0x07); //protocol error
      break;
    }

    if (buf[1] != 0x01) //TCP stream
    {
      outBuf.push_back(0x07); //unsupported
      break;
    }

    if (buf[3] != 0x01) //IPv4
    {
      outBuf.push_back(0x07); //unsupported
      break;
    }

    if (buf.size() < 10)
    {
      outBuf.push_back(0x07); //protocol error
      break;
    }

    event_base * base = bufferevent_get_base(bev);
    uint32_t ip = *(uint32_t *)&buf[4];
    uint16_t port = *(uint16_t *)&buf[8];
    outBuf.push_back(0x00);
    outBuf.push_back(0x00);
    outBuf.push_back(0x01);
    //addr
    outBuf.push_back(0x00);
    outBuf.push_back(0x00);
    outBuf.push_back(0x00);
    outBuf.push_back(0x00);
    //port
    outBuf.push_back(0x00);
    outBuf.push_back(0x00);
    evutil_socket_t new_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    evutil_make_socket_nonblocking(new_fd);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    addr.sin_port = port;

    TRACE_SINGLE(DBG, "EvList") << "Try to connect to ip: " << ip;

    bufferevent * newBev = bufferevent_socket_new(base, new_fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(newBev, NULL, NULL, onClientEvent, (void *)user);
    bufferevent_socket_connect(newBev, (sockaddr *)&addr, sizeof(addr));
    user->_hostBev = newBev;
    user->_outBuf.swap(outBuf);
    outBuf.clear();

    user->_state = Socks5Params::Connected;
    break;
  }
  case Socks5Params::Connected:
  {
    TRACE_SINGLE(DBG, "EvList") << "Connected, try to add to output client buffer";
    assert(user && user->_hostBev);
    auto cliOutput = bufferevent_get_output(user->_hostBev);
    if (evbuffer_add(cliOutput, (void *)buf.data(), buf.size()) != 0)
    {
      TRACE_SINGLE(ERR, "EvList") << "Fail to add buffer to client";
    }
    break;
  }
  default:
    TRACE_SINGLE(DBG, "EvList") << "Cannot recognize state\n";
    break;
  };

  if (outBuf.empty() == false)
  {
    TRACE_SINGLE(DBG, "EvList") << "Add buffer to write: " << outBuf;
    evbuffer_add(output, (void *)outBuf.data(), outBuf.size());
  }
}

extern "C"
void onEchoWrite(bufferevent * bev, void * arg)
{
  TRACE_SINGLE(DBG, "EvList") << "Got on write";
}

extern "C"
void onEchoEvent(bufferevent * bev, short events, void * arg)
{
  Socks5Params * user = (Socks5Params *)arg;

  if (events & BEV_EVENT_EOF)
  {
    TRACE_SINGLE(INF, "EvList") << "Buffer event got EOF, close";
    user->_active = false;
    bufferevent_free(bev);
    bufferevent_free(user->_hostBev);
  }

  if (events & BEV_EVENT_ERROR)
  {
    TRACE_SINGLE(INF, "EvList") << "Buffer event got error, close";
    user->_active = false;
    bufferevent_free(bev);
  }
}

extern "C"
void onAcceptConnection(evconnlistener * listener, evutil_socket_t fd, sockaddr * addr, int socklen, void * arg)
{
  TRACE_SINGLE(DBG, "EvList") << "On accept connection";
  event_base * base = evconnlistener_get_base(listener);

  MapSocks5Params * params = (MapSocks5Params *)arg;
  Socks5Params & user = (*params)[fd];
  user._active = true;
  user._state = Socks5Params::Unauthorized;
  user._fd = fd;
  user._addr = *addr;

  bufferevent * bufferEvent = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bufferEvent, onEchoRead, onEchoWrite, onEchoEvent, (void *)&user);
  bufferevent_enable(bufferEvent, EV_READ | EV_WRITE);
  user._clientBev = bufferEvent;
}

extern "C"
void onAcceptError(evconnlistener * listener, void * arg)
{
  event_base * base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  TRACE_SINGLE(ERR, "EvList") << "Got an error: " << evutil_socket_error_to_string(err) << " on listener";

  event_base_loopexit(base, NULL);
}

#include "NQCoreApplication.h"
#include "NQObject.h"

class A : public NQObject
{
public:
  A()
  {

  }

  DECL_SIGNAL(signal, int)
  DECL_SIGNAL(signal2)

  void foo(int n)
  {
    EMIT_ALL(signal, n);
    EMIT_ALL(signal2);
  }
};

#define qDebug() Traceable("Test").makeTrace("DBG")

class Test : public NQObject
{
public:
  Test()
  {
    connect(this, &Test::f, this, &Test::b1);
    EMIT_ALL(f);
    EMIT_ALL(f);
  }

  DECL_SIGNAL(f)

  void b1()
  {
    qDebug() << __PRETTY_FUNCTION__ << " called\n";
    connect(this, &Test::f, this, &Test::b2);
  }

  void b2() { qDebug() << __PRETTY_FUNCTION__ << " called\n"; }
};

int main(int argc, char * argv[])
{
  {
    NQCoreApplication app;

    Test a;

    return app.run();
  }

  if (argc == 2 || strcmp(argv[1], "--help") == 0)
  {
    TRACE_SINGLE(INF, "main") << "Usage: " << argv[0] << " [host] [port]";
    return 0;
  }

  uint32_t hostIP = 0;
  uint16_t hostPort = htons(35555);
  
  if (argc >= 2)
  {
    auto tmpAddr = inet_addr(argv[1]);
    hostIP = tmpAddr;
  }
  if (argc >= 3)
  {
    hostPort = htons(std::stoi(argv[2]));
  }

  WSADATA wsaData;
  assert(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);

  //Socks5ConnMng connMng;
  //TcpServer srv(connMng, IpAddressAndPort{hostIP, hostPort});
  //srv.runInThisThread();

  event_set_log_callback(eventLog);
  event_enable_debug_logging(EVENT_DBG_ALL);

  TRACE_SINGLE(DBG, "EvLoop") << "Available methods are:";
  auto ** methods = event_get_supported_methods();
  for (int i = 0; methods[i] != nullptr; ++i)
  {
    TRACE_SINGLE(DBG, "EvLoop") << "Method: " << methods[i];
  }

  event_base * base = event_base_new();
  if (!base)
  {
    TRACE_SINGLE(DBG, "EvLoop") << "Can't create event loop";
    return 1;
  }
  TRACE_SINGLE(DBG, "EvLoop") << "Event loop is using: " << event_base_get_method(base) << " method";

  MapSocks5Params params;

  sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = hostPort;
  saddr.sin_addr.s_addr = hostIP;
  evconnlistener * listener = evconnlistener_new_bind(base, onAcceptConnection, (void *)&params,
                                                      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                                      (sockaddr *)&saddr, sizeof(saddr));
  if (!listener)
  {
    TRACE_SINGLE(DBG, "EvList") << "Can't create listener";
    event_base_free(base);
    return 1;
  }

  event_base_dispatch(base);

  event_base_free(base);

  libevent_global_shutdown();
#if 0
  ExSocketHandler handler;
  TestConnection conn(handler);
  handler.runInThisThread();
#endif

#if 0
  //main socket
  TcpSocket sock;

  in_addr hostAddr;
  hostAddr.s_addr = hostIP;
  TRACE_SINGLE(DBG, "main") << "Start bind to: ip: " << inet_ntoa(hostAddr) << ", port: " << htons(hostPort);

  if (sock.bind(hostIP, hostPort) == false)
  {
    TRACE_SINGLE(ERR, "main") << "Can't bind socket";
    return 1;
  }

  if (sock.listen() == false)
  {
    TRACE_SINGLE(ERR, "main") << "Can't listen";
    return 1;
  }

  sock.setNonBlock(true);

  timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  fd_set readSocks;
  fd_set writeSocks;
  fd_set exceptSocks;
  FD_ZERO(&readSocks);
  FD_ZERO(&writeSocks);
  FD_ZERO(&exceptSocks);

  FD_SET(sock.getFileDescr(), &readSocks);

  std::vector<TcpSocket *> clients;

  const size_t buflen = 1024;
  char buf[buflen] = {};
  memset(buf, 0, buflen);

  while (true)
  {
    FD_ZERO(&readSocks);

    FD_SET(sock.getFileDescr(), &readSocks);

    for (auto it : clients)
    {
      FD_SET(it->getFileDescr(), &readSocks);
    }

    TRACE_SINGLE(DBG, "main") << "Select " << readSocks.fd_count << " socks";
    int ret = select(1, &readSocks, &writeSocks, nullptr, nullptr);

    if (ret == SOCKET_ERROR)
    {
      TRACE_SINGLE(ERR, "main") << "Select failed: " << WSAGetLastError();
      break;
    }

    if (FD_ISSET(sock.getFileDescr(), &readSocks))
    {
      struct sockaddr_in clientAddr;
      TcpSocket * client = sock.accept(clientAddr);

      if (client == nullptr)
      {
        TRACE_SINGLE(ERR, "main") << "Can't accept client: " << WSAGetLastError();
        continue;
      }

      client->setNonBlock(true);

      FD_SET(client->getFileDescr(), &readSocks);
      clients.push_back(client);

      TRACE_SINGLE(DBG, "main") << "Client accepted";
      continue;
    }

    for (auto it = clients.begin(); it != clients.end(); )
    {
      TcpSocket * client = *it;

      auto removeClient = [&] ()
      {
        delete client;
        FD_CLR(client->getFileDescr(), &readSocks);
        it = clients.erase(it);
        TRACE_SINGLE(DBG, "main") << "Clint removed, count: " << readSocks.fd_count;
      };

      if (FD_ISSET(client->getFileDescr(), &readSocks))
      {
        int rcvd = client->recv(buf, buflen);
        if (rcvd < 0)
        {
          if (WSAGetLastError() != WSAEWOULDBLOCK && errno != EWOULDBLOCK)
          {
            TRACE_SINGLE(DBG, "main") << "Recv failed: received: " << rcvd << ", error: " << WSAGetLastError();
            removeClient();
          }
          continue;
        }

        if (rcvd == 0)
        {
          TRACE_SINGLE(DBG, "main") << "Connection closed";
          removeClient();
          continue;
        }

        TRACE_SINGLE(DBG, "main") << "Data received: size: " <<  rcvd << ", buf: " << buf;
        memset(buf, 0, buflen);
      }

      ++it;
    }
  }
#endif

#if 0
  struct sockaddr_in clientAddr;
  TcpSocket * client = sock.accept(clientAddr);
  if (client == nullptr)
  {
    std::cerr << "Can't accept: " << WSAGetLastError() << "\n";
    return 1;
  }
  
  std::cout << "Connected from: ip: " << inet_ntoa(clientAddr.sin_addr) << ", port: " << htons(clientAddr.sin_port) << "\n";
  
  char buf[1024];
  int len = sizeof(buf);
  
  int recved = client->recv(buf, len);
  
  if (recved > 0)
  {
    Client cli;
    cli._auth = Client::AUTH_LOGINPASS;
    char buf2[len * 2] = {};
    for (int i = 0; i < recved; ++i)
      sprintf(buf2 + i * 2, "%02X", buf[i]);
    std::cout << "Received buf: " << buf2 << "\n";
    char ans[2];
    ans[0] = 0x05;
    ans[1] = 0x02;
    client->send(ans, sizeof(ans));
    memset(buf, 0, sizeof(buf));
    
    recved = client->recv(buf, len);
    memset(buf2, 0, sizeof(buf2));
    
    if (recved > 0)
    {
      for (int i = 0; i < recved; ++i)
        sprintf(buf2 + i * 2, "%02X", buf[i]);
      std::cout << "Next received buf: " << buf2 << "\n";
      ans[0] = 0x01;
      ans[1] = 0x00;
      client->send(ans, sizeof(ans));
      
      memset(buf, 0, sizeof(buf));
    
      recved = client->recv(buf, len);
      memset(buf2, 0, sizeof(buf2));
      
      if (recved > 0)
      {
        for (int i = 0; i < recved; ++i)
          sprintf(buf2 + i * 2, "%02X", buf[i]);
        std::cout << "Connect received buf: " << buf2 << "\n";
        
        struct in_addr targetAddr;
        memset(&targetAddr, 0, sizeof(targetAddr));
        targetAddr.s_addr = *(uint32_t *)(buf + 4);
        
        cli._dstIP = targetAddr.s_addr;
        cli._port = *(uint16_t *)(buf + 8);
        
        std::cout << "Target ip: " << inet_ntoa(targetAddr) << ", port: " << ntohs(cli._port) << "\n";
        
        cli._remoteSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (cli._remoteSock == INVALID_SOCKET)
        {
          std::cerr << "Can't create socket\n";
          return 1;
        }
        
        struct sockaddr_in remAddr;
        remAddr.sin_family = PF_INET;
        remAddr.sin_addr.s_addr = cli._dstIP;
        remAddr.sin_port = cli._port;
        
        char ans[10];
        ans[0] = 0x05;
        ans[1] = 0x00;
        ans[2] = 0x00;
        ans[3] = 0x01;
        *(uint32_t *)(ans + 4) = 0x00;
        ans[8] = 0x00;
        ans[9] = 0x00;
        
        int res = connect(cli._remoteSock, (struct sockaddr *)&remAddr, sizeof(remAddr));
        if (res == INVALID_SOCKET)
        {
          std::cerr << "Can't connect to remote server: error: " << WSAGetLastError() << ", res: " << res << "\n";
          ans[1] = 0x04;
          closesocket(cli._remoteSock);
        }
        
        client->send(ans, sizeof(ans));
        
        memset(buf, 0, sizeof(buf));
    
        recved = client->recv(buf, len);
        std::cout << "Recved: " << recved << "\n";
        
        while (recved > 0)
        {
          memset(buf2, 0, sizeof(buf2));
          for (int i = 0; i < recved; ++i)
            sprintf(buf2 + i * 2, "%02X", buf[i]);
          std::cout << "Receive msg buf: " << buf2 << "\n";
          
          memset(buf, 0, sizeof(buf));
          recved = client->recv(buf, len);
        }
        
        if (recved == 0)
        {
          closesocket(cli._remoteSock);
          delete client;
        }
      }
    }
    else
    {
      std::cerr << "Can't receive second\n";
    }
  }
  else
  {
    std::cerr << "Can't receive\n";
  }
#endif
  
  WSACleanup();
}
