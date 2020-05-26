import "nativesockets"
import "transport"

type
  TType* = enum
    T_STOP = 0,
    T_VOID = 1,
    T_BOOL = 2,
    T_BYTE = 3,
    T_DOUBLE = 4,
    T_I16 = 6,
    T_I32 = 8,
    T_U64 = 9,
    T_I64 = 10,
    T_STRING = 11,
    T_STRUCT = 12,
    T_MAP = 13,
    T_SET = 14,
    T_LIST = 15

  TMessageType* = enum
    T_CALL = 1,
    T_REPLY = 2,
    T_EXCEPTION = 3,
    T_ONEWAY = 4

  TProtocol* = ref object of RootObj
    transport*: TTransport

method writeMessageBegin(protocol: TProtocol, name: string, messageType: TMessageType, seqid: int32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeMessageEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeStructBegin(protocol: TProtocol, name: string) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeStructEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeFieldBegin(protocol: TProtocol, name: string, fieldType: TType, fieldId: int16) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeFieldEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeFieldStop(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeMapBegin(protocol: TProtocol, keyType: TType, valueType: TType, size: uint32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeMapEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeListBegin(protocol: TProtocol, elementType: TType, size: uint32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeListEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeSetBegin(protocol: TProtocol, elementType: TType, size: uint32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeSetEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeBool(protocol: TProtocol, value: bool) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeByte(protocol: TProtocol, value: int8) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeI16(protocol: TProtocol, value: int16) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeI32(protocol: TProtocol, value: int32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeI64(protocol: TProtocol, value: int64) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeDouble(protocol: TProtocol, value: float64) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeString(protocol: TProtocol, value: string) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method writeBinary(protocol: TProtocol, value: openArray[int8]) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readMessageBegin(protocol: TProtocol, name: var string, messageType: var TMessageType, seqid: var int32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readMessageEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readStructBegin(protocol: TProtocol, name: var string) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readStructEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readFieldBegin(protocol: TProtocol, name: var string, fieldType: var TType, fieldId: var int16) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readFieldEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readFieldStop(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readMapBegin(protocol: TProtocol, keyType: var TType, valueType: var TType, size: var uint32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readMapEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readListBegin(protocol: TProtocol, elementType: var TType, size: var uint32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readListEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readSetBegin(protocol: TProtocol, elementType: var TType, size: var uint32) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readSetEnd(protocol: TProtocol) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readBool(protocol: TProtocol): bool {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readByte(protocol: TProtocol): int8 {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readI16(protocol: TProtocol): int16 {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readI32(protocol: TProtocol): int32 {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readI64(protocol: TProtocol): int64 {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readDouble(protocol: TProtocol): float64 {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readString(protocol: TProtocol): string {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method readBinary(protocol: TProtocol): openArray[int8] {.base.} =
  raise newException(CatchableError, "Method without implementation override")

proc ntohll*(x: uint64): uint64 =
  when cpuEndian == bigEndian: result = x
  else: result = (ntohl(cast[uint32](x and 0xffffffff'u64)) shl 32) or (ntohl(cast[uint32](x shr 32)))

proc htonll*(x: uint64): uint64 =
  when cpuEndian == bigEndian: result = x
  else: ntohll(x)