//
// Created by ShiJiu on 2023/2/4.
//

//only use in 32 addr
#pragma once
#ifndef T_RPC_LFQUEUE_H
#define T_RPC_LFQUEUE_H
#include <cstdint>
#include <atomic>
using AtomicInfo =std::atomic_int64_t;

struct AtomicNode
{
    volatile AtomicInfo _info;
    void* data;
};


class LFQueue
{
    volatile AtomicInfo _tail;
    volatile AtomicInfo _head;
public:
    LFQueue();
    ~LFQueue();
    void Enqueue(AtomicNode* node);
    AtomicNode* Dequeue();
};

void LFQueue::Enqueue(AtomicNode *node) {
    int64_t val=0;
    auto ptr=reinterpret_cast<int32_t *>(&val);
    node->_info.store(val);
    ptr[0]=reinterpret_cast<int32_t>(node);
    auto old=_tail.exchange(val);
    reinterpret_cast<AtomicNode*>(reinterpret_cast<int32_t*>(&old)[0])->_info.store(val);
}
AtomicNode* LFQueue::Dequeue() {
    AtomicNode* ret= nullptr;
    int64_t info;
    int64_t newHead;
    do{
       info=_head;
       auto ptr=reinterpret_cast<int32_t *>(&info);
       if(ptr[0]==0)return nullptr;
       ret= reinterpret_cast<AtomicNode*>(ptr[0]);
       newHead=ret->_info.load();
       reinterpret_cast<int32_t*>(&newHead)[1]=ptr[1]+1;
    }while(_head.compare_exchange_strong(info,newHead));
    return ret;
}
#endif //T_RPC_LFQUEUE_H
