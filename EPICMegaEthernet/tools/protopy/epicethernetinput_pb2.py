# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: epicethernetinput.proto
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x17\x65picethernetinput.proto\")\n\x0c\x44igitalInput\x12\n\n\x02id\x18\x01 \x01(\x05\x12\r\n\x05value\x18\x02 \x01(\x08\"(\n\x0b\x41nalogInput\x12\n\n\x02id\x18\x01 \x01(\x05\x12\r\n\x05value\x18\x02 \x01(\x02\"V\n\x11\x45picEthernetInput\x12 \n\tdiginputs\x18\x01 \x03(\x0b\x32\r.DigitalInput\x12\x1f\n\tanainputs\x18\x02 \x03(\x0b\x32\x0c.AnalogInputb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'epicethernetinput_pb2', _globals)
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _globals['_DIGITALINPUT']._serialized_start=27
  _globals['_DIGITALINPUT']._serialized_end=68
  _globals['_ANALOGINPUT']._serialized_start=70
  _globals['_ANALOGINPUT']._serialized_end=110
  _globals['_EPICETHERNETINPUT']._serialized_start=112
  _globals['_EPICETHERNETINPUT']._serialized_end=198
# @@protoc_insertion_point(module_scope)
