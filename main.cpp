#include <iostream>
#include <cstring>

#include "include/ThreadSafeMemOp.hpp"
#include "include/Debug.hpp"

int main() {
    
    tsmo::debug::printBlocks();

    void* test0 = tsmo::malloc(30);
    tsmo::debug::printBlocks();

    void* test1 = tsmo::malloc(10);
    tsmo::debug::printBlocks();

    void* test2 = tsmo::malloc(100);
    tsmo::debug::printBlocks();


    tsmo::free(test1);
    tsmo::debug::printBlocks();

    tsmo::free(test0);
    tsmo::debug::printBlocks();

    tsmo::free(test2);
    tsmo::debug::printBlocks();

    return 0;
}