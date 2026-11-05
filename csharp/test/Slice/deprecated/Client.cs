// Copyright (c) ZeroC, Inc.

using DeprecatedTest;
using Test;

namespace Slice.deprecated;

// Make sure we can implement a deprecated interface without warning.
public class MyInterfaceI : MyInterfaceDisp_
{
    public override void opInt(int i, Ice.Current current)
    {
    }
}

public class Client : TestHelper
{
    private static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing deprecated definitions... ");
        Console.Out.Flush();

        // MyOtherInterfacePrx is not deprecated
        MyOtherInterfacePrx prx = MyOtherInterfacePrxHelper.createProxy(communicator, "foo");

        try
        {
            prx.opInt(5); // non-deprecated op
            test(false);
        }
        catch (Ice.NoEndpointException)
        {
            // expected
        }

        Console.Out.WriteLine("ok");
    }

    public override void run(string[] args)
    {
        using Ice.Communicator communicator = initialize(ref args);
        allTests(communicator);
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
