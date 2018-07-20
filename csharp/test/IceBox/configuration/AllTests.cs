// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
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

        if(service1.getProperty("IceBox.InheritProperties").Equals(""))
        {
            Console.Out.Write("testing service properties... ");
            Console.Out.Flush();

            test(service1.getProperty("Ice.ProgramName").Equals("IceBox-Service1"));
            test(service1.getProperty("Service").Equals("1"));
            test(service1.getProperty("Service1.Ovrd").Equals("2"));
            test(service1.getProperty("Service1.Unset").Equals(""));
            test(service1.getProperty("Arg").Equals("1"));

            string[] args1 = {"-a", "--Arg=2"};
            test(IceUtilInternal.Arrays.Equals(service1.getArgs(), args1));

            test(service2.getProperty("Ice.ProgramName").Equals("Test"));
            test(service2.getProperty("Service").Equals("2"));
            test(service2.getProperty("Service1.ArgProp").Equals(""));
            test(service2.getProperty("IceBox.InheritProperties").Equals("1"));

            string[] args2 = {"--Service1.ArgProp=1"};
            test(IceUtilInternal.Arrays.Equals(service2.getArgs(), args2));

            Console.Out.WriteLine("ok");

            Console.Out.Write("testing with shared communicator... ");
            Console.Out.Flush();

            test(service3.getProperty("Ice.ProgramName").Equals("IceBox-SharedCommunicator"));
            test(service3.getProperty("Service").Equals("4"));
            test(service3.getProperty("Prop").Equals(""));
            test(service3.getProperty("Service3.Prop").Equals("1"));
            test(service3.getProperty("Ice.Trace.Slicing").Equals("3"));

            test(service4.getProperty("Ice.ProgramName").Equals("IceBox-SharedCommunicator"));
            test(service4.getProperty("Service").Equals("4"));
            test(service4.getProperty("Prop").Equals(""));
            test(service4.getProperty("Service3.Prop").Equals("1"));
            test(service4.getProperty("Ice.Trace.Slicing").Equals("3"));

            string[] args4 = {"--Service3.Prop=2"};
            test(IceUtilInternal.Arrays.Equals(service4.getArgs(), args4));

            Console.Out.WriteLine("ok");
        }
        else
        {
            Console.Out.Write("testing property inheritance... ");
            Console.Out.Flush();

            test(service1.getProperty("Ice.ProgramName").Equals("IceBox2-Service1"));
            test(service1.getProperty("ServerProp").Equals("1"));
            test(service1.getProperty("OverrideMe").Equals("2"));
            test(service1.getProperty("UnsetMe").Equals(""));
            test(service1.getProperty("Service1.Prop").Equals("1"));
            test(service1.getProperty("Service1.ArgProp").Equals("2"));

            test(service2.getProperty("Ice.ProgramName").Equals("IceBox2-SharedCommunicator"));
            test(service2.getProperty("ServerProp").Equals("1"));
            test(service2.getProperty("OverrideMe").Equals("3"));
            test(service2.getProperty("UnsetMe").Equals(""));
            test(service2.getProperty("Service2.Prop").Equals("1"));

            Console.Out.WriteLine("ok");
        }
    }
}
