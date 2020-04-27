using Thrift;

void main()
{
    Socket socket = new Socket("localhost", 9090);
    BufferedTransport transport = new BufferedTransport(socket);
    Protocol protocol = new BinaryProtocol(transport);
    Calculator.Client client = new Calculator.Client(protocol);

    transport.open();

    client.ping();
    stdout.printf("ping()\n-\n");

    int sum = client.add(1, 1);
    stdout.printf(@"1+1=$sum\n-\n");

    Work work = new Work();

    work.Op = Operation.DIVIDE;
    work.Num1 = 1;
    work.Num2 = 0;

    InvalidOperation io;
    int quotient = client.calculate(1, work, out io);
    if (io != null)
    {
        stderr.printf(@"Invalid operation: $(io.Why)\n-\n");
    }
    else
    {
        stdout.printf("Whoa we can divide by 0\n-\n");
    }

    work.Op = Operation.SUBTRACT;
    work.Num1 = 15;
    work.Num2 = 10;

    int diff = client.calculate(1, work, out io);
    if (io != null)
    {
        stderr.printf(@"Invalid operation: $(io.Why)\n-\n");
    }
    else
    {
        stdout.printf(@"15-10=$diff\n-\n");
    }

    SharedStruct log = client.get_struct(1);
    stdout.printf(@"Check log: $(log.Value)\n-\n");

    transport.close();
}
