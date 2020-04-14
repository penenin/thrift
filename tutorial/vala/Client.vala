using Thrift;

void main()
{
    Transport transport = null;

    try
    {
        transport = new Socket("localhost", 9090);
        Protocol protocol = new BinaryProtocol(transport);
        Calculator.Client client = new Calculator.Client(protocol);

        transport.open();

        client.ping();
        stdout.printf("ping()\n");

        int sum = client.add(1, 1);
        stdout.printf(@"1+1=$sum\n");

        Work work = new Work();

        work.Op = Operation.DIVIDE;
        work.Num1 = 1;
        work.Num2 = 0;

        InvalidOperation io;
        int quotient = client.calculate(1, work, out io);
        if (io != null)
        {
            stderr.printf(@"Invalid operation: $(io.Why)\n");
        }
        else
        {
            stdout.printf("Whoa we can divide by 0\n");
        }

        work.Op = Operation.SUBTRACT;
        work.Num1 = 15;
        work.Num2 = 10;

        int diff = client.calculate(1, work, out io);
        if (io != null)
        {
            stderr.printf(@"Invalid operation: $(io.Why)\n");
        }
        else
        {
            stdout.printf(@"15-10=$diff\n");
        }

        SharedStruct log = client.get_struct(1);
        stdout.printf(@"Check log: $(log.Value)\n");
    }
    catch (Error e)
    {
        stderr.printf(@"$(e.message)\n");
    }
    finally
    {
        try
        {
            transport.close();
        }
        catch {}
    }
}
