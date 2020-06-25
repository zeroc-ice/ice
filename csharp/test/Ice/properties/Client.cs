//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Properties
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            {
                Console.Out.Write("testing load properties from UTF-8 path... ");
                Console.Out.Flush();
                var properties = new Dictionary<string, string>();
                properties.LoadIceConfigFile("./config/中国_client.config");
                Assert(properties["Ice.Trace.Network"] == "1");
                Assert(properties["Ice.Trace.Protocol"] == "1");
                Assert(properties["Config.Path"] == "./config/中国_client.config");
                Assert(properties["Ice.ProgramName"] == "PropertiesClient");
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
                Assert(properties["Config1"] == "Config1");
                Assert(properties["Config2"] == "Config2");
                Assert(properties["Config3"] == "Config3");
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

                Assert(properties.DictionaryEqual(props));
                Console.Out.WriteLine("ok");
            }

            {
                Console.Out.Write("testing configuration properties as TimeSpan... ");
                var timeSpanProperties = new Dictionary<string, string>
                {
                    { "Duration.Milliseconds", "100ms" },
                    { "Duration.Seconds", "5s" },
                    { "Duration.Minutes", "9m" },
                    { "Duration.Hours", "64h" },
                    { "Duration.Days", "7d" },
                    { "Duration.Infinite", "infinite" },

                    { "Duration.Bad.Double", "1.1ms" },
                    { "Duration.Bad.Negative", "-5s" },
                    { "Duration.Bad.Combination", "1m10s" },
                    { "Duration.Bad.AboveMax", $"{TimeSpan.MaxValue.TotalMilliseconds + 1}ms"},
                    { "Duration.Bad.NotANumber", "NaN"},
                    { "Duration.Bad.NoUnits", "42"}
                };

                using var communicator = new Communicator(timeSpanProperties);

                {
                    var duration = communicator.GetPropertyAsTimeSpan("Duration.Milliseconds");
                    TestHelper.Assert(duration.Equals(TimeSpan.FromMilliseconds(100)));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyString().Equals("100ms"));

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Seconds");
                    TestHelper.Assert(duration.Equals(TimeSpan.FromSeconds(5)));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyString().Equals("5s"));

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Minutes");
                    TestHelper.Assert(duration.Equals(TimeSpan.FromMinutes(9)));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyString().Equals("9m"));

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Hours");
                    TestHelper.Assert(duration.Equals(TimeSpan.FromHours(64)));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyString().Equals("64h"));

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Days");
                    TestHelper.Assert(duration.Equals(TimeSpan.FromDays(7)));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyString().Equals("7d"));

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Infinite");
                    TestHelper.Assert(duration.Equals(TimeSpan.FromMilliseconds(-1)));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyString().Equals("infinite"));
                }

                {
                    var duration = TimeSpan.Zero;
                    TestHelper.Assert(duration.ToPropertyString().Equals("0ms"));

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromMilliseconds(1));
                    TestHelper.Assert(duration.ToPropertyString().Equals("60001ms"));

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromSeconds(1));
                    TestHelper.Assert(duration.ToPropertyString().Equals("61s"));

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromSeconds(60));
                    TestHelper.Assert(duration.ToPropertyString().Equals("2m"));

                    duration = TimeSpan.FromDays(1).Add(TimeSpan.FromMilliseconds(10));
                    TestHelper.Assert(duration.ToPropertyString().Equals("86400010ms"));
                }

                foreach (string property in communicator.GetProperties("Duration.Bad").Keys)
                {
                    try
                    {
                        _ = communicator.GetPropertyAsTimeSpan(property);
                        TestHelper.Assert(false);
                    }
                    catch (InvalidConfigurationException)
                    {
                    }
                }

                Console.Out.WriteLine("ok");
            }
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
