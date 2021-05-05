// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/entities/popdata.h>
import "C"
import (
	"encoding/json"
	"runtime"
)

type PopData struct {
	ref *C.pop_pop_data_t
}

func generateDefaultPopData() *PopData {
	return createPopData(C.pop_pop_data_generate_default_value())
}

func createPopData(ref *C.pop_pop_data_t) *PopData {
	val := &PopData{ref: ref}
	runtime.SetFinalizer(val, func(v *PopData) {
		v.Free()
	})
	return val
}

func (v *PopData) Free() {
	if v.ref != nil {
		C.pop_pop_data_free(v.ref)
		v.ref = nil
	}
}

func (v *PopData) GetAtvs() []*Atv {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	array := C.pop_pop_data_get_atvs(v.ref)
	defer freeArrayAtv(&array)
	return createArrayAtv(&array)
}

func (v *PopData) GetVtbs() []*Vtb {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	array := C.pop_pop_data_get_vtbs(v.ref)
	defer freeArrayVtb(&array)
	return createArrayVtb(&array)
}

func (v *PopData) GetContext() []*VbkBlock {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	array := C.pop_pop_data_get_context(v.ref)
	defer freeArrayVbkBlock(&array)
	return createArrayVbkBlock(&array)
}

func (v *PopData) ToJSON(verbosity bool) (map[string]interface{}, error) {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	str := C.pop_pop_data_to_json(v.ref, C.bool(verbosity))
	defer freeArrayChar(&str)
	json_str := createString(&str)

	var res map[string]interface{}
	err := json.Unmarshal([]byte(json_str), &res)
	return res, err
}

func (v *PopData) SerializeToVbk() []byte {
	if v.ref == nil {
		panic("PopData does not initialized")
	}
	res := C.pop_pop_data_serialize_to_vbk(v.ref)
	defer freeArrayU8(&res)
	return createBytes(&res)
}

func DeserializeFromVbkPopData(bytes []byte) (*PopData, error) {
	state := NewValidationState2()
	defer state.Free()

	res := C.pop_pop_data_deserialize_from_vbk(createCBytes(bytes), state.ref)
	if res == nil {
		return nil, state.Error()
	}

	return createPopData(res), nil
}