#include <assert.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "AsyncRpc/coroutine/coroutine_hook.h"
#include "AsyncRpc/coroutine/coroutine.h"
#include "AsyncRpc/net/fd_event.h"
#include "AsyncRpc/net/reactor.h"
#include "AsyncRpc/net/timer.h"
#include "AsyncRpc/util/log.h"
#include "AsyncRpc/util/config.h"

#define HOOK_SYS_FUNC(name) name##_fun_ptr_t g_sys_##name##_fun = (name##_fun_ptr_t)dlsym(RTLD_NEXT, #name);


HOOK_SYS_FUNC(accept);
HOOK_SYS_FUNC(read);
HOOK_SYS_FUNC(write);
HOOK_SYS_FUNC(connect);
HOOK_SYS_FUNC(sleep);

// static int g_hook_enable = false;

// static int g_max_timeout = 75000;


namespace AsyncRpc {

extern AsyncRpc::Config::ptr gRpcConfig;

static bool g_hook = true;

void SetHook(bool value) {
	g_hook = value;
}

void toEpoll(AsyncRpc::FdWraper::ptr fd_event, int events) {
	
	AsyncRpc::Coroutine* cur_cor = AsyncRpc::Coroutine::GetCurrentCoroutine() ;
	if (events & AsyncRpc::IOEvent::READ) {
		DebugLog << "fd:[" << fd_event->getFd() << "], register read event to epoll";
		// fd_event->setCallBack(tinyrpc::IOEvent::READ, 
		// 	[cur_cor, fd_event]() {
		// 		tinyrpc::Coroutine::Resume(cur_cor);
		// 	}
		// );
		fd_event->setCoroutine(cur_cor);
		fd_event->addListenEvents(AsyncRpc::IOEvent::READ);
	}
	if (events & AsyncRpc::IOEvent::WRITE) {
		DebugLog << "fd:[" << fd_event->getFd() << "], register write event to epoll";
		// fd_event->setCallBack(tinyrpc::IOEvent::WRITE, 
		// 	[cur_cor]() {
		// 		tinyrpc::Coroutine::Resume(cur_cor);
		// 	}
		// );
		fd_event->setCoroutine(cur_cor);
		fd_event->addListenEvents(AsyncRpc::IOEvent::WRITE);
	}
	// fd_event->updateToReactor();
}

ssize_t read_hook(int fd, void *buf, size_t count) {
	DebugLog << "this is hook read";
  if (AsyncRpc::Coroutine::IsMainCoroutine()) {
    DebugLog << "hook disable, call sys read func";
    return g_sys_read_fun(fd, buf, count);
  }

	AsyncRpc::Reactor::GetReactor();
	// assert(reactor != nullptr);

  AsyncRpc::FdWraper::ptr fd_event = AsyncRpc::FdEventContainer::GetFdContainer()->getFdEvent(fd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(AsyncRpc::Reactor::GetReactor());  
  }

	// if (fd_event->isNonBlock()) {
		// DebugLog << "user set nonblock, call sys func";
		// return g_sys_read_fun(fd, buf, count);
	// }

	fd_event->setNonBlock();

	// must fitst register read event on epoll
	// because reactor should always care read event when a connection sockfd was created
	// so if first call sys read, and read return success, this fucntion will not register read event and return
	// for this connection sockfd, reactor will never care read event
  ssize_t n = g_sys_read_fun(fd, buf, count);
  if (n > 0) {
    return n;
  } 

	toEpoll(fd_event, AsyncRpc::IOEvent::READ);

	DebugLog << "read func to yield";
	AsyncRpc::Coroutine::Yield();

	fd_event->delListenEvents(AsyncRpc::IOEvent::READ);
	fd_event->clearCoroutine();
	// fd_event->updateToReactor();

	DebugLog << "read func yield back, now to call sys read";
	return g_sys_read_fun(fd, buf, count);

}

int accept_hook(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	DebugLog << "this is hook accept";
  if (AsyncRpc::Coroutine::IsMainCoroutine()) {
    DebugLog << "hook disable, call sys accept func";
    return g_sys_accept_fun(sockfd, addr, addrlen);
  }
	AsyncRpc::Reactor::GetReactor();
	// assert(reactor != nullptr);

  AsyncRpc::FdWraper::ptr fd_event = AsyncRpc::FdEventContainer::GetFdContainer()->getFdEvent(sockfd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(AsyncRpc::Reactor::GetReactor());  
  }

	// if (fd_event->isNonBlock()) {
		// DebugLog << "user set nonblock, call sys func";
		// return g_sys_accept_fun(sockfd, addr, addrlen);
	// }

	fd_event->setNonBlock();

  int n = g_sys_accept_fun(sockfd, addr, addrlen);
  if (n > 0) {
    return n;
  } 

	toEpoll(fd_event, AsyncRpc::IOEvent::READ);
	
	DebugLog << "accept func to yield";
	AsyncRpc::Coroutine::Yield();

	fd_event->delListenEvents(AsyncRpc::IOEvent::READ);
	// fd_event->updateToReactor();

	DebugLog << "accept func yield back, now to call sys accept";
	return g_sys_accept_fun(sockfd, addr, addrlen);

}

ssize_t write_hook(int fd, const void *buf, size_t count) {
	DebugLog << "this is hook write";
  if (AsyncRpc::Coroutine::IsMainCoroutine()) {
    DebugLog << "hook disable, call sys write func";
    return g_sys_write_fun(fd, buf, count);
  }
	AsyncRpc::Reactor::GetReactor();
	// assert(reactor != nullptr);

  AsyncRpc::FdWraper::ptr fd_event = AsyncRpc::FdEventContainer::GetFdContainer()->getFdEvent(fd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(AsyncRpc::Reactor::GetReactor());  
  }

	// if (fd_event->isNonBlock()) {
		// DebugLog << "user set nonblock, call sys func";
		// return g_sys_write_fun(fd, buf, count);
	// }

	fd_event->setNonBlock();

  ssize_t n = g_sys_write_fun(fd, buf, count);
  if (n > 0) {
    return n;
  }

	toEpoll(fd_event, AsyncRpc::IOEvent::WRITE);

	DebugLog << "write func to yield";
	AsyncRpc::Coroutine::Yield();

	fd_event->delListenEvents(AsyncRpc::IOEvent::WRITE);
	fd_event->clearCoroutine();
	// fd_event->updateToReactor();

	DebugLog << "write func yield back, now to call sys write";
	return g_sys_write_fun(fd, buf, count);

}

int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	DebugLog << "this is hook connect";
  if (AsyncRpc::Coroutine::IsMainCoroutine()) {
    DebugLog << "hook disable, call sys connect func";
    return g_sys_connect_fun(sockfd, addr, addrlen);
  }
	AsyncRpc::Reactor* reactor = AsyncRpc::Reactor::GetReactor();
	// assert(reactor != nullptr);

