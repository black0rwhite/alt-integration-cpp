package entities

import "io"

// Output ...
type Output struct {
	Address Address
	Coin    Coin
}

// ToVbkEncoding ...
func (v *Output) ToVbkEncoding(stream io.Writer) error {
	err := v.Address.ToVbkEncoding(stream)
	if err != nil {
		return err
	}
	return v.Coin.ToVbkEncoding(stream)
}

// FromVbkEncoding ...
func (v *Output) FromVbkEncoding(stream io.Reader) error {
	if err := v.Address.FromVbkEncoding(stream); err != nil {
		return err
	}
	return v.Coin.FromVbkEncoding(stream)
}