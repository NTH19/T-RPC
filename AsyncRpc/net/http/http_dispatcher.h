#ifndef TINYRPC_NET_HTTP_HTTP_DISPATCHER_H
#define TINYRPC_NET_HTTP_HTTP_DISPATCHER_H

#include <map>
#include <memory>
#include "AsyncRpc/net/abstract_dispatcher.h"
#include "AsyncRpc/net/http/http_servlet.h"


namespace AsyncRpc {

class HttpDispacther : public AbstractDispatcher {
 public:
  HttpDispacther() = default;

  ~HttpDispacther() = default;

  void dispatch(AbstractData* data, TcpConnection* conn);

  void registerServlet(const std::string& path, HttpServlet::ptr servlet);

 public:
  std::map<std::string, HttpServlet::ptr> m_servlets;
};


}



#endif