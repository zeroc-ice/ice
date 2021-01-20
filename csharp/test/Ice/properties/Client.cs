// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Properties
{
    public static class Client
    {
        public static async Task RunAsync()
        {
            {
                Console.Out.Write("testing load properties from UTF-8 path... ");
                Console.Out.Flush();
                var properties = new Dictionary<string, string>();
                properties.LoadIceConfigFile("./config/中国_client.config");
                TestHelper.Assert(properties["Ice.Trace.Network"] == "1");
                TestHelper.Assert(properties["Ice.Trace.Protocol"] == "1");
                TestHelper.Assert(properties["Config.Path"] == "./config/中国_client.config");
                TestHelper.Assert(properties["Ice.ProgramName"] == "PropertiesClient");
                Console.Out.WriteLine("ok");
            }

            // Try to load multiple config files.
            {
                Console.Out.Write("testing using Ice.Config with multiple config files... ");
                Console.Out.Flush();
                var properties = new Dictionary<string, string>();
                string[] a = new string[] { "--Ice.Config=config/config.1, config/config.2, config/config.3" };
                properties.ParseArgs(ref a);
                TestHelper.Assert(properties["Config1"] == "Config1");
                TestHelper.Assert(properties["Config2"] == "Config2");
                TestHelper.Assert(properties["Config3"] == "Config3");
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

                TestHelper.Assert(properties.DictionaryEqual(props));
                Console.Out.WriteLine("ok");
            }

            {
                Console.Out.Write("testing configuration properties as bool... ");
                var boolProperties = new Dictionary<string, string>
                {
                    { "Bool.True.Integer", "1" },
                    { "Bool.True.LowerCase", "true" },
                    { "Bool.True.UpperCase", "TRUE" },
                    { "Bool.True.InitialUpperCase", "True" },

                    { "Bool.False.Integer", "0" },
                    { "Bool.False.LowerCase", "false" },
                    { "Bool.False.UpperCase", "FALSE" },
                    { "Bool.False.InitialLowerCase", "False" },

                    {"Bool.Bad.Integer", "2"},
                    {"Bool.Bad.Empty", ""},
                    {"Bool.Bad.NonTrueFalseWord", "hello"},
                    {"Bool.Bad.Yes", "yes"}
                };

                await using var communicator = new Communicator(boolProperties);

                {
                    var value = communicator.GetPropertyAsBool("Bool.True.Integer");
                    TestHelper.Assert(value == true);
                    value = communicator.GetPropertyAsBool("Bool.True.LowerCase");
                    TestHelper.Assert(value == true);
                    value = communicator.GetPropertyAsBool("Bool.True.UpperCase");
                    TestHelper.Assert(value == true);
                    value = communicator.GetPropertyAsBool("Bool.True.InitialUpperCase");
                    TestHelper.Assert(value == true);

                    value = communicator.GetPropertyAsBool("Bool.False.Integer");
                    TestHelper.Assert(value == false);
                    value = communicator.GetPropertyAsBool("Bool.False.LowerCase");
                    TestHelper.Assert(value == false);
                    value = communicator.GetPropertyAsBool("Bool.False.UpperCase");
                    TestHelper.Assert(value == false);
                    value = communicator.GetPropertyAsBool("Bool.False.InitialLowerCase");
                    TestHelper.Assert(value == false);
                }

                foreach (string property in communicator.GetProperties("Bool.Bad").Keys)
                {
                    try
                    {
                        _ = communicator.GetPropertyAsBool(property);
                        TestHelper.Assert(false);
                    }
                    catch (InvalidConfigurationException)
                    {
                    }
                }

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
                    TestHelper.Assert(duration == TimeSpan.FromMilliseconds(100));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyValue() == "100ms");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Seconds");
                    TestHelper.Assert(duration == TimeSpan.FromSeconds(5));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyValue() == "5s");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Minutes");
                    TestHelper.Assert(duration == TimeSpan.FromMinutes(9));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyValue() == "9m");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Hours");
                    TestHelper.Assert(duration == TimeSpan.FromHours(64));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyValue() == "64h");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Days");
                    TestHelper.Assert(duration == TimeSpan.FromDays(7));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyValue() == "7d");

                    duration = communicator.GetPropertyAsTimeSpan("Duration.Infinite");
                    TestHelper.Assert(duration == TimeSpan.FromMilliseconds(-1));
                    TestHelper.Assert(duration.HasValue && duration.Value.ToPropertyValue() == "infinite");
                }

                {
                    var duration = TimeSpan.Zero;
                    TestHelper.Assert(duration.ToPropertyValue() == "0ms");

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromMilliseconds(1));
                    TestHelper.Assert(duration.ToPropertyValue() == "60001ms");

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromSeconds(1));
                    TestHelper.Assert(duration.ToPropertyValue() == "61s");

                    duration = TimeSpan.FromMinutes(1).Add(TimeSpan.FromSeconds(60));
                    TestHelper.Assert(duration.ToPropertyValue() == "2m");

                    duration = TimeSpan.FromDays(1).Add(TimeSpan.FromMilliseconds(10));
                    TestHelper.Assert(duration.ToPropertyValue() == "86400010ms");
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
                    TestHelper.Assert(size == 1);

                    size = communicator.GetPropertyAsByteSize("Size.K");
                    TestHelper.Assert(size == 1024);

                    size = communicator.GetPropertyAsByteSize("Size.M");
                    TestHelper.Assert(size == 1024 * 1024);

                    size = communicator.GetPropertyAsByteSize("Size.G");
                    TestHelper.Assert(size == 1024 * 1024 * 1024);

                    size = communicator.GetPropertyAsByteSize("Size.Zero");
                    TestHelper.Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.Zero.K");
                    TestHelper.Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.Zero.M");
                    TestHelper.Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.Zero.G");
                    TestHelper.Assert(size == 0);

                    size = communicator.GetPropertyAsByteSize("Size.MaxValue.K");
                    TestHelper.Assert(size == int.MaxValue);

                    size = communicator.GetPropertyAsByteSize("Size.MaxValue.M");
                    TestHelper.Assert(size == int.MaxValue);

                    size = communicator.GetPropertyAsByteSize("Size.MaxValue.G");
                    TestHelper.Assert(size == int.MaxValue);
                }

                foreach (string property in communicator.GetProperties("Size.Bad").Keys)
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

            {
                var services = new string[] { "Logging Service", "Authentication Service", "Printing Service" };
                var loggerProperties = new string[] { "IceSSL.Protocols=tls12,tls13",
                                                      "Ice.ProgramName=My Service",
                                                      "Test=Foo's Bar's Demo\"",
                                                      "Foo\"",
                                                      "x,y,z",
                                                      "b"};

                var properties = new Dictionary<string, string>
                {
                    { "Services", StringUtil.ToPropertyValue(services) },
                    { "LoggerProperties", StringUtil.ToPropertyValue(loggerProperties) }
                };

                Console.Out.Write("testing properties as list... ");
                await using var communicator = new Communicator(properties);
                TestHelper.Assert(communicator.GetPropertyAsList("Services")!.SequenceEqual(services));
                TestHelper.Assert(communicator.GetPropertyAsList("LoggerProperties")!.SequenceEqual(loggerProperties));
                Console.Out.WriteLine("ok");
            }
        }

        public static async Task<int> Main()
        {
            int status = 0;
            try
            {
                await RunAsync();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                status = 1;
            }
            return status;
        }
    }
}
