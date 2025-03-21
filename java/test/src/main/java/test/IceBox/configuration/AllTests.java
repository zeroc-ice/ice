// Copyright (c) ZeroC, Inc.

package test.IceBox.configuration;

import test.IceBox.configuration.Test.TestIntfPrx;

import java.io.PrintWriter;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper) {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        var service1 = TestIntfPrx.createProxy(communicator, "test:" + helper.getTestEndpoint(0));
        var service2 = TestIntfPrx.createProxy(communicator, "test:" + helper.getTestEndpoint(1));
        var service3 = TestIntfPrx.createProxy(communicator, "test:" + helper.getTestEndpoint(2));
        var service4 = TestIntfPrx.createProxy(communicator, "test:" + helper.getTestEndpoint(3));

        if (service1.getProperty("TestInheritProperties").isEmpty()) {
            out.print("testing service properties... ");
            out.flush();

            test(service1.getProperty("Ice.ProgramName").equals("IceBox-Service1"));
            test(service1.getProperty("Service").equals("1"));
            test(service1.getProperty("Service1.Ovrd").equals("2"));
            test(service1.getProperty("Service1.Unset").isEmpty());
            test(service1.getProperty("Arg").equals("1"));

            String[] args1 = {"-a", "--Arg=2"};
            test(java.util.Arrays.equals(service1.getArgs(), args1));

            test(service2.getProperty("Ice.ProgramName").equals("Test"));
            test(service2.getProperty("Service").equals("2"));
            test(service2.getProperty("Service1.ArgProp").isEmpty());

            String[] args2 = {"--Service1.ArgProp=1"};
            test(java.util.Arrays.equals(service2.getArgs(), args2));

            out.println("ok");

            out.print("testing with shared communicator... ");
            out.flush();

            test(service3.getProperty("Ice.ProgramName").equals("IceBox-SharedCommunicator"));
            test(service3.getProperty("Service").equals("4"));
            test(service3.getProperty("Prop").isEmpty());
            test(service3.getProperty("Service3.Prop").equals("1"));
            test(service3.getProperty("Ice.Trace.Slicing").equals("3"));

            test(service4.getProperty("Ice.ProgramName").equals("IceBox-SharedCommunicator"));
            test(service4.getProperty("Service").equals("4"));
            test(service4.getProperty("Prop").isEmpty());
            test(service4.getProperty("Service3.Prop").equals("1"));
            test(service4.getProperty("Ice.Trace.Slicing").equals("3"));

            String[] args4 = {"--Service3.Prop=2"};
            test(java.util.Arrays.equals(service4.getArgs(), args4));

            out.println("ok");
        } else {
            out.print("testing property inheritance... ");
            out.flush();

            test(service1.getProperty("Ice.ProgramName").equals("IceBox2-Service1"));
            test(service1.getProperty("ServerProp").equals("1"));
            test(service1.getProperty("OverrideMe").equals("2"));
            test(service1.getProperty("UnsetMe").isEmpty());
            test(service1.getProperty("Service1.Prop").equals("1"));
            test(service1.getProperty("Service1.ArgProp").equals("2"));

            test(service2.getProperty("Ice.ProgramName").equals("IceBox2-SharedCommunicator"));
            test(service2.getProperty("ServerProp").equals("1"));
            test(service2.getProperty("OverrideMe").equals("3"));
            test(service2.getProperty("UnsetMe").isEmpty());
            test(service2.getProperty("Service2.Prop").equals("1"));

            out.println("ok");
        }
    }
}
