#include <fcntl.h>
#include <unistd.h>
#include "fd_event.h"

namespace AsyncRpc {

static FdEventContainer* g_FdContainer = nullptr;

FdWraper::FdWraper(AsyncRpc::Reactor* reactor, int fd/*=-1*/) : m_fd(fd), m_reactor(reactor) {
    if (reactor == nullptr) {
      ErrorLog << "create reactor first";
    }
    // assert(reactor != nullptr);
}

FdWraper::FdWraper(int fd) : m_fd(fd) {

}

FdWraper::~FdWraper() {}

void FdWraper::handleEvent(int flag) {

  if (flag == READ) {
    m_read_callback();
  } else if (flag == WRITE) {
    m_write_callback();
  } else {
    ErrorLog << "error flag";
  }

}

void FdWraper::setCallBack(IOEvent flag, std::function<void()> cb) {
  if (flag == READ) {
    m_read_callback = cb;
  } else if (flag == WRITE) {
    m_write_callback = cb;
  } else {
    ErrorLog << "error flag";
  }
}

std::function<void()> FdWraper::getCallBack(IOEvent flag) const {
  if (flag == READ) {
    return m_read_callback;
  } else if (flag == WRITE) {
    return m_write_callback;
  }
  return nullptr;
}

void FdWraper::addListenEvents(IOEvent event) {
  if (m_listen_events & event) {
    DebugLog << "already has this event, skip";
    return;
  }
  m_listen_events |= event;
  updateToReactor();
  // DebugLog << "add succ";
}

void FdWraper::delListenEvents(IOEvent event) {
  if (m_listen_events & event) {

    DebugLog << "delete succ";
    m_listen_events &= ~event;
    updateToReactor();
    return;
  }
  DebugLog << "this event not exist, skip";

}

void FdWraper::updateToReactor() {

  epoll_event event;
  event.events = m_listen_events;
  event.data.ptr = this;
  // DebugLog << "reactor = " << m_reactor << "log m_tid =" << m_reactor->getTid();
  if (!m_reactor) {
    m_reactor = AsyncRpc::Reactor::GetReactor();
  }

  m_reactor->addEvent(m_fd, event);
}

void FdWraper::unregisterFromReactor () {
  if (!m_reactor) {
    m_reactor = AsyncRpc::Reactor::GetReactor();
  }
  m_reactor->delEvent(m_fd);
  m_listen_events = 0;
  m_read_callback = nullptr;
  m_write_callback = nullptr;
}

int FdWraper::getFd() const {
  return m_fd;
}

void FdWraper::setFd(const int fd) {
  m_fd = fd;
}

int FdWraper::getListenEvents() const {
  return m_listen_events; 
}

Reactor* FdWraper::getReactor() const {
  return m_reactor;
}

void FdWraper::setReactor(Reactor* r) {
  m_reactor = r;
}

void FdWraper::setNonBlock() {
  if (m_fd == -1) {
    ErrorLog << "error, fd=-1";
    return;
  }

  int flag = fcntl(m_fd, F_GETFL, 0); 
  if (flag & O_NONBLOCK) {
    DebugLog << "fd already set o_nonblock";
    return;
  }

  fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
  flag = fcntl(m_fd, F_GETFL, 0); 
  if (flag & O_NONBLOCK) {
    DebugLog << "succ set o_nonblock";
  } else {
    ErrorLog << "set o_nonblock error";
  }

}

bool FdWraper::isNonBlock() {
  if (m_fd == -1) {
    ErrorLog << "error, fd=-1";
    return false;
  }
  int flag = fcntl(m_fd, F_GETFL, 0); 
  return (flag & O_NONBLOCK);

}

void FdWraper::setCoroutine(Coroutine* cor) {
  m_coroutine = cor;
}

void FdWraper::clearCoroutine() {
  m_coroutine = nullptr;
}

Coroutine* FdWraper::getCoroutine() {
  return m_coroutine;
}



FdWraper::ptr FdEventContainer::getFdEvent(int fd) {

  RWMutex::ReadLock rlock(m_mutex);
  if (fd < static_cast<int>(m_fds.size())) {
    AsyncRpc::FdWraper::ptr re = m_fds[fd]; 
    rlock.unlock();
    return re;
  }
  rlock.unlock();

  RWMutex::WriteLock wlock(m_mutex);
  int n = (int)(fd * 1.5);
  for (int i = m_fds.size(); i < n; ++i) {
    m_fds.push_back(std::make_shared<FdWraper>(i));
  }
  AsyncRpc::FdWraper::ptr re = m_fds[fd]; 
  wlock.unlock();
  return re;

}

FdEventContainer::FdEventContainer(int size) {
  for(int i = 0; i < size; ++i) {
    m_fds.push_back(std::make_shared<FdWraper>(i));
  }

}

FdEventContainer* FdEventContainer::GetFdContainer() {
  if (g_FdContainer == nullptr) {
    g_FdContainer = new FdEventContainer(1000); 
  }
  return g_FdContainer;
}


}
