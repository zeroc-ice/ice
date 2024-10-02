// Copyright (c) ZeroC, Inc.

using System.Collections;
using System.Diagnostics;

namespace Ice.unknownProperties;

public class AllTests : global::Test.AllTests
{
    public static int allTests(global::Test.TestHelper helper)
    {
        var communicator = helper.communicator();
        var output = helper.getWriter();

        output.Write("Creating object adapter with unknown properties... ");
        communicator.getProperties().setProperty("Foo.Endpoints", "tcp -h 127.0.0.1");
        communicator.getProperties().setProperty("Foo.UnknownProperty", "bar");
        try
        {
            communicator.createObjectAdapter("Foo");
            AllTests.test(false);
        }
        catch(InitializationException)
        {
        }
        output.WriteLine("ok");

        output.Write("Creating proxy with unknown properties... ");

        communicator.getProperties().setProperty("Greeter", "hello:tcp -h 127.0.0.1 -p 10000");
        communicator.getProperties().setProperty("Greeter.UnknownProperty", "bar");
        try
        {
            communicator.propertyToProxy("Greeter");
            AllTests.test(false);
        }
        catch(InitializationException)
        {
        }
        output.WriteLine("ok");
        return 0;
    }
}
