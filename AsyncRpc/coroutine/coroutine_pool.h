#ifndef TINYRPC_COROUTINE_COUROUTINE_POOL_H
#define TINYRPC_COROUTINE_COUROUTINE_POOL_H

#include <vector>
#include "AsyncRpc/coroutine/coroutine.h"
#include "AsyncRpc/net/mutex.h"
#include "AsyncRpc/coroutine/memory.h"

namespace AsyncRpc {

class CoroutinePool {

 public:
  CoroutinePool(int pool_size, int stack_size = 1024 * 128);
  ~CoroutinePool();

  Coroutine::ptr getCoroutineInstanse();

  void returnCoroutine(Coroutine::ptr cor);

 private:
  int m_pool_size {0};
  int m_stack_size {0};

  // first--ptr of cor
  // second
  //    false -- can be dispatched
  //    true -- can't be dispatched
  std::vector<std::pair<Coroutine::ptr, bool>> m_free_cors;

  Mutex m_mutex;

  std::vector<Memory::ptr> m_memory_pool;
};


CoroutinePool* GetCoroutinePool();

}


#endif