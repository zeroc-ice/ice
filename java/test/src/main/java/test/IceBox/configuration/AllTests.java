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

            test("IceBox-Service1".equals(service1.getProperty("Ice.ProgramName")));
            test("1".equals(service1.getProperty("Service")));
            test("2".equals(service1.getProperty("Service1.Ovrd")));
            test(service1.getProperty("Service1.Unset").isEmpty());
            test("1".equals(service1.getProperty("Arg")));

            String[] args1 = {"-a", "--Arg=2"};
            test(java.util.Arrays.equals(service1.getArgs(), args1));

            test("Test".equals(service2.getProperty("Ice.ProgramName")));
            test("2".equals(service2.getProperty("Service")));
            test(service2.getProperty("Service1.ArgProp").isEmpty());

            String[] args2 = {"--Service1.ArgProp=1"};
            test(java.util.Arrays.equals(service2.getArgs(), args2));

            out.println("ok");

            out.print("testing with shared communicator... ");
            out.flush();

            test("IceBox-SharedCommunicator".equals(service3.getProperty("Ice.ProgramName")));
            test("4".equals(service3.getProperty("Service")));
            test(service3.getProperty("Prop").isEmpty());
            test("1".equals(service3.getProperty("Service3.Prop")));
            test("3".equals(service3.getProperty("Ice.Trace.Slicing")));

            test("IceBox-SharedCommunicator".equals(service4.getProperty("Ice.ProgramName")));
            test("4".equals(service4.getProperty("Service")));
            test(service4.getProperty("Prop").isEmpty());
            test("1".equals(service4.getProperty("Service3.Prop")));
            test("3".equals(service4.getProperty("Ice.Trace.Slicing")));

            String[] args4 = {"--Service3.Prop=2"};
            test(java.util.Arrays.equals(service4.getArgs(), args4));

            out.println("ok");
        } else {
            out.print("testing property inheritance... ");
            out.flush();

            test("IceBox2-Service1".equals(service1.getProperty("Ice.ProgramName")));
            test("1".equals(service1.getProperty("ServerProp")));
            test("2".equals(service1.getProperty("OverrideMe")));
            test(service1.getProperty("UnsetMe").isEmpty());
            test("1".equals(service1.getProperty("Service1.Prop")));
            test("2".equals(service1.getProperty("Service1.ArgProp")));

            test("IceBox2-SharedCommunicator".equals(service2.getProperty("Ice.ProgramName")));
            test("1".equals(service2.getProperty("ServerProp")));
            test("3".equals(service2.getProperty("OverrideMe")));
            test(service2.getProperty("UnsetMe").isEmpty());
            test("1".equals(service2.getProperty("Service2.Prop")));

            out.println("ok");
        }
    }

    private AllTests() {
    }
}
