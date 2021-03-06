// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/mock_miner.hpp>

using namespace boost::python;
using namespace altintegration;

struct Payloads {
  ATV atv;
  std::vector<VTB> vtbs;
  std::vector<VbkBlock> context;

  std::string toPrettyString() const {
    return fmt::sprintf("Payloads(atv, vtbs=%d)", vtbs.size());
  }

  list prepare() const {
    list args;

    list listctx;
    list listvtbs;
    for (const auto& vtb : vtbs) {
      listvtbs.append(HexStr(vtb.toVbkEncoding()));
    }

    for (const auto& b : context) {
      listctx.append(HexStr(b.toVbkEncoding()));
    }

    list listatvs;
    listatvs.append(HexStr(atv.toVbkEncoding()));

    args.append(listctx);
    args.append(listvtbs);
    args.append(listatvs);
    return args;
  }
};

struct MockMinerProxy : private MockMiner {
  using base = MockMiner;

  std::string toPrettyString() {
    std::ostringstream ss;
    ss << "MockMiner(btc=" << this->getBtcTip().toPrettyString();
    ss << ", vbk=" << this->getVbkTip().toPrettyString();
    ss << ")";
    return ss.str();
  }

  VbkBlock getVbkTip() { return vbk().getBestChain().tip()->getHeader(); }
  BtcBlock getBtcTip() { return btc().getBestChain().tip()->getHeader(); }

  VbkBlock getVbkBlock(const std::string& hash) {
    auto index = vbk().getBlockIndex(VbkBlock::hash_t::fromHex(hash));
    if (!index) {
      throw std::invalid_argument("Can not find VbkBlock " + hash);
    }

    return index->getHeader();
  }

  BtcBlock getBtcBlock(const std::string& hash) {
    auto index = btc().getBlockIndex(BtcBlock::hash_t::fromHex(hash));
    if (!index) {
      throw std::invalid_argument("Can not find BtcBlock " + hash);
    }

    return index->getHeader();
  }

  VbkBlock mineVbkBlocks(const std::string& prevHash, size_t num) {
    auto* index = vbk().getBlockIndex(VbkBlock::hash_t::fromHex(prevHash));
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHash);
    }

    return base::mineVbkBlocks(num, *index)->getHeader();
  }
  BtcBlock mineBtcBlocks(const std::string& prevHash, size_t num) {
    auto* index = btc().getBlockIndex(BtcBlock::hash_t::fromHex(prevHash));
    if (!index) {
      throw std::logic_error("MockMiner: can't find prev block for mining: " +
                             prevHash);
    }

    return base::mineBtcBlocks(num, *index)->getHeader();
  }
  VbkBlock mineVbkBlocks(size_t num) {
    auto* index = vbk().getBestChain().tip();
    return base::mineVbkBlocks(num, *index)->getHeader();
  }
  BtcBlock mineBtcBlocks(size_t num) {
    auto* index = btc().getBestChain().tip();
    return base::mineBtcBlocks(num, *index)->getHeader();
  }

  BtcTx createBtcTxEndorsingVbkBlock(const VbkBlock& publishedBlock) {
    return base::createBtcTxEndorsingVbkBlock(publishedBlock);
  }

  VbkPopTx createVbkPopTxEndorsingVbkBlock(
      const BtcBlock& containingBlock,
      const BtcTx& containingTx,
      const VbkBlock& publishedBlock,
      const BtcBlock::hash_t& lastKnownBtcBlockHash) {
    return base::createVbkPopTxEndorsingVbkBlock(
        containingBlock, containingTx, publishedBlock, lastKnownBtcBlockHash);
  }

  VbkTx createVbkTxEndorsingAltBlock(const PublicationData& publicationData) {
    return base::createVbkTxEndorsingAltBlock(publicationData);
  }

  void endorseVbkBlock(const VbkBlock& block,
                       const std::string& lastKnownBtcHash,
                       size_t vtbs = 1) {
    return endorseVbkBlock(block,
                           getVbkTip().getHash().toHex(),
                           getBtcTip().getHash().toHex(),
                           lastKnownBtcHash,
                           vtbs);
  }

  void endorseVbkBlock(const VbkBlock& block,
                       const std::string& prevVbk,
                       const std::string& prevBtc,
                       const std::string& lastKnownBtcHash,
                       size_t vtbs = 1) {
    auto* prevVbkIndex =
        vbk().getBlockIndex(VbkBlock::hash_t::fromHex(prevVbk));
    if (!prevVbkIndex) {
      throw std::logic_error(
          "MockMiner: endorseVbkBlock - can not find prevVbkIndex: " + prevVbk);
    }

    auto* prevBtcIndex =
        btc().getBlockIndex(BtcBlock::hash_t::fromHex(prevBtc));
    if (!prevBtcIndex) {
      throw std::logic_error(
          "MockMiner: envdorseVbkBlock - can not find prevBtcIndex: " +
          prevBtc);
    }

    std::vector<VbkPopTx> popTransactions(vtbs);
    for (size_t i = 0; i < vtbs; i++) {
      auto btctx = base::createBtcTxEndorsingVbkBlock(block);
      prevBtcIndex = base::mineBtcBlocks(1, *prevBtcIndex, {btctx});
      popTransactions[i] = base::createVbkPopTxEndorsingVbkBlock(
          prevBtcIndex->getHeader(),
          btctx,
          block,
          BtcBlock::hash_t::fromHex(lastKnownBtcHash));
    }

    base::mineVbkBlocks(1, *prevVbkIndex, popTransactions);
  }

  Payloads endorseAltBlock(const PublicationData& pub,
                           const std::string& lastVbkBlock) {
    Payloads payloads;
    auto vbkindex =
        vbk().getBlockIndex(VbkBlock::hash_t::fromHex(lastVbkBlock));
    if (!vbkindex) {
      throw std::logic_error(
          "MockMiner: endorseAltBlock called with unknown lastVbkBlock: " +
          lastVbkBlock);
    }
    auto vbktx = base::createVbkTxEndorsingAltBlock(pub);
    auto* vbkblock = base::mineVbkBlocks(1, {vbktx});
    payloads.atv = base::getATVs(*vbkblock)[0];

    std::vector<VbkBlock> context;
    // in range of blocks [lastVbkBlock... vbk tip] look for VTBs and put them
    // into Payloads
    auto vbktip = vbk().getBestChain().tip();
    auto last =
        vbkindex->pprev ? vbkindex->pprev->getHash() : vbkindex->getHash();
    auto& vtbs = payloads.vtbs;
    while (vbktip->getHash() != last) {
      auto it = vbkPayloads.find(vbktip->getHash());
      if (it != vbkPayloads.end()) {
        // insert in reverse order
        std::for_each(it->second.rbegin(),
                      it->second.rend(),
                      [&](const VTB& vtb) { vtbs.push_back(vtb); });
      }
      context.push_back(vbktip->getHeader());
      vbktip = vbktip->pprev;
    }
    std::reverse(vtbs.begin(), vtbs.end());

    std::reverse(context.begin(), context.end());
    payloads.context = context;

    return payloads;
  }
};

