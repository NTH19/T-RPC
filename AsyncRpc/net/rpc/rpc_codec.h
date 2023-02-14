#ifndef TINYRPC_NET_TINYPB_TINYPB_CODEC_H
#define TINYRPC_NET_TINYPB_TINYPB_CODEC_H

#include <stdint.h>
#include "AsyncRpc/net/abstract_codec.h"
#include "AsyncRpc/net/abstract_data.h"
#include "AsyncRpc/net/rpc/rpc_data.h"

namespace AsyncRpc {


class RpcCodeC: public AbstractCodeC {
 public:
  // typedef std::shared_ptr<TinyPbCodeC> ptr;

  RpcCodeC();

  ~RpcCodeC ();

  // overwrite
  void encode(TcpBuffer* buf, AbstractData* data);
  
  // overwrite
  void decode(TcpBuffer* buf, AbstractData* data);

  // overwrite
  virtual ProtocalType getProtocalType();

  const char* encodePbData(RpcStruct* data, int& len);


};

} 


#endif
