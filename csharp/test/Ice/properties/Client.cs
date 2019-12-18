//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Reflection;
using Ice;

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
            var properties = new Dictionary<string, string>();
            properties.LoadIceConfigFile("./config/中国_client.config");
            test(properties["Ice.Trace.Network"] == "1");
            test(properties["Ice.Trace.Protocol"] == "1");
            test(properties["Config.Path"] == "./config/中国_client.config");
            test(properties["Ice.ProgramName"] == "PropertiesClient");
            Console.Out.WriteLine("ok");
        }

        //
        // Try to load multiple config files.
        //
        {
            Console.Out.Write("testing using Ice.Config with multiple config files... ");
            Console.Out.Flush();
            var properties = new Dictionary<string, string>();
            string[] a = new string[] { "--Ice.Config=config/config.1, config/config.2, config/config.3" };
            properties.ParseArgs(ref a);
            test(properties["Config1"] == "Config1");
            test(properties["Config2"] == "Config2");
            test(properties["Config3"] == "Config3");
            Console.Out.WriteLine("ok");
        }

        {
            Console.Out.Write("testing configuration file escapes... ");
            Console.Out.Flush();
            var properties = new Dictionary<string, string>();
            var a = new string[] { "--Ice.Config=config/escapes.cfg" };
            properties.ParseArgs(ref a);

            var props = new Dictionary<string, string>
            {
                { "Foo\tBar", "3" },
                { "Foo\\tBar", "4" },
                { "Escape\\ Space", "2" },
                { "Prop1", "1" },
                { "Prop2", "2" },
                { "Prop3", "3" },
                { "My Prop1", "1" },
                { "My Prop2", "2" },
                { "My.Prop1", "a property" },
                { "My.Prop2", "a     property" },
                { "My.Prop3", "  a     property  " },
                { "My.Prop4", "  a     property  " },
                { "My.Prop5", "a \\ property" },
                { "foo=bar", "1" },
                { "foo#bar", "2" },
                { "foo bar", "3" },
                { "A", "1" },
                { "B", "2 3 4" },
                { "C", "5=#6" },
                { "AServer", "\\\\server\\dir" },
                { "BServer", "\\server\\dir" },
                { "Ice.Config", "config/escapes.cfg" }
            };

            test(Collections.Equals(properties, props));
            Console.Out.WriteLine("ok");
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
