// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace Ice.plugin;

public class Client : Test.TestHelper
{
    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);

    public override void run(string[] args)
    {
        string pluginPath = "msbuild/plugin/net8.0/Plugin.dll";
        {
            Console.Write("testing a simple plug-in... ");
            Console.Out.Flush();
            var properties = new Ice.Properties();
            properties.setProperty(
                "Ice.Plugin.Test",
                $"{pluginPath}:Ice.plugin.PluginFactory 'C:\\Program Files\\' --DatabasePath 'C:\\Program Files\\Application\\db'");
            using (Ice.Communicator communicator = initialize(properties))
            {
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing a simple plug-in that fails to initialize... ");
            Console.Out.Flush();
            try
            {
                var properties = new Ice.Properties();
                properties.setProperty("Ice.Plugin.Test", $"{pluginPath}:Ice.plugin.PluginInitializeFailFactory");
                using Communicator _ = initialize(properties);
                test(false);
            }
            catch (Ice.PluginInitializationException ex)
            {
                test(ex.InnerException.Message == "PluginInitializeFailException");
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing plug-in load order... ");
            Console.Out.Flush();
            var properties = new Ice.Properties();
            properties.setProperty("Ice.Plugin.PluginOne", $"{pluginPath}:Ice.plugin.PluginOneFactory");
            properties.setProperty("Ice.Plugin.PluginTwo", $"{pluginPath}:Ice.plugin.PluginTwoFactory");
            properties.setProperty("Ice.Plugin.PluginThree", $"{pluginPath}:Ice.plugin.PluginThreeFactory");
            properties.setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo"); // Exclude PluginThree
            using (Ice.Communicator communicator = initialize(properties))
            {
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing plug-in manager... ");
            Console.Out.Flush();

            var properties = new Ice.Properties();
            properties.setProperty("Ice.Plugin.PluginOne", $"{pluginPath}:Ice.plugin.PluginOneFactory");
            properties.setProperty("Ice.Plugin.PluginTwo", $"{pluginPath}:Ice.plugin.PluginTwoFactory");
            properties.setProperty("Ice.Plugin.PluginThree", $"{pluginPath}:Ice.plugin.PluginThreeFactory");
            properties.setProperty("Ice.Plugin.PluginThree", $"{pluginPath}:Ice.plugin.PluginThreeFactory");
            properties.setProperty("Ice.InitPlugins", "0");

            MyPlugin p4 = null;
            using (Ice.Communicator communicator = initialize(properties))
            {
                Ice.PluginManager pm = communicator.getPluginManager();
                test(pm.getPlugin("PluginOne") != null);
                test(pm.getPlugin("PluginTwo") != null);
                test(pm.getPlugin("PluginThree") != null);

                p4 = new MyPlugin();
                pm.addPlugin("PluginFour", p4);
                test(pm.getPlugin("PluginFour") != null);

                pm.initializePlugins();

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
                var properties = new Ice.Properties();
                properties.setProperty("Ice.Plugin.PluginOneFail", $"{pluginPath}:Ice.plugin.PluginOneFailFactory");
                properties.setProperty("Ice.Plugin.PluginTwoFail", $"{pluginPath}:Ice.plugin.PluginTwoFailFactory");
                properties.setProperty("Ice.Plugin.PluginThreeFail", $"{pluginPath}:Ice.plugin.PluginThreeFailFactory");
                properties.setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
                using Communicator _ = initialize(properties);
            }
            catch (Ice.PluginInitializationException ex)
            {
                test(ex.InnerException.Message == "PluginInitializeFailException");
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing that IceDiscovery can't be loaded under a different name... ");
            Console.Out.Flush();
            try
            {
                var properties = new Ice.Properties();
                properties.setProperty("Ice.Plugin.Discovery", "IceDiscovery:IceDiscovery.PluginFactory");
                using Communicator _ = initialize(properties);
                test(false);
            }
            catch (Ice.PluginInitializationException ex)
            {
                test(ex.Message == "The Discovery plug-in must be named 'IceDiscovery'.");
            }
            Console.WriteLine("ok");
        }
    }

    internal class MyPlugin : Ice.Plugin
    {
        public bool isInitialized() => _initialized;

        public bool isDestroyed() => _destroyed;

        public void initialize() => _initialized = true;

        public void destroy() => _destroyed = true;

        private bool _initialized;
        private bool _destroyed;
    }
}
