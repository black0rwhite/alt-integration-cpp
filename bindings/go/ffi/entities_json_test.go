// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestJSONBtcBlock(t *testing.T) {
	assert := assert.New(t)

	btc_block := generateDefaultBtcBlock()

	json, err := btc_block.ToJSON()
	assert.NoError(err)

	assert.Equal(json["timestamp"], float64(1))
	assert.Equal(json["bits"], float64(1))
	assert.Equal(json["nonce"], float64(1))
	assert.Equal(json["version"], float64(1))
	assert.Equal(json["merkleRoot"], "0101010101010101010101010101010101010101010101010101010101010101")
	assert.Equal(json["previousBlock"], "0101010101010101010101010101010101010101010101010101010101010101")
	assert.Equal(json["hash"], "164a73ce49bb95df8f547169d5731f069a83714bb431afa09a24e088c9ff051c")
}

func TestJSONVbkBlock(t *testing.T) {
	assert := assert.New(t)

	vbk_block := generateDefaultVbkBlock()

	json, err := vbk_block.ToJSON()
	assert.NoError(err)

	assert.Equal(json["height"], float64(1))
	assert.Equal(json["version"], float64(1))
	assert.Equal(json["timestamp"], float64(1))
	assert.Equal(json["difficulty"], float64(1))
	assert.Equal(json["nonce"], float64(1))
	assert.Equal(json["hash"], "c8431c39104b669d234f54b2a6b76adb581c5352cad81826")
	assert.Equal(json["previousBlock"], "010101010101010101010101")
	assert.Equal(json["previousKeystone"], "010101010101010101")
	assert.Equal(json["secondPreviousKeystone"], "010101010101010101")
	assert.Equal(json["merkleRoot"], "01010101010101010101010101010101")

}

func TestJSONAltBlock(t *testing.T) {
	assert := assert.New(t)

	alt_block := generateDefaultAltBlock()

	json, err := alt_block.ToJSON(true)
	assert.NoError(err)

	assert.Equal(json["hash"], "01010101010101010101010101010101")
	assert.Equal(json["previousBlock"], "02020202020202020202020202020202")
	assert.Equal(json["timestamp"], float64(1))
	assert.Equal(json["height"], float64(1))
}

func TestJSONVtb(t *testing.T) {
	assert := assert.New(t)

	vtb := generateDefaultVtb()

	json_vbk_block, err := generateDefaultVbkBlock().ToJSON()
	assert.NoError(err)

	json, err := vtb.ToJSON()
	assert.NoError(err)

	assert.Equal(json["containingBlock"], json_vbk_block)
}

func TestJSONAtv(t *testing.T) {
	assert := assert.New(t)

	atv := generateDefaultAtv()

	json_vbk_block, err := generateDefaultVbkBlock().ToJSON()
	assert.NoError(err)

	json, err := atv.ToJSON()
	assert.NoError(err)

	assert.Equal(json["blockOfProof"], json_vbk_block)
}