import "nativesockets"
import "../protocol"
import "../transport"

type
  TBinaryProtocol* = ref object of TProtocol
    strictWrite: bool

method writeMessageBegin(protocol: TBinaryProtocol, name: string, messageType: TMessageType, seqid: int32) =
  if protocol.strictWrite:
    let version = VERSION_1 or messageType
    protocol.writeI32(version)
    protocol.writeString(name)
    protocol.writeI32(seqid)
  else:
    protocol.writeString(name)
    protocol.writeByte(messageType)
    protocol.writeI32(seqid)

method writeMessageEnd(protocol: TBinaryProtocol) =

method writeStructBegin(protocol: TBinaryProtocol, name: string) =

method writeStructEnd(protocol: TBinaryProtocol) =

method writeFieldBegin(protocol: TBinaryProtocol, name: string, fieldType: TType, fieldId: int16) =
  protocol.writeByte(fieldType)
  protocol.writeI16(fieldId)

method writeFieldEnd(protocol: TBinaryProtocol) =

method writeFieldStop(protocol: TBinaryProtocol) =
  protocol.writeByte(TType.STOP)

method writeMapBegin(protocol: TBinaryProtocol, keyType: TType, valueType: TType, size: int32) =
  protocol.writeByte(keyType)
  protocol.writeByte(valueType)
  protocol.writeI32(size)

method writeMapEnd(protocol: TBinaryProtocol) =

method writeListBegin(protocol: TBinaryProtocol, elementType: TType, size: int32) =
  protocol.writeByte(elementType)
  protocol.writeI32(size)

method writeListEnd(protocol: TBinaryProtocol) =

method writeSetBegin(protocol: TBinaryProtocol, elementType: TType, size: int32) =
  protocol.writeByte(elementType)
  protocol.writeI32(size)

method writeSetEnd(protocol: TBinaryProtocol) =

method writeBool(protocol: TBinaryProtocol, value: bool) =
  protocol.writeByte(value ? 1 : 0)

method writeByte(protocol: TBinaryProtocol, value: int8) =
  let bytes = cast[array[1, byte]](value)
  protocol.transport.write(bytes)

method writeI16(protocol: TBinaryProtocol, value: int16) =
  let net = ntohs(cast[uint16](value))
  let bytes = cast[array[0..1, byte]](net)
  protocol.transport.write(bytes)

method writeI32(protocol: TBinaryProtocol, value: int32) =
  let net = ntohl(value)
  let bytes = cast[array[0..3, byte]](net)
  protocol.transport.write(bytes)

method writeI64(protocol: TBinaryProtocol, value: int64) =
  let net = ntohll(value)
  let bytes = cast[array[0..7, byte]](net)
  protocol.transport.write(bytes)

method writeDouble(protocol: TBinaryProtocol, value: float64) =
  let net = ntohll(cast[uint64](value))
  let bytes = cast[array[0..7, byte]](net)
  protocol.transport.write(bytes)

method writeString(protocol: TBinaryProtocol, value: string) =
  protocol.transport.write(value)

method writeBinary(protocol: TBinaryProtocol, value: openArray[int8]) =
  protocol.transport.write(value)

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

method readMapBegin(protocol: TBinaryProtocol, keyType: var TType, valueType: var TType, var size: uint32) =
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

method readBool(protocol: TBinaryProtocol): bool =
  raise newException(CatchableError, "Method without implementation override")

method readByte(protocol: TBinaryProtocol): int8 =
  raise newException(CatchableError, "Method without implementation override")

method readI16(protocol: TBinaryProtocol): int16 =
  raise newException(CatchableError, "Method without implementation override")

method readI32(protocol: TBinaryProtocol): int32 =
  raise newException(CatchableError, "Method without implementation override")

method readI64(protocol: TBinaryProtocol): int64 =
  raise newException(CatchableError, "Method without implementation override")

method readDouble(protocol: TBinaryProtocol): float64 =
  raise newException(CatchableError, "Method without implementation override")

method readString(protocol: TBinaryProtocol): string =
  raise newException(CatchableError, "Method without implementation override")

method readBinary(protocol: TBinaryProtocol): openArray[int8] =
  raise newException(CatchableError, "Method without implementation override")
