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
#include "optional"
template<typename T>
class LFQueue{
private:
    struct Node{
        T data;
        Node* next;
    };
    struct alignas(16) HeadInfo{
        Node* next;
        uint64_t seq;
    };
    volatile std::atomic<HeadInfo> head_;
    volatile std::atomic<Node*>tail_;
public:
    LFQueue();
    ~LFQueue();
    void Enqueue(T data);
    std::optional<T> Dequeue();
};
template<typename T>
LFQueue<T>::LFQueue() {
    assert(head_.is_lock_free());
    assert(tail_.is_lock_free());
    auto ptr=new Node{0, nullptr};
    head_.store({ptr,0});
    tail_.store(ptr);
}
template<typename T>
LFQueue<T>::~LFQueue() {

}
template<typename T>
void LFQueue<T>::Enqueue(T data) {
    auto node = new LFQueue::Node{data, nullptr};
    auto old = tail_.exchange(node);
    old->next=node;
}
template<typename T>
std::optional<T> LFQueue<T>::Dequeue() {
    HeadInfo headSnapshot,newHead;
    headSnapshot=head_.load();
    Node*nextNode;
    do{
        nextNode=headSnapshot.next->next;
        if(nextNode==nullptr)return std::nullopt;
        newHead=HeadInfo{nextNode,headSnapshot.seq};
    }while(!head_.compare_exchange_strong(headSnapshot,newHead));
    return {nextNode->data};
}
#endif //T_RPC_LFQUEUE_H
