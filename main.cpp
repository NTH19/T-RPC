#include <google/protobuf/service.h>
#include <atomic>
#include <future>
#include "AsyncRpc/util/start.h"
#include "AsyncRpc/net/http/http_request.h"
#include "AsyncRpc/net/http/http_response.h"
#include "AsyncRpc/net/http/http_servlet.h"
#include "AsyncRpc/net/http/http_define.h"
#include "AsyncRpc/net/rpc/rpc_channel.h"
#include "AsyncRpc/net/rpc/rpc_controller.h"
#include "AsyncRpc/net/rpc/rpc_closure.h"
#include "AsyncRpc/net/net_address.h"
#include "test_tinypb_server.pb.h"


const char* html = "<html><body><h1>Welcome to TinyRPC, just enjoy it!</h1><p>%s</p></body></html>";

AsyncRpc::IPAddress::ptr addr = std::make_shared<AsyncRpc::IPAddress>("127.0.0.1", 20000);

class BlockCallHttpServlet : public AsyncRpc::HttpServlet {
 public:
  BlockCallHttpServlet() = default;
  ~BlockCallHttpServlet() = default;

  void handle(AsyncRpc::HttpRequest* req, AsyncRpc::HttpResponse* res) {
    AppDebugLog << "BlockCallHttpServlet get request ";
    AppDebugLog << "BlockCallHttpServlet success recive http request, now to get http response";
    setHttpCode(res, AsyncRpc::HTTP_OK);
    setHttpContentType(res, "text/html;charset=utf-8");

    queryAgeReq rpc_req;
    queryAgeRes rpc_res;
    AppDebugLog << "now to call QueryServer TinyRPC server to query who's id is " << req->m_query_maps["id"];
    rpc_req.set_id(std::atoi(req->m_query_maps["id"].c_str()));

    AsyncRpc::RpcChannel channel(addr);
    QueryService_Stub stub(&channel);

    AsyncRpc::RpcController rpc_controller;
    rpc_controller.SetTimeout(5000);

    AppDebugLog << "BlockCallHttpServlet end to call RPC";
    stub.query_age(&rpc_controller, &rpc_req, &rpc_res, NULL);
    AppDebugLog << "BlockCallHttpServlet end to call RPC";

    if (rpc_controller.ErrorCode() != 0) {
      AppDebugLog << "failed to call QueryServer rpc server";
      char buf[512];
      sprintf(buf, html, "failed to call QueryServer rpc server");
      setHttpBody(res, std::string(buf));
      return;
    }

    if (rpc_res.ret_code() != 0) {
      std::stringstream ss;
      ss << "QueryServer rpc server return bad result, ret = " << rpc_res.ret_code() << ", and res_info = " << rpc_res.res_info();
      AppDebugLog << ss.str();
      char buf[512];
      sprintf(buf, html, ss.str().c_str());
      setHttpBody(res, std::string(buf));
      return;
    }

    std::stringstream ss;
    ss << "Success!! Your age is," << rpc_res.age() << " and Your id is " << rpc_res.id();

    char buf[512];
    sprintf(buf, html, ss.str().c_str());
    setHttpBody(res, std::string(buf));

  }

  std::string getServletName() {
    return "BlockCallHttpServlet";
  }

};


class QPSHttpServlet : public AsyncRpc::HttpServlet {
 public:
  QPSHttpServlet() = default;
  ~QPSHttpServlet() = default;

  void handle(AsyncRpc::HttpRequest* req, AsyncRpc::HttpResponse* res) {
    AppDebugLog << "QPSHttpServlet get request";
    setHttpCode(res, AsyncRpc::HTTP_OK);
    setHttpContentType(res, "text/html;charset=utf-8");

    std::stringstream ss;
    ss << "QPSHttpServlet Echo Success!! Your id is," << req->m_query_maps["id"];
    char buf[512];
    sprintf(buf, html, ss.str().c_str());
    setHttpBody(res, std::string(buf));
    AppDebugLog << ss.str();
  }

  std::string getServletName() {
    return "QPSHttpServlet";
  }

};


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Start TinyRPC server error, input argc is not 2!");
    printf("Start TinyRPC server like this: \n");
    printf("./server a.xml\n");
    return 0;
  }

  AsyncRpc::InitConfig(argv[1]);

  REGISTER_HTTP_SERVLET("/qps", QPSHttpServlet);

  REGISTER_HTTP_SERVLET("/block", BlockCallHttpServlet);

  AsyncRpc::StartRpcServer();
  return 0;
}