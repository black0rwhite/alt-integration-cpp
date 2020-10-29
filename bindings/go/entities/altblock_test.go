package entities

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

var defaultAltBlock = AltBlock{
	parseHex("1fec8aa4983d69395010e4d18cd8b943749d5b4f575e88a375debdc5ed22531c"),
	[]byte{},
	156,
	1466,
}

func TestAltBlockRoundTrip(t *testing.T) {
	assert := assert.New(t)

	outputStream := new(bytes.Buffer)
	assert.NoError(defaultAltBlock.ToVbkEncoding(outputStream))
	stream := bytes.NewReader(outputStream.Bytes())
	deserializedBlock := AltBlock{}
	assert.NoError(deserializedBlock.FromVbkEncoding(stream))

	assert.Equal(defaultAltBlock.Hash, deserializedBlock.Hash)
	assert.Equal(defaultAltBlock.Height, deserializedBlock.Height)
	assert.Equal(defaultAltBlock.Timestamp, deserializedBlock.Timestamp)
}