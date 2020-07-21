//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using ZeroC.Ice;
using Test;

namespace ZeroC.IceBox.Test.Configuration
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var service1 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(0)}", communicator);
            var service2 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(1)}", communicator);
            var service3 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(2)}", communicator);
            var service4 = ITestIntfPrx.Parse($"test:{helper.GetTestEndpoint(3)}", communicator);

            if (service1.GetProperty("IceBox.InheritProperties").Length == 0)
            {
                Console.Out.Write("testing service properties... ");
                Console.Out.Flush();

                TestHelper.Assert(service1.GetProperty("Ice.ProgramName") == "IceBox-Service1");
                TestHelper.Assert(service1.GetProperty("Service") == "1");
                TestHelper.Assert(service1.GetProperty("Service1.Ovrd") == "2");
                TestHelper.Assert(service1.GetProperty("Service1.Unset").Length == 0);
                TestHelper.Assert(service1.GetProperty("Arg") == "1");

                string[] args1 = { "-a", "--Arg=2" };
                TestHelper.Assert(service1.GetArgs().SequenceEqual(args1));

                TestHelper.Assert(service2.GetProperty("Ice.ProgramName") == "Test");
                TestHelper.Assert(service2.GetProperty("Service") == "2");
                TestHelper.Assert(service2.GetProperty("Service1.ArgProp").Length == 0);
                TestHelper.Assert(service2.GetProperty("IceBox.InheritProperties") == "1");

                string[] args2 = { "--Service1.ArgProp=1" };
                TestHelper.Assert(service2.GetArgs().SequenceEqual(args2));

                Console.Out.WriteLine("ok");

                Console.Out.Write("testing with shared communicator... ");
                Console.Out.Flush();

                TestHelper.Assert(service3.GetProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
                TestHelper.Assert(service3.GetProperty("Service") == "4");
                TestHelper.Assert(service3.GetProperty("Prop").Length == 0);
                TestHelper.Assert(service3.GetProperty("Service3.Prop") == "1");
                TestHelper.Assert(service3.GetProperty("Ice.Trace.Slicing") == "3");

                TestHelper.Assert(service4.GetProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
                TestHelper.Assert(service4.GetProperty("Service") == "4");
                TestHelper.Assert(service4.GetProperty("Prop").Length == 0);
                TestHelper.Assert(service4.GetProperty("Service3.Prop") == "1");
                TestHelper.Assert(service4.GetProperty("Ice.Trace.Slicing") == "3");

                string[] args4 = { "--Service3.Prop=2" };
                TestHelper.Assert(service4.GetArgs().SequenceEqual(args4));

                Console.Out.WriteLine("ok");
            }
            else
            {
                Console.Out.Write("testing property inheritance... ");
                Console.Out.Flush();

                TestHelper.Assert(service1.GetProperty("Ice.ProgramName") == "IceBox2-Service1");
                TestHelper.Assert(service1.GetProperty("ServerProp") == "1");
                TestHelper.Assert(service1.GetProperty("OverrideMe") == "2");
                TestHelper.Assert(service1.GetProperty("UnsetMe").Length == 0);
                TestHelper.Assert(service1.GetProperty("Service1.Prop") == "1");
                TestHelper.Assert(service1.GetProperty("Service1.ArgProp") == "2");

                TestHelper.Assert(service2.GetProperty("Ice.ProgramName") == "IceBox2-SharedCommunicator");
                TestHelper.Assert(service2.GetProperty("ServerProp") == "1");
                TestHelper.Assert(service2.GetProperty("OverrideMe") == "3");
                TestHelper.Assert(service2.GetProperty("UnsetMe").Length == 0);
                TestHelper.Assert(service2.GetProperty("Service2.Prop") == "1");

                Console.Out.WriteLine("ok");
            }
        }
    }
}
