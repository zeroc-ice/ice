// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.IO;
using System.Reflection;

[assembly: CLSCompliant(true)]

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
#if NET45
        string pluginPath = "msbuild/plugin/net45/Plugin.dll";
#else
        string pluginPath =
            String.Format("msbuild/plugin/netstandard2.0/{0}/Plugin.dll",
                          Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase)));
#endif
        {
            Console.Write("testing a simple plug-in... ");
            Console.Out.Flush();
            Ice.Properties properties = Ice.Util.createProperties();
            properties.setProperty("Ice.Plugin.Test",
                pluginPath + ":PluginFactory 'C:\\Program Files\\' --DatabasePath " +
                "'C:\\Program Files\\Application\\db'");
            using(var communicator = initialize(properties))
            {
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing a simple plug-in that fails to initialize... ");
            Console.Out.Flush();
            try
            {
                Ice.Properties properties = Ice.Util.createProperties();
                properties.setProperty("Ice.Plugin.Test", pluginPath + ":PluginInitializeFailFactory");
                initialize(properties);
                test(false);
            }
            catch(Ice.PluginInitializationException ex)
            {
                test(ex.InnerException.Message.Equals("PluginInitializeFailException"));
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing plug-in load order... ");
            Console.Out.Flush();
            Ice.Properties properties = Ice.Util.createProperties();
            properties.setProperty("Ice.Plugin.PluginOne", pluginPath + ":PluginOneFactory");
            properties.setProperty("Ice.Plugin.PluginTwo", pluginPath + ":PluginTwoFactory");
            properties.setProperty("Ice.Plugin.PluginThree", pluginPath + ":PluginThreeFactory");
            properties.setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo"); // Exclude PluginThree
            using(var communicator = initialize(properties))
            {
            }
            Console.WriteLine("ok");
        }

        {
            Console.Write("testing plug-in manager... ");
            Console.Out.Flush();

            Ice.Properties properties = Ice.Util.createProperties();
            properties.setProperty("Ice.Plugin.PluginOne", pluginPath + ":PluginOneFactory");
            properties.setProperty("Ice.Plugin.PluginTwo", pluginPath + ":PluginTwoFactory");
            properties.setProperty("Ice.Plugin.PluginThree", pluginPath + ":PluginThreeFactory");
            properties.setProperty("Ice.Plugin.PluginThree", pluginPath + ":PluginThreeFactory");
            properties.setProperty("Ice.InitPlugins", "0");

            MyPlugin p4 = null;
            using(var communicator = initialize(properties))
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
                Ice.Properties properties = Ice.Util.createProperties();
                properties.setProperty("Ice.Plugin.PluginOneFail", pluginPath + ":PluginOneFailFactory");
                properties.setProperty("Ice.Plugin.PluginTwoFail", pluginPath + ":PluginTwoFailFactory");
                properties.setProperty("Ice.Plugin.PluginThreeFail", pluginPath + ":PluginThreeFailFactory");
                properties.setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
                initialize(properties);
            }
            catch(Ice.PluginInitializationException ex)
            {
                test(ex.InnerException.Message.Equals("PluginInitializeFailException"));
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
