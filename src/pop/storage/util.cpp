// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <veriblock/pop/blockchain/alt_block_tree.hpp>
#include <veriblock/pop/crypto/progpow.hpp>
#include <veriblock/pop/pop_context.hpp>
#include <veriblock/pop/storage/util.hpp>

namespace altintegration {

namespace detail {

template <typename Block>
using OnBlockCallback_t =
    std::function<void(typename Block::hash_t, const StoredBlockIndex<Block>&)>;

template <typename Index>
bool loadBlocksAndTip(
    std::vector<Index>& out,
    typename Index::block_t::hash_t& tipout,
    BlockReader& storage,
    ValidationState& state,
    const OnBlockCallback_t<typename Index::block_t>& onBlock = {}) {
  using block_t = typename Index::block_t;
  using hash_t = typename block_t::hash_t;

  auto it = storage.getBlockIterator<block_t>();
  for (it->seek_start(); it->valid(); it->next()) {
    Index val;
    if (!it->value(val)) {
      return state.Invalid("bad-value", "Can not read block data");
    }
    // if callback is supplied, execute it
    if (onBlock) {
      hash_t hash;
      if (!it->key(hash)) {
        return state.Invalid("bad-key", "Can not read block key");
      }
      onBlock(hash, val);
    }
    out.push_back(val);
  }

  bool res = storage.getTip<block_t>(tipout);

  if (!res && !out.empty()) {
    return state.Invalid(block_t::name() + "-bad-tip",
                         "Can not read block tip");
  }

  if (res && out.empty()) {
    return state.Invalid(block_t::name() + "-state-corruption",
                         "Can not read blocks");
  }

  return true;
}

template <typename BlockTreeT>
bool loadTree(BlockTreeT& out,
              const typename BlockTreeT::block_t::hash_t& tip_hash,
              std::vector<typename BlockTreeT::stored_index_t>& blocks,
              ValidationState& state) {
  if (!loadBlocksIntoTree(out, tip_hash, blocks, state)) {
    return state.Invalid("bad-tree");
  }

  auto* t = out.getBestChain().tip();
  VBK_ASSERT(t != nullptr);

  return true;
}

template <typename BlockTreeT>
bool validateLoadBlock(const BlockTreeT&,
                       const typename BlockTreeT::stored_index_t&,
                       ValidationState&) {
  return true;
}

template <>
bool validateLoadBlock(const AltBlockTree& tree,
                       const typename AltBlockTree::stored_index_t& index,
                       ValidationState& state) {
  const auto* current = tree.getBlockIndex(index.header->getHash());
  const auto endorsedByIds =
      map_get_id_from_pointers<uint256, const AltEndorsement>(
          current->endorsedBy);
  if (!same_vectors_unique_unordered(endorsedByIds,
                                     index.addon.endorsedByHashes)) {
    return state.Invalid("alt-block-invalid-stored-endorsed-by");
  }
  return true;
}

template <>
bool validateLoadBlock(const VbkBlockTree& tree,
                       const typename VbkBlockTree::stored_index_t& index,
                       ValidationState& state) {
  const auto* current = tree.getBlockIndex(index.header->getHash());
  const auto endorsedByIds =
      map_get_id_from_pointers<uint256, const VbkEndorsement>(
          current->endorsedBy);
  if (!same_vectors_unique_unordered(endorsedByIds,
                                     index.addon.endorsedByHashes)) {
    return state.Invalid("vbk-block-invalid-stored-endorsed-by");
  }

  const auto blockOfProofIds =
      map_get_id_from_pointers<uint256, const AltEndorsement>(
          current->blockOfProofEndorsements);
  if (!same_vectors_unordered(blockOfProofIds,
                              index.addon.blockOfProofEndorsementHashes)) {
    return state.Invalid(
        "vbk-block-invalid-stored-block-of-proof-endorsements");
  }
  return true;
}

template <>
bool validateLoadBlock(const BtcBlockTree& tree,
                       const typename BtcBlockTree::stored_index_t& index,
                       ValidationState& state) {
  const auto* current = tree.getBlockIndex(index.header->getHash());
  const auto blockOfProofIds =
      map_get_id_from_pointers<uint256, const VbkEndorsement>(
          current->blockOfProofEndorsements);
  if (!same_vectors_unordered(blockOfProofIds,
                              index.addon.blockOfProofEndorsementHashes)) {
    return state.Invalid(
        "btc-block-invalid-stored-block-of-proof-endorsements");
  }
  return true;
}

template <typename BlockTreeT>
bool loadValidateTree(
    const BlockTreeT& tree,
    const std::vector<typename BlockTreeT::stored_index_t>& blocks,
    ValidationState& state) {
  using block_t = typename BlockTreeT::block_t;

  for (const auto& block : blocks) {
    if (!validateLoadBlock(tree, block, state)) {
      return state.Invalid("load-validate-tree",
                           fmt::format("Invalid stored {} block {}",
                                       block_t::name(),
                                       block.toPrettyString()));
    }
  }
  return true;
}

}  // namespace detail

bool loadTrees(PopContext& context,
               BlockReader& storage,
               ValidationState& state) {
  std::vector<typename BtcBlockTree::stored_index_t> btcblocks;
  typename BtcBlock::hash_t btctip;
  if (!detail::loadBlocksAndTip(btcblocks, btctip, storage, state)) {
    return state.Invalid("load-btc-tree-blocks");
  }
  std::vector<typename VbkBlockTree::stored_index_t> vbkblocks;
  typename VbkBlock::hash_t vbktip;
  if (!detail::loadBlocksAndTip(
          vbkblocks,
          vbktip,
          storage,
          state,  // on every block, take its hash and warmup progpow header
                  // cache
          [](VbkBlock::hash_t hash, const StoredBlockIndex<VbkBlock>& index) {
            auto serializedHeader = SerializeToRaw(*index.header);
            progpow::insertHeaderCacheEntry(serializedHeader, std::move(hash));
          })) {
    return state.Invalid("load-vbk-tree-blocks");
  }
  std::vector<typename AltBlockTree::stored_index_t> altblocks;
  typename AltBlock::hash_t alttip;
  if (!detail::loadBlocksAndTip(altblocks, alttip, storage, state)) {
    return state.Invalid("load-alt-tree-blocks");
  }

  if (!detail::loadTree(
          context.getAltBlockTree().btc(), btctip, btcblocks, state)) {
    return state.Invalid("failed-to-load-btc-tree");
  }

  if (!detail::loadTree(
          context.getAltBlockTree().vbk(), vbktip, vbkblocks, state)) {
    return state.Invalid("failed-to-load-vbk-tree");
  }

  if (!detail::loadTree(context.getAltBlockTree(), alttip, altblocks, state)) {
    return state.Invalid("failed-to-load-alt-tree");
  }

  VBK_ASSERT_MSG(detail::loadValidateTree(
                     context.getAltBlockTree().btc(), btcblocks, state),
                 "Failed to validate stored BTC tree");
  VBK_ASSERT_MSG(detail::loadValidateTree(
                     context.getAltBlockTree().vbk(), vbkblocks, state),
                 "Failed to validate stored VBK tree");
  VBK_ASSERT_MSG(
      detail::loadValidateTree(context.getAltBlockTree(), altblocks, state),
      "Failed to validate stored ALT tree");
  return true;
}

template <typename BlockIndexT>
void validateBlockIndex(const BlockIndexT&) {}

template <>
void validateBlockIndex(const BlockIndex<VbkBlock>& index) {
  const auto& vtbids = index.getPayloadIds<VTB>();
  VBK_ASSERT_MSG(!hasDuplicateIds<VTB>(vtbids), "Duplicate VTB IDs");
}

void saveTrees(const AltBlockTree& tree, BlockBatch& batch) {
  saveTree(tree.btc(), batch);
  saveTree(tree.vbk(), batch);
  saveTree(tree, batch);
}

}  // namespace altintegration