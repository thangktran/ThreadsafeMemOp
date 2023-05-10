#include "include/ThreadSafeMemOp.hpp"
#include "include/Debug.hpp"

#include <mutex>
#include <iostream>

#include <unistd.h>


namespace tsmo {


constexpr size_t SBRK_MULTIPLIER = 10;
constexpr size_t ALIGN_SIZE = 8; // linux requires 8-byte alignment
size_t align(size_t size) {
    return ((size + (ALIGN_SIZE-1)) & ~(ALIGN_SIZE-1));
} 


typedef struct MetadataNode {
    size_t size;
    bool free;
    MetadataNode* next = nullptr;
    MetadataNode* prev = nullptr;
} MetadataNode;


// ----------- Helpers -------------
MetadataNode* getFreeBlock(size_t size); // implement first fit.
void splitBlock(MetadataNode* node, size_t size);
void mergeBlocks(MetadataNode* blockHeader); // aka coalescing from literature
MetadataNode* increaseDataSpace(size_t size);

void* increasePtrByBytes(void* ptr, size_t size);
void* decreasePtrByBytes(void* ptr, size_t size);


// ----------- details implementation -------------

// global variables
const size_t METADATA_ALIGNED_SIZE = align(sizeof(MetadataNode));
std::mutex globalMutex;
void* currentBrkPoint = nullptr;
MetadataNode* globalHead = nullptr;
MetadataNode* globalTail = nullptr;


void* malloc(size_t size) {

    std::scoped_lock<std::mutex> lck(globalMutex);

    MetadataNode* freeBlock = getFreeBlock(size);

    // not enough memory -> increase sbrk
    if (freeBlock == nullptr) {
        freeBlock = increaseDataSpace(size);
    }

    // could not request any more memory from sbrk
    if (freeBlock == nullptr) {
        return nullptr;
    }

    return increasePtrByBytes(freeBlock, METADATA_ALIGNED_SIZE);
}


void free(void* ptr) {

    std::scoped_lock<std::mutex> lck(globalMutex);

    MetadataNode* blockHeader = static_cast<MetadataNode*>( decreasePtrByBytes(ptr, METADATA_ALIGNED_SIZE) );

    // sanity check
    if (blockHeader < globalHead or blockHeader > currentBrkPoint) {
        std::cerr << "Invalid memory block" << std::endl;
        return;
    }

    // TODO: invalidate freed memory block
    blockHeader->free = true;
    mergeBlocks(blockHeader);
}


// ----------- helper details implementation -------------

MetadataNode* getFreeBlock(size_t size) {

    MetadataNode* currHead = globalHead;

    MetadataNode* currentTail = nullptr;

    while(currHead != nullptr) {

        if (currHead->size > (size+METADATA_ALIGNED_SIZE) and currHead->free) {

            splitBlock(currHead, size);
            return currHead;
        }
        
        currentTail = currHead;
        currHead = currHead->next;
    }

    if (currHead == nullptr and currentTail != nullptr) {
        globalTail = currentTail;
    }

    return currHead;
}


// assume that node->size is sufficiently large enough.
// this should be checked by the client who calls this func.
void splitBlock(MetadataNode* node, size_t size) {

    void* newPosition = increasePtrByBytes(node, METADATA_ALIGNED_SIZE + size);
    MetadataNode* splittedBlock = static_cast<MetadataNode*>(newPosition);

    splittedBlock->size = node->size - size - METADATA_ALIGNED_SIZE;
    splittedBlock->free = true;
    splittedBlock->next = node->next;
    splittedBlock->prev = node;

    if (splittedBlock->next != nullptr) {
        splittedBlock->next->prev = splittedBlock;
    }

    node->size = size;
    node->free = false;
    node->next = splittedBlock;
}


void mergeBlocks(MetadataNode* blockHeader) {


    // merge next block if possible.
    MetadataNode* next = blockHeader->next;
    
    while (next != nullptr and next->free) {
        
        blockHeader->size = blockHeader->size + next->size + METADATA_ALIGNED_SIZE;
        blockHeader->next = next->next;

        if (next->next != nullptr) {
            next->next->prev = blockHeader;
        }

        next = blockHeader->next;
    }

    // merge previous block if possible.
    MetadataNode* prev = blockHeader->prev;

    while (prev != nullptr and prev->free) {
        
        prev->size = prev->size + blockHeader->size + METADATA_ALIGNED_SIZE;
        prev->next = blockHeader->next;

        if (blockHeader->next != nullptr) {
            blockHeader->next->prev = prev;
        }

        prev = prev->prev;
    }

}


MetadataNode* increaseDataSpace(size_t size) {

    currentBrkPoint = sbrk(0);
    MetadataNode* newBlock = static_cast<MetadataNode*>(currentBrkPoint);

    if (globalHead == nullptr) {
        globalHead = newBlock;
    }

    size_t allocationSize = SBRK_MULTIPLIER * (size + METADATA_ALIGNED_SIZE);
    allocationSize = align(allocationSize);
    void* result = sbrk(allocationSize);

    if (result == (void*)-1) {
        std::cerr << "Cannot increase data space" << std::endl;
        return nullptr;
    }

    currentBrkPoint = increasePtrByBytes(currentBrkPoint, allocationSize);

    newBlock->size = allocationSize - METADATA_ALIGNED_SIZE;
    newBlock->free = false;
    newBlock->next = nullptr;
    newBlock->prev = globalTail;
    
    if (newBlock->prev != nullptr) {
        newBlock->prev->next = newBlock;
    }

    if (newBlock->size > size + METADATA_ALIGNED_SIZE) {
        splitBlock(newBlock, size);
    }

    return newBlock;
}


void* increasePtrByBytes(void* ptr, size_t size) {
    return static_cast<char*>(ptr) + size;
}


void* decreasePtrByBytes(void* ptr, size_t size) {
    return static_cast<char*>(ptr) - size;
}


void debug::printBlocks() {
    
    unsigned idx = 0;
    MetadataNode* currentHead = globalHead;

    std::cout << "Memory List" << std::endl;
    std::cout << "=======================================" << std::endl;

    while (currentHead != nullptr) {
        
        std::cout << "[" << idx << "] head: " << currentHead << std::endl;
        std::cout << "[" << idx << "] head->size: " << currentHead->size << std::endl;
        std::cout << "[" << idx << "] head->free: " << currentHead->free << std::endl;
        std::cout << "[" << idx << "] head->next: " << currentHead->next << std::endl;
        std::cout << "[" << idx << "] head->prev: " << currentHead->prev << std::endl;
        std::cout << std::endl;

        currentHead = currentHead->next;
        ++idx;
    }

}


bool debug::operator==(const tsmo::debug::BlockInfo& lhs, const tsmo::debug::BlockInfo& rhs) {
    return std::tie(lhs.size, lhs.free) == std::tie(rhs.size, rhs.free);
}

bool debug::operator!=(const tsmo::debug::BlockInfo& lhs, const tsmo::debug::BlockInfo& rhs) {
    return std::tie(lhs.size, lhs.free) != std::tie(rhs.size, rhs.free);
}


std::vector<debug::BlockInfo> debug::getBlocksInfo() {

    std::vector<debug::BlockInfo> results;

    MetadataNode* currHead = globalHead;

    while (currHead != nullptr) {

        results.emplace_back(currHead->size, currHead->free);
        currHead = currHead->next;

    }

    return results;
}


} // namespace tsmo

