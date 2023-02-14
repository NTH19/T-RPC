#ifndef TINYRPC_NET_TINYPB_TINYPB_RPC_CHANNEL_H
#define TINYRPC_NET_TINYPB_TINYPB_RPC_CHANNEL_H 

#include <memory>
#include <google/protobuf/service.h>
#include "AsyncRpc/net/net_address.h"
#include "AsyncRpc/net/tcp/tcp_client.h"

namespace AsyncRpc {

class RpcChannel : public google::protobuf::RpcChannel {

 public:
  typedef std::shared_ptr<RpcChannel> ptr;
  RpcChannel(NetAddress::ptr addr);
  ~RpcChannel() = default;

void CallMethod(const google::protobuf::MethodDescriptor* method, 
    google::protobuf::RpcController* controller, 
    const google::protobuf::Message* request, 
    google::protobuf::Message* response, 
    google::protobuf::Closure* done);
 
 private:
  NetAddress::ptr m_addr;
  // TcpClient::ptr m_client;

};

}



#endif