type
  TTransport* = ref object of RootObj

method write*(transport: TTransport, buf: openArray[byte]) {.base.} =
  raise newException(CatchableError, "Method without implementation override")