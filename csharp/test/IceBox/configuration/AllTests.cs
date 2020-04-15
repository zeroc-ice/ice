//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

public class AllTests
{
    public static void allTests(TestHelper helper)
    {
        Ice.Communicator? communicator = helper.Communicator();
        TestHelper.Assert(communicator != null);
        var service1 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
        var service2 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(1)}", communicator);
        var service3 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(2)}", communicator);
        var service4 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(3)}", communicator);

        if (service1.getProperty("IceBox.InheritProperties") == "")
        {
            Console.Out.Write("testing service properties... ");
            Console.Out.Flush();

            TestHelper.Assert(service1.getProperty("Ice.ProgramName") == "IceBox-Service1");
            TestHelper.Assert(service1.getProperty("Service") == "1");
            TestHelper.Assert(service1.getProperty("Service1.Ovrd") == "2");
            TestHelper.Assert(service1.getProperty("Service1.Unset") == "");
            TestHelper.Assert(service1.getProperty("Arg") == "1");

            string[] args1 = { "-a", "--Arg=2" };
            TestHelper.Assert(Collections.Equals(service1.getArgs(), args1));

            TestHelper.Assert(service2.getProperty("Ice.ProgramName") == "Test");
            TestHelper.Assert(service2.getProperty("Service") == "2");
            TestHelper.Assert(service2.getProperty("Service1.ArgProp") == "");
            TestHelper.Assert(service2.getProperty("IceBox.InheritProperties") == "1");

            string[] args2 = { "--Service1.ArgProp=1" };
            TestHelper.Assert(Collections.Equals(service2.getArgs(), args2));

            Console.Out.WriteLine("ok");

            Console.Out.Write("testing with shared communicator... ");
            Console.Out.Flush();

            TestHelper.Assert(service3.getProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
            TestHelper.Assert(service3.getProperty("Service") == "4");
            TestHelper.Assert(service3.getProperty("Prop") == "");
            TestHelper.Assert(service3.getProperty("Service3.Prop") == "1");
            TestHelper.Assert(service3.getProperty("Ice.Trace.Slicing") == "3");

            TestHelper.Assert(service4.getProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
            TestHelper.Assert(service4.getProperty("Service") == "4");
            TestHelper.Assert(service4.getProperty("Prop") == "");
            TestHelper.Assert(service4.getProperty("Service3.Prop") == "1");
            TestHelper.Assert(service4.getProperty("Ice.Trace.Slicing") == "3");

            string[] args4 = { "--Service3.Prop=2" };
            TestHelper.Assert(Collections.Equals(service4.getArgs(), args4));

            Console.Out.WriteLine("ok");
        }
        else
        {
            Console.Out.Write("testing property inheritance... ");
            Console.Out.Flush();

            TestHelper.Assert(service1.getProperty("Ice.ProgramName") == "IceBox2-Service1");
            TestHelper.Assert(service1.getProperty("ServerProp") == "1");
            TestHelper.Assert(service1.getProperty("OverrideMe") == "2");
            TestHelper.Assert(service1.getProperty("UnsetMe") == "");
            TestHelper.Assert(service1.getProperty("Service1.Prop") == "1");
            TestHelper.Assert(service1.getProperty("Service1.ArgProp") == "2");

            TestHelper.Assert(service2.getProperty("Ice.ProgramName") == "IceBox2-SharedCommunicator");
            TestHelper.Assert(service2.getProperty("ServerProp") == "1");
            TestHelper.Assert(service2.getProperty("OverrideMe") == "3");
            TestHelper.Assert(service2.getProperty("UnsetMe") == "");
            TestHelper.Assert(service2.getProperty("Service2.Prop") == "1");

            Console.Out.WriteLine("ok");
        }
    }
}
