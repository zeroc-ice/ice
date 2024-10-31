// Copyright (c) ZeroC, Inc.

using Test;

public class AllTests : Test.AllTests
{
    public static void allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        TestIntfPrx service1 = TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                          helper.getTestEndpoint(0)));
        TestIntfPrx service2 = TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                          helper.getTestEndpoint(1)));
        TestIntfPrx service3 = TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                          helper.getTestEndpoint(2)));
        TestIntfPrx service4 = TestIntfPrxHelper.uncheckedCast(communicator.stringToProxy("test:" +
                                                                                          helper.getTestEndpoint(3)));

        if (service1.getProperty("TestInheritProperties").Length == 0)
        {
            Console.Out.Write("testing service properties... ");
            Console.Out.Flush();

            test(service1.getProperty("Ice.ProgramName") == "IceBox-Service1");
            test(service1.getProperty("Service") == "1");
            test(service1.getProperty("Service1.Ovrd") == "2");
            test(service1.getProperty("Service1.Unset").Length == 0);
            test(service1.getProperty("Arg") == "1");

            string[] args1 = { "-a", "--Arg=2" };
            test(Enumerable.SequenceEqual(service1.getArgs(), args1));

            test(service2.getProperty("Ice.ProgramName") == "Test");
            test(service2.getProperty("Service") == "2");
            test(service2.getProperty("Service1.ArgProp").Length == 0);

            string[] args2 = { "--Service1.ArgProp=1" };
            test(Enumerable.SequenceEqual(service2.getArgs(), args2));

            Console.Out.WriteLine("ok");

            Console.Out.Write("testing with shared communicator... ");
            Console.Out.Flush();

            test(service3.getProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
            test(service3.getProperty("Service") == "4");
            test(service3.getProperty("Prop").Length == 0);
            test(service3.getProperty("Service3.Prop") == "1");
            test(service3.getProperty("Ice.Trace.Slicing") == "3");

            test(service4.getProperty("Ice.ProgramName") == "IceBox-SharedCommunicator");
            test(service4.getProperty("Service") == "4");
            test(service4.getProperty("Prop").Length == 0);
            test(service4.getProperty("Service3.Prop") == "1");
            test(service4.getProperty("Ice.Trace.Slicing") == "3");

            string[] args4 = { "--Service3.Prop=2" };
            test(Enumerable.SequenceEqual(service4.getArgs(), args4));

            Console.Out.WriteLine("ok");
        }
        else
        {
            Console.Out.Write("testing property inheritance... ");
            Console.Out.Flush();

            test(service1.getProperty("Ice.ProgramName") == "IceBox2-Service1");
            test(service1.getProperty("ServerProp") == "1");
            test(service1.getProperty("OverrideMe") == "2");
            test(service1.getProperty("UnsetMe").Length == 0);
            test(service1.getProperty("Service1.Prop") == "1");
            test(service1.getProperty("Service1.ArgProp") == "2");

            test(service2.getProperty("Ice.ProgramName") == "IceBox2-SharedCommunicator");
            test(service2.getProperty("ServerProp") == "1");
            test(service2.getProperty("OverrideMe") == "3");
            test(service2.getProperty("UnsetMe").Length == 0);
            test(service2.getProperty("Service2.Prop") == "1");

            Console.Out.WriteLine("ok");
        }
    }
}
