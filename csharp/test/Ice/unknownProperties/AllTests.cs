// Copyright (c) ZeroC, Inc.

namespace Ice.unknownProperties;

public class AllTests : Test.AllTests
{
    public static int allTests(Test.TestHelper helper)
    {
        var communicator = helper.communicator();
        var output = helper.getWriter();

        output.Write("Creating object adapter with unknown properties fails... ");
        communicator.getProperties().setProperty("Foo.Endpoints", "tcp -h 127.0.0.1");
        communicator.getProperties().setProperty("Foo.UnknownProperty", "bar");
        try
        {
            communicator.createObjectAdapter("Foo");
            test(false);
        }
        catch (UnknownPropertyException)
        {
        }
        output.WriteLine("ok");

        output.Write("Creating proxy with unknown properties fails... ");

        communicator.getProperties().setProperty("Greeter", "hello:tcp -h 127.0.0.1 -p 10000");
        communicator.getProperties().setProperty("Greeter.UnknownProperty", "bar");
        try
        {
            communicator.propertyToProxy("Greeter");
            test(false);
        }
        catch (UnknownPropertyException)
        {
        }
        output.WriteLine("ok");
        return 0;
    }
}