boost::shared_ptr<MockMinerProxy> makeMiner() {
  return boost::shared_ptr<MockMinerProxy>(new MockMinerProxy());
}

void init_primitives();
void init_entities();

BOOST_PYTHON_MODULE(pypopminer) {
  init_primitives();
  init_entities();

  class_<Payloads, boost::shared_ptr<Payloads>>("Payloads")
      .def("__repr__", &Payloads::toPrettyString)
      .def("prepare", &Payloads::prepare)
      .def_readonly("atv", &Payloads::atv)
      .def_readonly("vtbs", &Payloads::vtbs)
      .def_readonly("context", &Payloads::context);

  // required to deal with function overloading
  BtcBlock (MockMinerProxy::*fx1)(const std::string&, size_t) =
      &MockMinerProxy::mineBtcBlocks;
  BtcBlock (MockMinerProxy::*fx2)(size_t) = &MockMinerProxy::mineBtcBlocks;
  VbkBlock (MockMinerProxy::*fx3)(const std::string&, size_t) =
      &MockMinerProxy::mineVbkBlocks;
  VbkBlock (MockMinerProxy::*fx4)(size_t) = &MockMinerProxy::mineVbkBlocks;

  void (MockMinerProxy::*fx5)(
      const VbkBlock& block, const std::string& lastKnownBtcHash, size_t) =
      &MockMinerProxy::endorseVbkBlock;
  void (MockMinerProxy::*fx6)(const VbkBlock& block,
                              const std::string& prevVbk,
                              const std::string& prevBtc,
                              const std::string& lastKnownBtcHash,
                              size_t) = &MockMinerProxy::endorseVbkBlock;

  class_<MockMinerProxy, boost::noncopyable, boost::shared_ptr<MockMinerProxy>>(
      "MockMiner", no_init)
      .def("__init__", make_constructor(makeMiner))
      .def("__repr__", &MockMinerProxy::toPrettyString)
      .def_readonly(
          "vbkTip", &MockMinerProxy::getVbkTip, "Current BtcTip in MockMiner")
      .def_readonly(
          "btcTip", &MockMinerProxy::getBtcTip, "Current VbkTip in MockMiner")
      .def("getBtcBlock",
           &MockMinerProxy::getBtcBlock,
           "Given block hash, returns BtcBlock if it exists, throws otherwise")
      .def("getVbkBlock",
           &MockMinerProxy::getVbkBlock,
           "Given block hash, returns VbkBlock if it exists, throws otherwise")
      .def("mineBtcBlocks", fx1)
      .def("mineBtcBlocks", fx2)
      .def("mineVbkBlocks", fx3)
      .def("mineVbkBlocks", fx4)
      .def("createBtcTxEndorsingVbkBlock",
           &MockMinerProxy::createBtcTxEndorsingVbkBlock)
      .def("createVbkPopTxEndorsingVbkBlock",
           &MockMinerProxy::createVbkPopTxEndorsingVbkBlock)
      .def("createVbkTxEndorsingAltBlock",
           &MockMinerProxy::createVbkTxEndorsingAltBlock)
      .def("endorseVbkBlock", fx5)
      .def("endorseVbkBlock", fx6)
      .def("endorseAltBlock", &MockMinerProxy::endorseAltBlock);
}