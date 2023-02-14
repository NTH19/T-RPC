#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "AsyncRpc/net/rpc/rpc_controller.h"

namespace AsyncRpc {

void RpcController::Reset() {}

bool RpcController::Failed() const {
  return m_is_failed;
}

std::string RpcController::ErrorText() const {
  return m_error_info;
}

void RpcController::StartCancel() {}

void RpcController::SetFailed(const std::string& reason) {
  m_is_failed = true;
  m_error_info = reason;
}

bool RpcController::IsCanceled() const {
  return false;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {

}

void RpcController::SetErrorCode(const int error_code) {
  m_error_code = error_code;
}

int RpcController::ErrorCode() const {
  return m_error_code; 
}

const std::string& RpcController::MsgSeq() const {
  return m_msg_req;
}

void RpcController::SetMsgReq(const std::string& msg_req) {
  m_msg_req = msg_req;
}

void RpcController::SetError(const int err_code, const std::string& err_info) {
  SetFailed(err_info);
  SetErrorCode(err_code);
}

void RpcController::SetPeerAddr(NetAddress::ptr addr) {
  m_peer_addr = addr;
}

void RpcController::SetLocalAddr(NetAddress::ptr addr) {
  m_local_addr = addr;
}
NetAddress::ptr RpcController::PeerAddr() {
  return m_peer_addr;
}
  
NetAddress::ptr RpcController::LocalAddr() {
  return m_local_addr;
}

void RpcController::SetTimeout(const int timeout) {
  m_timeout = timeout;
}
int RpcController::Timeout() const {
  return m_timeout;
}

void RpcController::SetMethodName(const std::string& name) {
  m_method_name = name;
}

std::string RpcController::GetMethodName() {
  return m_method_name;
}

void RpcController::SetMethodFullName(const std::string& name) {
  m_full_name = name;
}

std::string RpcController::GetMethodFullName() {
  return m_full_name;
}


}