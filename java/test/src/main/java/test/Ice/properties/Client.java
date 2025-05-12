// Copyright (c) ZeroC, Inc.

package test.Ice.properties;

import com.zeroc.Ice.Properties;
import com.zeroc.Ice.PropertyException;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Client extends TestHelper {
    public static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    @Override
    public void run(String[] args) {
        {
            System.out.print("testing load properties from UTF-8 path... ");
            Properties properties = new Properties();
            properties.load(configPath);
            test("1".equals(properties.getIceProperty("Ice.Trace.Network")));
            test("1".equals(properties.getIceProperty("Ice.Trace.Protocol")));
            test(configPath.equals(properties.getProperty("Config.Path")));
            test("PropertiesClient".equals(properties.getIceProperty("Ice.ProgramName")));
            System.out.println("ok");
        }

        {
            //
            // Try to load multiple config files.
            //
            System.out.print("testing using Ice.Config with multiple config files... ");
            String[] args1 =
                new String[]{"--Ice.Config=config/config.1, config/config.2, config/config.3"};
            Properties properties = new Properties(args1);
            test("Config1".equals(properties.getProperty("Config1")));
            test("Config2".equals(properties.getProperty("Config2")));
            test("Config3".equals(properties.getProperty("Config3")));
            System.out.println("ok");
        }

        {
            System.out.print("testing configuration file escapes... ");
            String[] args1 = new String[]{"--Ice.Config=config/escapes.cfg"};
            Properties properties = new Properties(args1);

            String[] props =
                new String[]{
                    "Foo\tBar", "3",
                    "Foo\\tBar", "4",
                    "Escape\\ Space", "2",
                    "Prop1", "1",
                    "Prop2", "2",
                    "Prop3", "3",
                    "My Prop1", "1",
                    "My Prop2", "2",
                    "My.Prop1", "a property",
                    "My.Prop2", "a     property",
                    "My.Prop3", "  a     property  ",
                    "My.Prop4", "  a     property  ",
                    "My.Prop5", "a \\ property",
                    "foo=bar", "1",
                    "foo#bar", "2",
                    "foo bar", "3",
                    "A", "1",
                    "B", "2 3 4",
                    "C", "5=#6",
                    "AServer", "\\\\server\\dir",
                    "BServer", "\\server\\dir",
                    ""
                };

            for (int i = 0; !props[i].isEmpty(); i += 2) {
                test(properties.getProperty(props[i]).equals(props[i + 1]));
            }
            System.out.println("ok");
        }

        {
            System.out.print("testing ice properties with set default values...");
            Properties properties = new Properties();

            String toStringMode = properties.getIceProperty("Ice.ToStringMode");
            test("Unicode".equals(toStringMode));

            int closeTimeout = properties.getIcePropertyAsInt("Ice.Connection.Client.CloseTimeout");
            test(closeTimeout == 10);

            String[] retryIntervals = properties.getIcePropertyAsList("Ice.RetryIntervals");
            test(retryIntervals.length == 1);
            test("0".equals(retryIntervals[0]));

            System.out.println("ok");
        }

        {
            System.out.print("testing ice properties with unset default values...");
            Properties properties = new Properties();

            String stringValue = properties.getIceProperty("Ice.Default.Host");
            test(stringValue.isEmpty());

            int intValue = properties.getIcePropertyAsInt("Ice.Default.Host");
            test(intValue == 0);

            String[] listValue = properties.getIcePropertyAsList("Ice.Default.Host");
            test(listValue.length == 0);

            System.out.println("ok");
        }

        {
            System.out.print("testing that getting an unknown ice property throws an exception...");
            Properties properties = new Properties();
            try {
                properties.getIceProperty("Ice.UnknownProperty");
                test(false);
            } catch (PropertyException ex) {}
            System.out.println("ok");
        }

        {
            System.out.print("testing that setting an unknown ice property throws an exception...");
            Properties properties = new Properties();
            try {
                properties.setProperty("Ice.UnknownProperty", "bar");
                test(false);
            } catch (PropertyException ex) {}
            System.out.println("ok");
        }

        {
            try (var communicator = Util.initialize()) {
                var properties = communicator.getProperties();

                System.out.print(
                    "testing that creating an object adapter with unknown properties throws an"
                        + " exception...");
                properties.setProperty("FooOA.Endpoints", "tcp -h 127.0.0.1");
                properties.setProperty("FooOA.UnknownProperty", "bar");
                try {
                    communicator.createObjectAdapter("FooOA");
                    test(false);
                } catch (PropertyException ex) {}
                System.out.println("ok");

                System.out.print(
                    "testing that creating a proxy with unknown properties throws an"
                        + " exception...");
                properties.setProperty("FooProxy", "test:tcp -h 127.0.0.1 -p 10000");
                properties.setProperty("FooProxy.UnknownProperty", "bar");
                try {
                    communicator.propertyToProxy("FooProxy");
                    test(false);
                } catch (PropertyException ex) {}
                System.out.println("ok");

                System.out.print(
                    "testing that setting a property in an opt-in prefix that is not configured"
                        + " throws an exception...");
                System.out.flush();
                try {
                    properties.setProperty("IceGrid.InstanceName", "TestGrid");
                    test(false);
                } catch (PropertyException ex) {}
                System.out.println("ok");
            }
        }

        {
            System.out.print(
                "testing that passing a property multiple times on the command line uses the"
                    + " last value... ");
            System.out.flush();
            String[] commandLineArgs = {"--Ice.MessageSizeMax=10", "--Ice.MessageSizeMax=20"};
            Properties properties = new Properties(commandLineArgs);
            test("20".equals(properties.getIceProperty("Ice.MessageSizeMax")));
            System.out.println("ok");
        }

        {
            System.out.print(
                "testing that trying to read a non-numeric value as an int throws... ");
            System.out.flush();

            Properties properties = new Properties();
            properties.setProperty("Foo", "bar");
            try {
                properties.getIcePropertyAsInt("Foo");
                test(false);
            } catch (PropertyException ex) {}
            System.out.println("ok");
        }
    }

    //CHECKSTYLE:OFF: AvoidEscapedUnicodeCharacters
    private static final String configPath = "./config/\u4E2D\u56FD_client.config";
    //CHECKSTYLE:ON: AvoidEscapedUnicodeCharacters
}
