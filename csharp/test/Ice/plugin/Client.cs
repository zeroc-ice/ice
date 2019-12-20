//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }

    public override void run(string[] args)
    {
        string pluginPath =
            String.Format("msbuild/plugin/{0}/Plugin.dll",
                          Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase)));
        {
            Console.Write("testing a simple plug-in... ");
            Console.Out.Flush();
            using var communicator = initialize(new Dictionary<string, string>
            {
                {
                    "Ice.Plugin.Test",
                    $"{pluginPath }:PluginFactory 'C:\\Program Files\\' --DatabasePath 'C:\\Program Files\\Application\\db'"
                }
            });
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing a simple plug-in that fails to initialize... ");
            Console.Out.Flush();
            try
            {
                initialize(new Dictionary<string, string>()
                {
                    {
                        "Ice.Plugin.Test", $"{pluginPath}:PluginInitializeFailFactory"
                    }
                });
                test(false);
            }
            catch (System.Exception)
            {
                //Excepted
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing plug-in load order... ");
            Console.Out.Flush();

            using var communicator = initialize(new Dictionary<string, string>()
            {
                { "Ice.Plugin.PluginOne", $"{pluginPath}:PluginOneFactory" },
                { "Ice.Plugin.PluginTwo", $"{pluginPath}:PluginTwoFactory" },
                { "Ice.Plugin.PluginThree", $"{pluginPath}:PluginThreeFactory" },
                { "Ice.PluginLoadOrder", "PluginOne, PluginTwo" } // Exclude PluginThree
            });
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing plug-in manager... ");
            Console.Out.Flush();

            MyPlugin p4;
            {
                using var communicator = initialize(new Dictionary<string, string>()
                {
                    { "Ice.Plugin.PluginOne", $"{pluginPath}:PluginOneFactory" },
                    { "Ice.Plugin.PluginTwo", $"{pluginPath}:PluginTwoFactory" },
                    { "Ice.Plugin.PluginThree", $"{pluginPath}:PluginThreeFactory" },
                    { "Ice.InitPlugins", "0"}
                });

                test(communicator.GetPlugin("PluginOne") != null);
                test(communicator.GetPlugin("PluginTwo") != null);
                test(communicator.GetPlugin("PluginThree") != null);

                p4 = new MyPlugin();
                communicator.AddPlugin("PluginFour", p4);
                test(communicator.GetPlugin("PluginFour") != null);

                communicator.InitializePlugins();

                test(p4.isInitialized());
            }
            test(p4.isDestroyed());
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing destroy when a plug-in fails to initialize... ");
            Console.Out.Flush();
            try
            {
                initialize(new Dictionary<string, string>()
                {
                    { "Ice.Plugin.PluginOneFail", $"{pluginPath}:PluginOneFailFactory" },
                    { "Ice.Plugin.PluginTwoFail", $"{pluginPath}:PluginTwoFailFactory" },
                    { "Ice.Plugin.PluginThreeFail", $"{pluginPath}:PluginThreeFailFactory" },
                    { "Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail"}
                });
            }
            catch (Exception)
            {
                // Expected
            }
            Console.WriteLine("ok");
        }
    }

    internal class MyPlugin : Ice.Plugin
    {
        public bool isInitialized()
        {
            return _initialized;
        }

        public bool isDestroyed()
        {
            return _destroyed;
        }

        public void initialize()
        {
            _initialized = true;
        }

        public void destroy()
        {
            _destroyed = true;
        }

        private bool _initialized = false;
        private bool _destroyed = false;
    }
}
