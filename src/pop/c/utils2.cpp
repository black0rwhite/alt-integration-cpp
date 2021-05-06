// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "entities/altblock.hpp"
#include "entities/popdata.hpp"
#include "entities/publication_data.hpp"
#include "pop_context2.hpp"
#include "veriblock/pop/assert.hpp"
#include "veriblock/pop/c/utils2.h"

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ENTITY_NAME(publication_data) *,
                           generate_publication_data,
                           POP_ARRAY_NAME(u8) endorsed_block_header,
                           POP_ARRAY_NAME(u8) tx_root,
                           POP_ARRAY_NAME(u8) payout_info,
                           const POP_ENTITY_NAME(pop_data) * pop_data) {
  VBK_ASSERT(self);
  VBK_ASSERT(pop_data);
  VBK_ASSERT(endorsed_block_header.data);
  VBK_ASSERT(tx_root.data);
  VBK_ASSERT(payout_info.data);

  using namespace altintegration;

  std::vector<uint8_t> v_endorsed_block_header(
      endorsed_block_header.data,
      endorsed_block_header.data + endorsed_block_header.size);
  std::vector<uint8_t> v_tx_root(tx_root.data, tx_root.data + tx_root.size);
  std::vector<uint8_t> v_payout_info(payout_info.data,
                                     payout_info.data + payout_info.size);

  PublicationData publication_data;
  if (!self->ref->generatePublicationData(publication_data,
                                          v_endorsed_block_header,
                                          v_tx_root,
                                          pop_data->ref,
                                          v_payout_info)) {
    return nullptr;
  }

  auto* res = new POP_ENTITY_NAME(publication_data);
  res->ref = std::move(publication_data);
  return res;
}

POP_ENTITY_CUSTOM_FUNCTION(pop_context,
                           POP_ARRAY_NAME(u8),
                           calculate_top_level_merkle_root,
                           POP_ARRAY_NAME(u8) tx_root,
                           POP_ARRAY_NAME(u8) prev_block_hash,
                           const POP_ENTITY_NAME(pop_data) * pop_data) {
  VBK_ASSERT(self);
  VBK_ASSERT(pop_data);
  VBK_ASSERT(tx_root.data);
  VBK_ASSERT(prev_block_hash.data);

  std::vector<uint8_t> v_hash(prev_block_hash.data,
                              prev_block_hash.data + prev_block_hash.size);

  auto* index = self->ref->getAltBlockTree().getBlockIndex(v_hash);
  VBK_ASSERT_MSG(index != nullptr,
                 "Can't find a block with hash %s. Did you forget to make "
                 "TopLevelMerkleRoot calculation stateful?",
                 altintegration::HexStr(v_hash));

  auto hash = altintegration::CalculateTopLevelMerkleRoot(
      std::vector<uint8_t>(tx_root.data, tx_root.data + tx_root.size),
      pop_data->ref,
      index,
      self->ref->getAltBlockTree().getParams());

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}