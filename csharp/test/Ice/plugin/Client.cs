// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;
using Ice;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public static int Main(string[] args)
    {
        Ice.Communicator communicator = null;
        Console.Write("testing a simple plug-in... ");
        Console.Out.Flush();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.Test",
                                            "plugins/Plugin.dll:PluginFactory 'C:\\Program Files\\' --DatabasePath " +
                                            "'C:\\Program Files\\Application\\db'");
            communicator = Ice.Util.initialize(ref args, initData);
            communicator.destroy();
        }
        catch(Ice.LocalException ex)
        {
            Console.WriteLine(ex.ToString());;
            test(false);
        }
        Console.WriteLine("ok");

        Console.Write("testing a simple plug-in that fails to initialize... ");
        Console.Out.Flush();
        communicator = null;
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.Test", "plugins/Plugin.dll:PluginInitializeFailFactory");
            communicator = Ice.Util.initialize(ref args, initData);
            test(false);
        }
        catch(Ice.PluginInitializationException ex)
        {
            test(ex.InnerException.Message.Equals("PluginInitializeFailException"));
        }
        test(communicator == null);
        Console.WriteLine("ok");

        Console.Write("testing plug-in load order... ");
        Console.Out.Flush();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.PluginOne", "plugins/Plugin.dll:PluginOneFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwo", "plugins/Plugin.dll:PluginTwoFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThree", "plugins/Plugin.dll:PluginThreeFactory");
            initData.properties.setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo"); // Exclude PluginThree
            communicator = Ice.Util.initialize(ref args, initData);
            communicator.destroy();
        }
        catch(Ice.LocalException ex)
        {
            Console.WriteLine(ex.ToString());;
            test(false);
        }
        Console.WriteLine("ok");

        Console.Write("testing plug-in manager... ");
        Console.Out.Flush();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.PluginOne", "plugins/Plugin.dll:PluginOneFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwo", "plugins/Plugin.dll:PluginTwoFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThree", "plugins/Plugin.dll:PluginThreeFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThree", "plugins/Plugin.dll:PluginThreeFactory");
            initData.properties.setProperty("Ice.InitPlugins", "0");
            communicator = Ice.Util.initialize(ref args, initData);

            Ice.PluginManager pm = communicator.getPluginManager();
            test(pm.getPlugin("PluginOne") != null);
            test(pm.getPlugin("PluginTwo") != null);
            test(pm.getPlugin("PluginThree") != null);

            MyPlugin p4 = new MyPlugin();
            pm.addPlugin("PluginFour", p4);
            test(pm.getPlugin("PluginFour") != null);

            pm.initializePlugins();

            test(p4.isInitialized());

            communicator.destroy();

            test(p4.isDestroyed());
        }
        catch(Ice.LocalException ex)
        {
            Console.WriteLine(ex.ToString());;
            test(false);
        }
        Console.WriteLine("ok");

        Console.Write("testing destroy when a plug-in fails to initialize... ");
        Console.Out.Flush();
        communicator = null;
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.PluginOneFail",
                                            "plugins/Plugin.dll:PluginOneFailFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwoFail",
                                            "plugins/Plugin.dll:PluginTwoFailFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThreeFail",
                                            "plugins/Plugin.dll:PluginThreeFailFactory");
            initData.properties.setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
            communicator = Ice.Util.initialize(ref args, initData);
        }
        catch(Ice.PluginInitializationException ex)
        {
            test(ex.InnerException.Message.Equals("PluginInitializeFailException"));
        }
        test(communicator == null);
        Console.WriteLine("ok");

        return 0;
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
