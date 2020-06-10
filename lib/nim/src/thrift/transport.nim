type
  TTransport* = ref object of RootObj

method read*(transport: TTransport, buf: openArray[int8]) {.base.} =
  raise newException(CatchableError, "Method without implementation override")

method write*(transport: TTransport, buf: openArray[int8]) {.base.} =
  raise newException(CatchableError, "Method without implementation override")