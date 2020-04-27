using Thrift;

public class CalculatorHandler : ICalculator, SharedService
{
    HashTable<int, SharedStruct> log;

    public CalculatorHandler()
    {
        log = new HashTable<int, SharedStruct>(int_hash, int_equal);
    }

    public void ping()
    {
        stdout.printf("ping()\n");
    }

    public int32 add(int32 n1, int32 n2)
    {
        stdout.printf(@"add($n1,$n2)\n");
        return n1 + n2;
    }

    public int32 calculate(int32 logid, Work work, out InvalidOperation ouch)
    {
        ouch = null;
        stdout.printf(@"calculate($logid, [$(work.Op),$(work.Num1),$(work.Num2)])\n");
        int val = 0;
        switch (work.Op)
        {
            case Operation.ADD:
                val = work.Num1 + work.Num2;
                break;

            case Operation.SUBTRACT:
                val = work.Num1 - work.Num2;
                break;

            case Operation.MULTIPLY:
                val = work.Num1 * work.Num2;
                break;

            case Operation.DIVIDE:
                if (work.Num2 == 0)
                {
                    ouch = new InvalidOperation();
                    ouch.WhatOp = work.Op;
                    ouch.Why = "Cannot divide by 0";
                    break;
                }
                val = work.Num1 / work.Num2;
                break;

            default:
                {
                    ouch = new InvalidOperation();
                    ouch.WhatOp = work.Op;
                    ouch.Why = "Unknown operation";
                    break;
                }
        }

        SharedStruct entry = new SharedStruct();
        entry.Key = logid;
        entry.Value = val.to_string();
        log[logid] = entry;

        return val;
    }

    public SharedStruct getStruct(int key)
    {
        stdout.printf("getStruct($key)\n");
        return log[key];
    }

    public void zip()
    {
        stdout.printf("zip()\n");
    }
}

void main()
{
    try
    {
        CalculatorHandler handler = new CalculatorHandler();
        Calculator.Processor processor = new Calculator.Processor(handler);
        ServerTransport server_transport = new ServerSocket(9090);
        BufferedTransportFactory transport_factory = new BufferedTransportFactory();
        BinaryProtocolFactory protocol_factory = new BinaryProtocolFactory();
        Server server = new SimpleServer.with_factories(processor, server_transport, transport_factory, protocol_factory);

        stdout.printf("Starting the server...\n");
        server.serve();
    }
    catch (Error e)
    {
        stderr.printf(@"$(e.message)\n");
    }
    stdout.printf("done.\n");
}
