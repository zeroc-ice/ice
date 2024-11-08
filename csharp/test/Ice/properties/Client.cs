// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        {
            Console.Out.Write("testing load properties from UTF-8 path... ");
            Console.Out.Flush();
            Ice.Properties properties = new Ice.Properties();
            properties.load("./config/中国_client.config");
            test(properties.getIceProperty("Ice.Trace.Network") == "1");
            test(properties.getIceProperty("Ice.Trace.Protocol") == "1");
            test(properties.getProperty("Config.Path") == "./config/中国_client.config");
            test(properties.getIceProperty("Ice.ProgramName") == "PropertiesClient");
            Console.Out.WriteLine("ok");
            Console.Out.Write("testing load properties from UTF-8 path using Ice::Application... ");
            Console.Out.Flush();
            Console.Out.WriteLine("ok");
        }

        //
        // Try to load multiple config files.
        //
        {
            Console.Out.Write("testing using Ice.Config with multiple config files... ");
            Console.Out.Flush();
            string[] args1 = new string[] { "--Ice.Config=config/config.1, config/config.2, config/config.3" };
            Ice.Properties properties = new Ice.Properties(ref args1);
            test(properties.getProperty("Config1") == "Config1");
            test(properties.getProperty("Config2") == "Config2");
            test(properties.getProperty("Config3") == "Config3");
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing configuration file escapes... ");
            Console.Out.Flush();
            string[] args1 = new string[] { "--Ice.Config=config/escapes.cfg" };
            Ice.Properties properties = new Ice.Properties(ref args1);

            string[] props = new string[]{"Foo\tBar", "3",
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
                                          ""};

            for (int i = 0; props[i].Length > 0; i += 2)
            {
                test(properties.getProperty(props[i]).Equals(props[i + 1]));
            }
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing ice properties with set default values...");
            Console.Out.Flush();
            Ice.Properties properties = new Ice.Properties();
            string toStringMode = properties.getIceProperty("Ice.ToStringMode");
            test(toStringMode == "Unicode");
            int closeTimeout = properties.getIcePropertyAsInt("Ice.Connection.Client.CloseTimeout");
            test(closeTimeout == 10);
            string[] retryIntervals = properties.getIcePropertyAsList("Ice.RetryIntervals");
            test(retryIntervals.Length == 1);
            test(retryIntervals[0] == "0");
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing ice properties with unset default values...");
            Console.Out.Flush();
            Ice.Properties properties = new Ice.Properties();
            string stringValue = properties.getIceProperty("Ice.Admin.Router");
            test(stringValue == "");
            int intValue = properties.getIcePropertyAsInt("Ice.Admin.Router");
            test(intValue == 0);
            string[] listValue = properties.getIcePropertyAsList("Ice.Admin.Router");
            test(listValue.Length == 0);
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing that getting an unknown ice property throws an exception...");
            Console.Out.Flush();
            Ice.Properties properties = new Ice.Properties();
            try
            {
                properties.getIceProperty("Ice.UnknownProperty");
                test(false);
            }
            catch (Ice.PropertyException)
            {
            }
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing that setting an unknown ice property throws an exception...");
            Console.Out.Flush();
            Ice.Properties properties = new Ice.Properties();
            try
            {
                properties.setProperty("Ice.UnknownProperty", "bar");
                test(false);
            }
            catch (Ice.PropertyException)
            {
            }
            Console.Out.WriteLine("ok");
        }

        {
            using Ice.Communicator communicator = Ice.Util.initialize();
            Ice.Properties properties = communicator.getProperties();

            Console.Out.Write("testing that creating an object adapter with unknown properties throws an exception...");
            properties.setProperty("FooOA.Endpoints", "tcp -h 127.0.0.1");
            properties.setProperty("FooOA.UnknownProperty", "bar");
            try
            {
                communicator.createObjectAdapter("FooOA");
                test(false);
            }
            catch (Ice.PropertyException)
            {
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing that creating a proxy with unknown properties throws an exception...");
            properties.setProperty("FooProxy", "test:tcp -h 127.0.0.1 -p 10000");
            properties.setProperty("FooProxy.UnknownProperty", "bar");
            try
            {
                communicator.propertyToProxy("FooProxy");
                test(false);
            }
            catch (Ice.PropertyException)
            {
            }

            Console.Out.WriteLine("ok");

            Console.Out.Write("testing that setting a property in an opt-in prefix that is not configured throws an exception...");
            Console.Out.Flush();
            try
            {
                properties.setProperty("IceGrid.InstanceName", "TestGrid");
                test(false);
            }
            catch (Ice.PropertyException)
            {
            }
            Console.Out.WriteLine("ok");
        }
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
