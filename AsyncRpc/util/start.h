#ifndef TINYRPC_COMM_START_H
#define TINYRPC_COMM_START_H

#include <google/protobuf/service.h>
#include <memory>
#include <stdio.h>
#include <functional>
#include "AsyncRpc/util/log.h"
#include "AsyncRpc/net/tcp/tcp_server.h"
#include "AsyncRpc/net/timer.h"

namespace tinyrpc {


#define REGISTER_HTTP_SERVLET(path, servlet) \
 do { \
  if(!tinyrpc::GetServer()->registerHttpServlet(path, std::make_shared<servlet>())) { \
    printf("Start TinyRPC server error, because register http servelt error, please look up rpc log get more details!\n"); \
    tinyrpc::Exit(0); \
  } \
 } while(0)\

#define REGISTER_SERVICE(service) \
 do { \
  if (!tinyrpc::GetServer()->registerService(std::make_shared<service>())) { \
    printf("Start TinyRPC server error, because register protobuf service error, please look up rpc log get more details!\n"); \
    tinyrpc::Exit(0); \
  } \
 } while(0)\


void InitConfig(const char* file);

// void RegisterService(google::protobuf::Service* service);

void StartRpcServer();

TcpServer::ptr GetServer();

int GetIOThreadPoolSize();

Config::ptr GetConfig();

void AddTimerEvent(TimerEvent::ptr event);

}

#endif