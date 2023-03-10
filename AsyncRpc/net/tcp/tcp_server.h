#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include <map>
#include <google/protobuf/service.h>
#include "AsyncRpc/net/reactor.h"
#include "AsyncRpc/net/fd_event.h"
#include "AsyncRpc/net/timer.h"
#include "AsyncRpc/net/net_address.h"
#include "AsyncRpc/net/tcp/tcp_connection.h"
#include "AsyncRpc/net/tcp/io_thread.h"
#include "AsyncRpc/net/tcp/tcp_connection_time_wheel.h"
#include "AsyncRpc/net/abstract_codec.h"
#include "AsyncRpc/net/abstract_dispatcher.h"
#include "AsyncRpc/net/http/http_dispatcher.h"
#include "AsyncRpc/net/http/http_servlet.h"


namespace AsyncRpc {

class TcpAcceptor {

 public:

  typedef std::shared_ptr<TcpAcceptor> ptr;
  TcpAcceptor(NetAddress::ptr net_addr);

  void init();

  int toAccept();

  ~TcpAcceptor();

  NetAddress::ptr getPeerAddr() {
    return m_peer_addr;
  }

  NetAddress::ptr geLocalAddr() {
    return m_local_addr;
  }
 
 private:
  int m_family {-1};
  int m_fd {-1};

  NetAddress::ptr m_local_addr {nullptr};
  NetAddress::ptr m_peer_addr {nullptr};

};


class TcpServer {

 public:

  typedef std::shared_ptr<TcpServer> ptr;

	TcpServer(NetAddress::ptr addr, ProtocalType type = TinyPb_Protocal);

  ~TcpServer();

  void start();

  void addCoroutine(AsyncRpc::Coroutine::ptr cor);

  bool registerService(std::shared_ptr<google::protobuf::Service> service);

  bool registerHttpServlet(const std::string& url_path, HttpServlet::ptr servlet);

  TcpConnection::ptr addClient(IOThread* io_thread, int fd);

  void freshTcpConnection(TcpTimeWheel::TcpConnectionSlot::ptr slot);


 public:
  AbstractDispatcher::ptr getDispatcher();

  AbstractCodeC::ptr getCodec();

  NetAddress::ptr getPeerAddr();

  NetAddress::ptr getLocalAddr();

  IOThreadPool::ptr getIOThreadPool();

  TcpTimeWheel::ptr getTimeWheel();


 private:
  void MainAcceptCorFunc();

  void ClearClientTimerFunc();

 private:
  
  NetAddress::ptr m_addr;

  TcpAcceptor::ptr m_acceptor;

  int m_tcp_counts {0};

  Reactor* m_main_reactor {nullptr};

  bool m_is_stop_accept {false};

  Coroutine::ptr m_accept_cor;
  
  AbstractDispatcher::ptr m_dispatcher;

  AbstractCodeC::ptr m_codec;

  IOThreadPool::ptr m_io_pool;

  ProtocalType m_protocal_type {TinyPb_Protocal};

  TcpTimeWheel::ptr m_time_wheel;

  std::map<int, std::shared_ptr<TcpConnection>> m_clients;

  TimerEvent::ptr m_clear_clent_timer_event {nullptr};

};

}

#endif