  AsyncRpc::FdWraper::ptr fd_event = AsyncRpc::FdEventContainer::GetFdContainer()->getFdEvent(sockfd);
  if(fd_event->getReactor() == nullptr) {
    fd_event->setReactor(reactor);  
  }
	AsyncRpc::Coroutine* cur_cor = AsyncRpc::Coroutine::GetCurrentCoroutine();

	// if (fd_event->isNonBlock()) {
		// DebugLog << "user set nonblock, call sys func";
    // return g_sys_connect_fun(sockfd, addr, addrlen);
	// }
	
	fd_event->setNonBlock();
  int n = g_sys_connect_fun(sockfd, addr, addrlen);
  if (n == 0) {
    DebugLog << "direct connect succ, return";
    return n;
  } else if (errno != EINPROGRESS) {
		DebugLog << "connect error and errno is't EINPROGRESS, errno=" << errno <<  ",error=" << strerror(errno);
    return n;
  }

	DebugLog << "errno == EINPROGRESS";

  toEpoll(fd_event, AsyncRpc::IOEvent::WRITE);

	bool is_timeout = false;		// 是否超时

	// 超时函数句柄
  auto timeout_cb = [&is_timeout, cur_cor](){
		// 设置超时标志，然后唤醒协程
		is_timeout = true;
		AsyncRpc::Coroutine::Resume(cur_cor);
  };

