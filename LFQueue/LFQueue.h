//
// Created by ShiJiu on 2023/2/4.
//

//only use in 32 addr
#pragma once
#ifndef T_RPC_LFQUEUE_H
#define T_RPC_LFQUEUE_H
#include <cstdint>
#include "cassert"
#include <atomic>

#include<queue>

struct alignas(16) NodeInfo{
    uintptr_t next;
    uint64_t seq;
};
using AtomicNodeInfo=std::atomic<NodeInfo>;
struct Node{
    uintptr_t data;
    AtomicNodeInfo info;
};
class LFQueue{
private:
    volatile AtomicNodeInfo head_,tail_;
public:
    LFQueue();
    ~LFQueue();
    void Enqueue(Node*node);
    Node* Dequeue();
};
LFQueue::LFQueue() {
    assert(head_.is_lock_free());
    auto ptr=new Node{0,NodeInfo{0,0}};
    head_.store(NodeInfo{reinterpret_cast<uintptr_t>(ptr),0});
    tail_.store(head_);
}
LFQueue::~LFQueue() {

}
void LFQueue::Enqueue(Node* node) {
    node->info.store({0,0});
    auto old=tail_.exchange({reinterpret_cast<uintptr_t>(node),0});
    reinterpret_cast<Node*>(old.next)->info.store(tail_);
}
Node* LFQueue::Dequeue() {
    NodeInfo headSnapshot,newHead;
    Node*nextNode;
    do{
        headSnapshot=head_;
        nextNode=reinterpret_cast<Node*>(reinterpret_cast<Node*>(headSnapshot.next)->info.load().next);
        if(nextNode==nullptr)return nextNode;
        newHead=NodeInfo{reinterpret_cast<uintptr_t>(nextNode),headSnapshot.seq+1};
    }while(head_.compare_exchange_weak(headSnapshot,newHead));
    delete reinterpret_cast<Node*>(headSnapshot.next);
    auto ret=new Node{nextNode->data};
    return ret;
}
#endif //T_RPC_LFQUEUE_H
