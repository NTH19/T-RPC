#include <iostream>
#include <atomic>
using namespace std;

struct A{
    uintptr_t next;
    uint64_t seq;
};
int main() {
    atomic<A>t;

    std::cout <<t.is_lock_free() << std::endl;
    return 0;
}