  AsyncRpc::TimerEvent::ptr event = std::make_shared<AsyncRpc::TimerEvent>(gRpcConfig->m_max_connect_timeout, false, timeout_cb);
  
  AsyncRpc::Timer* timer = reactor->getTimer();  
  timer->addTimerEvent(event);

  AsyncRpc::Coroutine::Yield();

	// write事件需要删除，因为连接成功后后面会重新监听该fd的写事件。
	fd_event->delListenEvents(AsyncRpc::IOEvent::WRITE); 
	fd_event->clearCoroutine();
	// fd_event->updateToReactor();

	// 定时器也需要删除
	timer->delTimerEvent(event);

	n = g_sys_connect_fun(sockfd, addr, addrlen);
	if ((n < 0 && errno == EISCONN) || n == 0) {
		DebugLog << "connect succ";
		return 0;
	}

	if (is_timeout) {
    ErrorLog << "connect error,  timeout[ " << gRpcConfig->m_max_connect_timeout << "ms]";
		errno = ETIMEDOUT;
	} 

	DebugLog << "connect error and errno=" << errno <<  ", error=" << strerror(errno);
	return -1;

}

unsigned int sleep_hook(unsigned int seconds) {

	DebugLog << "this is hook sleep";
  if (AsyncRpc::Coroutine::IsMainCoroutine()) {
    DebugLog << "hook disable, call sys sleep func";
    return g_sys_sleep_fun(seconds);
  }

	AsyncRpc::Coroutine* cur_cor = AsyncRpc::Coroutine::GetCurrentCoroutine();

	bool is_timeout = false;
	auto timeout_cb = [cur_cor, &is_timeout](){
		DebugLog << "onTime, now resume sleep cor";
		is_timeout = true;
		// 设置超时标志，然后唤醒协程
		AsyncRpc::Coroutine::Resume(cur_cor);
  };

  AsyncRpc::TimerEvent::ptr event = std::make_shared<AsyncRpc::TimerEvent>(1000 * seconds, false, timeout_cb);
  
  AsyncRpc::Reactor::GetReactor()->getTimer()->addTimerEvent(event);

	DebugLog << "now to yield sleep";
	// beacuse read or wirte maybe resume this coroutine, so when this cor be resumed, must check is timeout, otherwise should yield again
	while (!is_timeout) {
		AsyncRpc::Coroutine::Yield();
	}

	// 定时器也需要删除
	// tinyrpc::Reactor::GetReactor()->getTimer()->delTimerEvent(event);

	return 0;

}


}


extern "C" {


int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	if (!AsyncRpc::g_hook) {
		return g_sys_accept_fun(sockfd, addr, addrlen);
	} else {
		return AsyncRpc::accept_hook(sockfd, addr, addrlen);
	}
}

ssize_t read(int fd, void *buf, size_t count) {
	if (!AsyncRpc::g_hook) {
		return g_sys_read_fun(fd, buf, count);
	} else {
		return AsyncRpc::read_hook(fd, buf, count);
	}
}

ssize_t write(int fd, const void *buf, size_t count) {
	if (!AsyncRpc::g_hook) {
		return g_sys_write_fun(fd, buf, count);
	} else {
		return AsyncRpc::write_hook(fd, buf, count);
	}
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if (!AsyncRpc::g_hook) {
		return g_sys_connect_fun(sockfd, addr, addrlen);
	} else {
		return AsyncRpc::connect_hook(sockfd, addr, addrlen);
	}
}

unsigned int sleep(unsigned int seconds) {
	if (!AsyncRpc::g_hook) {
		return g_sys_sleep_fun(seconds);
	} else {
		return AsyncRpc::sleep_hook(seconds);
	}
}

}