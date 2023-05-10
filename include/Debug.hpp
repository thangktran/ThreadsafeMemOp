#pragma once

#include <vector>
#include <tuple>

namespace tsmo::debug {

void printBlocks();

typedef struct BlockInfo {
    size_t size;
    bool free;
} BlockInfo;

bool operator==(const tsmo::debug::BlockInfo& lhs, const tsmo::debug::BlockInfo& rhs);
bool operator!=(const tsmo::debug::BlockInfo& lhs, const tsmo::debug::BlockInfo& rhs);

std::vector<BlockInfo> getBlocksInfo();

} // namespace tsmo::debug