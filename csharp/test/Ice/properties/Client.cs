//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Properties
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
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

                await using var communicator = new Communicator(timeSpanProperties);

                {
                    var duration = communicator.GetPropertyAsTimeSpan("Duration.Milliseconds");
                    Assert(duration == TimeSpan.FromMilliseconds(100));
                    Assert(duration.HasValue && duration.Value.ToPropertyString() == "100ms");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Seconds");
                    Assert(duration == TimeSpan.FromSeconds(5));
                    Assert(duration.HasValue && duration.Value.ToPropertyString() == "5s");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Minutes");
                    Assert(duration == TimeSpan.FromMinutes(9));
                    Assert(duration.HasValue && duration.Value.ToPropertyString() == "9m");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Hours");
                    Assert(duration == TimeSpan.FromHours(64));
                    Assert(duration.HasValue && duration.Value.ToPropertyString() == "64h");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Days");
                    Assert(duration == TimeSpan.FromDays(7));
                    Assert(duration.HasValue && duration.Value.ToPropertyString() == "7d");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Infinite");
                    Assert(duration == TimeSpan.FromMilliseconds(-1));
                    Assert(duration.HasValue && duration.Value.ToPropertyString() == "infinite");
                }

                {
                    var duration = TimeSpan.Zero;
                    Assert(duration.ToPropertyString() == "0ms");

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromMilliseconds(1));
                    Assert(duration.ToPropertyString() == "60001ms");

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromSeconds(1));
                    Assert(duration.ToPropertyString() == "61s");

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromSeconds(60));
                    Assert(duration.ToPropertyString() == "2m");

                    duration = TimeSpan.FromDays(1).Add(TimeSpan.FromMilliseconds(10));
                    Assert(duration.ToPropertyString() == "86400010ms");
                }

                foreach (string property in communicator.GetProperties("Duration.Bad").Keys)
                {
                    try
                    {
                        _ = communicator.GetPropertyAsTimeSpan(property);
                        Assert(false);
                    }
                    catch (InvalidConfigurationException)
                    {
                    }
                }

                Console.Out.WriteLine("ok");
            }

            {
                Console.Out.Write("testing configuration properties as byte size... ");
                var sizeProperties = new Dictionary<string, string>
                {
                    { "Size", "1" },
                    { "Size.K", "1K" },
                    { "Size.M", "1M" },
                    { "Size.G", "1G" },

                    { "Size.Zero", "0" },
                    { "Size.Zero.K", "0K" },
                    { "Size.Zero.M", "0M" },
                    { "Size.Zero.G", "0G" },

                    { "Size.MaxValue.K", $"{int.MaxValue}K" },
                    { "Size.MaxValue.M", $"{int.MaxValue}M" },
                    { "Size.MaxValue.G", $"{int.MaxValue}G" },

                    { "Size.Bad.Word", "x"},
                    { "Size.Bad.Negative", "-1B"},
                    { "Size.Bad.InvalidUnit", "1b"},
                    { "Size.Bad.NotANumber", "NaN"},
                    { "Size.Bad.Double.B", "1.0" },
                };

                await using var communicator = new Communicator(sizeProperties);

                {
                    int? size = communicator.GetPropertyAsByteSize("Size");
                    Assert(size == 1);

                    size = communicator.GetPropertyAsByteSize("Size.K");
                    Assert(size == 1024);

                    size = communicator.GetPropertyAsByteSize("Size.M");
                    Assert(size == 1024 * 1024);

                    size = communicator.GetPropertyAsByteSize("Size.G");
                    Assert(size == 1024 * 1024 * 1024);

                    size = communicator.GetPropertyAsByteSize("Size.Zero");
                    Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.Zero.K");
                    Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.Zero.M");
                    Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.Zero.G");
                    Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.MaxValue.K");
                    Assert(size == int.MaxValue);

                    size = communicator.GetPropertyAsByteSize("Size.MaxValue.M");
                    Assert(size == int.MaxValue);

                    size = communicator.GetPropertyAsByteSize("Size.MaxValue.G");
                    Assert(size == int.MaxValue);
                }

                foreach (string property in communicator.GetProperties("Size.Bad").Keys)
                {
                    try
                    {
                        _ = communicator.GetPropertyAsTimeSpan(property);
                        Assert(false);
                    }
                    catch (InvalidConfigurationException)
                    {
                    }
                }

                Console.Out.WriteLine("ok");
            }
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
