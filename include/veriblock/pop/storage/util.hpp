// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
#define VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP

#include <vector>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/logger.hpp>
#include <veriblock/pop/storage/block_reader.hpp>
#include <veriblock/pop/validation_state.hpp>

#include "block_batch.hpp"

namespace altintegration {

//! efficiently loads `blocks` into tree (they must be sorted by height) and
//! does validation of these blocks. Sets tip after loading.
//! @invariant NOT atomic
template <typename BlockTreeT>
bool LoadBlocks(
    BlockTreeT& tree,
    std::vector<std::unique_ptr<typename BlockTreeT::index_t>>& blocks,
    const typename BlockTreeT::hash_t& tiphash,
    ValidationState& state) {
  using index_t = typename BlockTreeT::index_t;
  using block_t = typename BlockTreeT::block_t;
  VBK_LOG_WARN("Loading %d %s blocks with tip %s",
               blocks.size(),
               block_t::name(),
               HexStr(tiphash));
  VBK_ASSERT(tree.isBootstrapped() && "tree must be bootstrapped");

  // first, sort them by height
  std::sort(
      blocks.begin(),
      blocks.end(),
      [](const std::unique_ptr<index_t>& a, const std::unique_ptr<index_t>& b) {
        return a->getHeight() < b->getHeight();
      });

  for (auto& block : blocks) {
    // load blocks one by one
    if (!tree.loadBlock(std::move(block), state)) {
      return state.Invalid("load-tree");
    }
  }

  return tree.loadTip(tiphash, state);
}

//! @private
template <typename BlockIndexT>
void validateBlockIndex(const BlockIndexT&);

//! @private
template <typename BlockTreeT>
void SaveTree(
    BlockTreeT& tree,
    BlockBatch& batch,
    std::function<void(const typename BlockTreeT::index_t&)> validator) {
  using index_t = typename BlockTreeT::index_t;
  std::vector<const index_t*> dirty_indices;

  // map pair<hash, shared_ptr<index_t>> to vector<index_t*>
  for (auto& block : tree.getBlocks()) {
    auto& index = block.second;
    if (index->isDirty()) {
      index->unsetDirty();
      dirty_indices.push_back(index.get());
    }
  }

  // sort by height in descending order, because we need to calculate index hash
  // during saving. this is needed to be progpow-cache friendly, as cache will
  // be warm for last blocks.
  std::sort(dirty_indices.begin(),
            dirty_indices.end(),
            [](const index_t* a, const index_t* b) {
              return b->getHeight() < a->getHeight();
            });

  // write indices
  for (const index_t* index : dirty_indices) {
    validator(*index);
    batch.writeBlock(*index);
  }

  batch.writeTip(*tree.getBestChain().tip());
}

//! @private
template <typename BlockTreeT>
void SaveTree(BlockTreeT& tree, BlockBatch& batch) {
  SaveTree(tree, batch, &validateBlockIndex<typename BlockTreeT::index_t>);
}

struct AltBlockTree;

//! Save all (BTC/VBK/ALT) trees on disk in a single Batch.
void SaveAllTrees(const AltBlockTree& tree, BlockBatch& batch);

struct PopContext;

//! Load all (ALT/VBK/BTC) trees from disk into memory.
bool LoadAllTrees(PopContext& context,
                  BlockReader& reader,
                  ValidationState& state);

}  // namespace altintegration

#endif  // VERIBLOCK_POP_CPP_STORAGE_UTIL_HPP
