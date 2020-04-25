//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using Test;

public class Client : TestHelper
{
    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);

    public override void Run(string[] args)
    {
        string pluginPath =
            string.Format("msbuild/plugin/{0}/Plugin.dll",
                          Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase)));
        {
            Console.Write("testing a simple plug-in... ");
            Console.Out.Flush();
            using var communicator = Initialize(new Dictionary<string, string>
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
                Initialize(new Dictionary<string, string>()
                {
                    {
                        "Ice.Plugin.Test", $"{pluginPath}:PluginInitializeFailFactory"
                    }
                });
                Assert(false);
            }
            catch (Exception)
            {
                // Expected
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing plug-in load order... ");
            Console.Out.Flush();

            using var communicator = Initialize(new Dictionary<string, string>()
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
                using var communicator = Initialize(new Dictionary<string, string>()
                {
                    { "Ice.Plugin.PluginOne", $"{pluginPath}:PluginOneFactory" },
                    { "Ice.Plugin.PluginTwo", $"{pluginPath}:PluginTwoFactory" },
                    { "Ice.Plugin.PluginThree", $"{pluginPath}:PluginThreeFactory" },
                    { "Ice.InitPlugins", "0"}
                });

                Assert(communicator.GetPlugin("PluginOne") != null);
                Assert(communicator.GetPlugin("PluginTwo") != null);
                Assert(communicator.GetPlugin("PluginThree") != null);

                p4 = new MyPlugin();
                communicator.AddPlugin("PluginFour", p4);
                Assert(communicator.GetPlugin("PluginFour") != null);

                communicator.InitializePlugins();

                Assert(p4.isInitialized());
            }
            Assert(p4.isDestroyed());
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing destroy when a plug-in fails to initialize... ");
            Console.Out.Flush();
            try
            {
                Initialize(new Dictionary<string, string>()
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

    internal class MyPlugin : Ice.IPlugin
    {
        public bool isInitialized() => _initialized;

        public bool isDestroyed() => _destroyed;

        public void Initialize() => _initialized = true;

        public void Destroy() => _destroyed = true;

        private bool _initialized = false;
        private bool _destroyed = false;
    }
}
