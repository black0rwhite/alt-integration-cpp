// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include <memory>

// clang-format off
#include <veriblock/pop/adaptors/picojson.hpp>
// clang-format on

#include "btcblock.hpp"
#include "veriblock/pop/assert.hpp"

POP_ENTITY_FREE_SIGNATURE(btc_block) {
  if (self != nullptr) {
    delete self;
    self = nullptr;
  }
}

POP_ARRAY_FREE_SIGNATURE(btc_block) {
  if (self != nullptr) {
    delete[] self->data;
    self = nullptr;
  }
}

POP_ENTITY_GETTER_FUNCTION(btc_block, POP_ARRAY_NAME(u8), hash) {
  VBK_ASSERT(self);

  auto hash = self->ref.getHash();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_FUNCTION(btc_block, POP_ARRAY_NAME(u8), previous_block) {
  VBK_ASSERT(self);

  auto hash = self->ref.getPreviousBlock();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_FUNCTION(btc_block, POP_ARRAY_NAME(u8), merkle_root) {
  VBK_ASSERT(self);

  auto hash = self->ref.getMerkleRoot();

  POP_ARRAY_NAME(u8) res;
  res.data = new uint8_t[hash.size()];
  std::copy(hash.begin(), hash.end(), res.data);
  res.size = hash.size();

  return res;
}

POP_ENTITY_GETTER_FUNCTION(btc_block, uint32_t, version) {
  VBK_ASSERT(self);

  return self->ref.getVersion();
}

POP_ENTITY_GETTER_FUNCTION(btc_block, uint32_t, timestamp) {
  VBK_ASSERT(self);

  return self->ref.getTimestamp();
}

POP_ENTITY_GETTER_FUNCTION(btc_block, uint32_t, difficulty) {
  VBK_ASSERT(self);

  return self->ref.getDifficulty();
}

POP_ENTITY_GETTER_FUNCTION(btc_block, uint32_t, nonce) {
  VBK_ASSERT(self);

  return self->ref.getNonce();
}

POP_ENTITY_TO_JSON(btc_block) {
  VBK_ASSERT(self);

  std::string json =
      altintegration::ToJSON<picojson::value>(self->ref).serialize(true);

  POP_ARRAY_NAME(string) res;
  res.size = json.size();
  res.data = new char[res.size];
  strncpy(res.data, json.c_str(), res.size);

  return res;
}

POP_GENERATE_DEFAULT_VALUE(btc_block) {
  auto* v = new POP_ENTITY_NAME(btc_block);
  v->ref = default_value::generateDefaultValue<altintegration::BtcBlock>();
  return v;
}

namespace default_value {
template <>
altintegration::BtcBlock generateDefaultValue<altintegration::BtcBlock>() {
  altintegration::BtcBlock res;
  res.setNonce(1);
  res.setTimestamp(1);
  res.setVersion(1);
  res.setDifficulty(1);
  res.setPreviousBlock({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
  res.setMerkleRoot({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
  return res;
}
}  // namespace default_value