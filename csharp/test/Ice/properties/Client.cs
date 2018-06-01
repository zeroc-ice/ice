// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    class PropertiesClient : Ice.Application
    {
        public override int
        run(String[] args)
        {
            Ice.Properties properties = communicator().getProperties();
            test(properties.getProperty("Ice.Trace.Network").Equals("1"));
            test(properties.getProperty("Ice.Trace.Protocol").Equals("1"));
            test(properties.getProperty("Config.Path").Equals("./config/中国_client.config"));
            test(properties.getProperty("Ice.ProgramName").Equals("PropertiesClient"));
            test(appName().Equals(properties.getProperty("Ice.ProgramName")));
            return 0;
        }
    }

    public override void run(string[] args)
    {
        {
            Console.Out.Write("testing load properties from UTF-8 path... ");
            Console.Out.Flush();
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("./config/中国_client.config");
            test(properties.getProperty("Ice.Trace.Network").Equals("1"));
            test(properties.getProperty("Ice.Trace.Protocol").Equals("1"));
            test(properties.getProperty("Config.Path").Equals("./config/中国_client.config"));
            test(properties.getProperty("Ice.ProgramName").Equals("PropertiesClient"));
            Console.Out.WriteLine("ok");
            Console.Out.Write("testing load properties from UTF-8 path using Ice::Application... ");
            Console.Out.Flush();
            PropertiesClient c = new PropertiesClient();
            c.main(args, "./config/中国_client.config");
            Console.Out.WriteLine("ok");
        }

        //
        // Try to load multiple config files.
        //
        {
            Console.Out.Write("testing using Ice.Config with multiple config files... ");
            Console.Out.Flush();
            string[] args1 = new string[]{"--Ice.Config=config/config.1, config/config.2, config/config.3"};
            Ice.Properties properties = Ice.Util.createProperties(ref args1);
            test(properties.getProperty("Config1").Equals("Config1"));
            test(properties.getProperty("Config2").Equals("Config2"));
            test(properties.getProperty("Config3").Equals("Config3"));
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing configuration file escapes... ");
            Console.Out.Flush();
            string[] args1 = new string[]{"--Ice.Config=config/escapes.cfg"};
            Ice.Properties properties = Ice.Util.createProperties(ref args1);

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

            for(int i = 0; props[i].Length > 0; i += 2)
            {
                test(properties.getProperty(props[i]).Equals(props[i + 1]));
            }
            Console.Out.WriteLine("ok");
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
