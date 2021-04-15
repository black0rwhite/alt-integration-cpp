// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_C_MOCK_MINER2_HPP
#define VERIBLOCK_POP_CPP_C_MOCK_MINER2_HPP

#include "veriblock/pop/c/mock_miner2.h"
#include "veriblock/pop/mock_miner.hpp"

struct __pop_mock_miner {
  altintegration::MockMiner ref;
};

#endif