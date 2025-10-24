// Copyright (c) ZeroC, Inc.

using System.Globalization;

namespace Ice.udp;

public class Client : global::Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.SndSize", "16384");
        await using Communicator communicator = initialize(properties);

        await AllTests.allTests(this);

        int num;
        try
        {
            num = args.Length == 1 ? int.Parse(args[0], CultureInfo.InvariantCulture) : 1;
        }
        catch (FormatException)
        {
            num = 1;
        }

        for (int i = 0; i < num; ++i)
        {
            ObjectPrx prx = communicator.stringToProxy("control:" + getTestEndpoint(i, "tcp"));
            Test.TestIntfPrxHelper.uncheckedCast(prx).shutdown();
        }
    }

    public static Task<int> Main(string[] args) => global::Test.TestDriver.runTestAsync<Client>(args);
}
