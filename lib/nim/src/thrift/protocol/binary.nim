import "nativesockets"
import "../protocol"
import "../transport"

const VERSION_1 = 0x80010000'i32

type
  TBinaryProtocol* = ref object of TProtocol
    strictWrite: bool

method writeByte(protocol: TBinaryProtocol, value: int8) =
  let bytes = cast[array[1, int8]](value)
  protocol.transport.write(bytes)

method writeBool(protocol: TBinaryProtocol, value: bool) =
  if value:
    protocol.writeByte(1)
  else:
    protocol.writeByte(0)

method writeI16(protocol: TBinaryProtocol, value: int16) =
  let net = ntohs(cast[uint16](value))
  let bytes = cast[array[2, int8]](net)
  protocol.transport.write(bytes)

method writeI32(protocol: TBinaryProtocol, value: int32) =
  let net = ntohl(cast[uint32](value))
  let bytes = cast[array[4, int8]](net)
  protocol.transport.write(bytes)

method writeI64(protocol: TBinaryProtocol, value: int64) =
  let net = ntohll(cast[uint64](value))
  let bytes = cast[array[8, int8]](net)
  protocol.transport.write(bytes)

method writeDouble(protocol: TBinaryProtocol, value: float64) =
  let net = ntohll(cast[uint64](value))
  let bytes = cast[array[8, int8]](net)
  protocol.transport.write(bytes)

method writeString(protocol: TBinaryProtocol, value: string) =
  protocol.transport.write(cast[seq[int8]](value))

method writeBinary(protocol: TBinaryProtocol, value: openArray[int8]) =
  protocol.transport.write(value)

method writeMessageBegin(protocol: TBinaryProtocol, name: string, messageType: TMessageType, seqid: int32) =
  if protocol.strictWrite:
    let version = VERSION_1 or messageType.int32
    protocol.writeI32(version)
    protocol.writeString(name)
    protocol.writeI32(seqid)
  else:
    protocol.writeString(name)
    protocol.writeByte(messageType.int8)
    protocol.writeI32(seqid)

method writeMessageEnd(protocol: TBinaryProtocol) =
  discard

method writeStructBegin(protocol: TBinaryProtocol, name: string) =
  discard

method writeStructEnd(protocol: TBinaryProtocol) =
  discard

method writeFieldBegin(protocol: TBinaryProtocol, name: string, fieldType: TType, fieldId: int16) =
  protocol.writeByte(fieldType.int8)
  protocol.writeI16(fieldId)

method writeFieldEnd(protocol: TBinaryProtocol) =
  discard

method writeFieldStop(protocol: TBinaryProtocol) =
  protocol.writeByte(TType.STOP.int8)

method writeMapBegin(protocol: TBinaryProtocol, keyType: TType, valueType: TType, size: uint32) =
  protocol.writeByte(keyType.int8)
  protocol.writeByte(valueType.int8)
  protocol.writeI32(size.int32)

method writeMapEnd(protocol: TBinaryProtocol) =
  discard

method writeListBegin(protocol: TBinaryProtocol, elementType: TType, size: uint32) =
  protocol.writeByte(elementType.int8)
  protocol.writeI32(size.int32)

method writeListEnd(protocol: TBinaryProtocol) =
  discard

method writeSetBegin(protocol: TBinaryProtocol, elementType: TType, size: uint32) =
  protocol.writeByte(elementType.int8)
  protocol.writeI32(size.int32)

method writeSetEnd(protocol: TBinaryProtocol) =
  discard

method readByte(protocol: TBinaryProtocol): int8 =
  var 
    buf: array[1, int8]
  protocol.transport.read(buf)
  result = cast[int8](buf)

method readBool(protocol: TBinaryProtocol): bool =
  let value = protocol.readByte()
  if value == 1:
    result = true
  else:
    result = false

method readI16(protocol: TBinaryProtocol): int16 =
  var 
    buf: array[2, int8]
  protocol.transport.read(buf)
  result = cast[int16](buf)

method readI32(protocol: TBinaryProtocol): int32 =
  var 
    buf: array[4, int8]
  protocol.transport.read(buf)
  result = cast[int32](buf)

method readI64(protocol: TBinaryProtocol): int64 =
  var 
    buf: array[8, int8]
  protocol.transport.read(buf)
  result = cast[int64](buf)

method readDouble(protocol: TBinaryProtocol): float64 =
  var 
    buf: array[8, int8]
  protocol.transport.read(buf)
  result = cast[float64](buf)

method readString(protocol: TBinaryProtocol): string =
  raise newException(CatchableError, "Method without implementation override")

method readBinary(protocol: TBinaryProtocol): seq[int8] =
  raise newException(CatchableError, "Method without implementation override")

method readMessageBegin(protocol: TBinaryProtocol, name: var string, messageType: var TMessageType, seqid: var int32) =
  raise newException(CatchableError, "Method without implementation override")

method readMessageEnd(protocol: TBinaryProtocol) =
  raise newException(CatchableError, "Method without implementation override")

method readStructBegin(protocol: TBinaryProtocol, name: var string) =
  raise newException(CatchableError, "Method without implementation override")

method readStructEnd(protocol: TBinaryProtocol) =
  raise newException(CatchableError, "Method without implementation override")

method readFieldBegin(protocol: TBinaryProtocol, name: var string, fieldType: var TType, fieldId: var int16) =
  raise newException(CatchableError, "Method without implementation override")

method readFieldEnd(protocol: TBinaryProtocol) =
  raise newException(CatchableError, "Method without implementation override")

method readFieldStop(protocol: TBinaryProtocol) =
  raise newException(CatchableError, "Method without implementation override")

method readMapBegin(protocol: TBinaryProtocol, keyType: var TType, valueType: var TType, size: var uint32) =
  raise newException(CatchableError, "Method without implementation override")

method readMapEnd(protocol: TBinaryProtocol) =
  raise newException(CatchableError, "Method without implementation override")

method readListBegin(protocol: TBinaryProtocol, elementType: var TType, size: var uint32) =
  raise newException(CatchableError, "Method without implementation override")

method readListEnd(protocol: TBinaryProtocol) =
  raise newException(CatchableError, "Method without implementation override")

method readSetBegin(protocol: TBinaryProtocol, elementType: var TType, size: var uint32) =
  raise newException(CatchableError, "Method without implementation override")

method readSetEnd(protocol: TBinaryProtocol) =
  raise newException(CatchableError, "Method without implementation override")
