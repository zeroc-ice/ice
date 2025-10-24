// Copyright (c) ZeroC, Inc.

using System.Globalization;
using System.Text;

namespace Ice.udp;

public class Server : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");

        using Communicator communicator = initialize(properties);
        int num = 0;
        try
        {
            num = args.Length == 1 ? int.Parse(args[0], CultureInfo.InvariantCulture) : 0;
        }
        catch (FormatException)
        {
        }

        communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
        adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("control"));
        adapter.activate();
        serverReady();
        if (num == 0)
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter");
            adapter2.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
            adapter2.activate();
        }

        var endpoint = new StringBuilder();
        //
        // Use loopback to prevent other machines to answer.
        //
        if (properties.getIceProperty("Ice.IPv6") == "1")
        {
            endpoint.Append("udp -h \"ff15::1:1\"");
        }
        else
        {
            endpoint.Append("udp -h 239.255.1.1");
        }
        endpoint.Append(" -p ");
        endpoint.Append(getTestPort(properties, 10));
        communicator.getProperties().setProperty("McastTestAdapter.Endpoints", endpoint.ToString());
        Ice.ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
        mcastAdapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        mcastAdapter.activate();

        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Server>(args);
}
