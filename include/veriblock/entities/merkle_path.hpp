// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
#define ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "veriblock/consts.hpp"
#include "veriblock/hashutil.hpp"
#include "veriblock/serde.hpp"
#include "veriblock/uint.hpp"

namespace altintegration {

/**
 * @struct MerklePath
 *
 * Path in Merkle tree which proves that `subject` is inside this tree.
 */
struct MerklePath {
  int32_t index{};
  uint256 subject{};
  std::vector<uint256> layers{};

  /**
   * Convert MerklePath to data stream using MerklePath basic byte format
   * @param stream data stream to write into
   */
  void toRaw(WriteStream& stream) const;

  /**
   * Convert MerklePath to data stream using MerklePath VBK byte format
   * @param stream data stream to write into
   */
  void toVbkEncoding(WriteStream& stream) const;

  size_t estimateSize() const;

  /**
   * Calculate the hash of the merkle root
   * @return hash merkle root hash
   */
  uint256 calculateMerkleRoot() const;
};

template <typename JsonValue>
JsonValue ToJSON(const MerklePath& m) {
  JsonValue obj = json::makeEmptyObject<JsonValue>();
  json::putIntKV(obj, "index", m.index);
  json::putStringKV(obj, "subject", HexStr(m.subject));
  json::putArrayKV(obj, "layers", m.layers);
  return obj;
}

bool DeserializeFromRaw(ReadStream& stream,
                        const uint256& subject,
                        MerklePath& out,
                        ValidationState& state);

bool DeserializeFromVbkEncoding(ReadStream& stream,
                                const uint256& subject,
                                MerklePath& out,
                                ValidationState& state);

}  // namespace altintegration

#endif  // ALT_INTEGRATION_INCLUDE_VERIBLOCK_ENTITIES_MERKLE_PATH_HPP_
