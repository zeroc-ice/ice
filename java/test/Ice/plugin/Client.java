// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin;

import java.io.PrintWriter;

public class Client extends test.Util.Application
{
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        PrintWriter printWriter = getWriter();
        printWriter.print("testing a simple plug-in... ");
        printWriter.flush();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.Test",
                "../../../lib/IceTestPlugins.jar:test.Ice.plugin.plugins.PluginFactory " +
                "'C:\\Program Files\\' --DatabasePath 'C:\\Program Files\\Application\\db'");
            communicator = Ice.Util.initialize(args, initData);
            communicator.destroy();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
        printWriter.println("ok");

        printWriter.print("testing a simple plug-in that fails to initialize... ");
        printWriter.flush();
        communicator = null;
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.Test",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginInitializeFailFactory");
            communicator = Ice.Util.initialize(args, initData);
            test(false);
        }
        catch(RuntimeException ex)
        {
            test(ex.getMessage().equals("PluginInitializeFailException"));
        }
        test(communicator == null);
        printWriter.println("ok");

        printWriter.print("testing plug-in load order... ");
        printWriter.flush();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.PluginOne",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginOneFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwo",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginTwoFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThree",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginThreeFactory");
            initData.properties.setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo"); // Exclude PluginThree
            communicator = Ice.Util.initialize(args, initData);
            communicator.destroy();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
        printWriter.println("ok");

        printWriter.print("testing plug-in manager... ");
        printWriter.flush();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.PluginOne",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginOneFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwo",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginTwoFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThree",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginThreeFactory");
            initData.properties.setProperty("Ice.PluginLoadOrder", "PluginOne, PluginTwo");
            initData.properties.setProperty("Ice.InitPlugins", "0");
            communicator = Ice.Util.initialize(args, initData);

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
            ex.printStackTrace();
            test(false);
        }
        printWriter.println("ok");

        printWriter.print("testing destroy when a plug-in fails to initialize... ");
        printWriter.flush();
        communicator = null;
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.setProperty("Ice.Plugin.PluginOneFail",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginOneFailFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwoFail",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginTwoFailFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThreeFail",
                                            "../../../lib/:test.Ice.plugin.plugins.PluginThreeFailFactory");
            initData.properties.setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
            communicator = Ice.Util.initialize(args, initData);
        }
        catch(RuntimeException ex)
        {
            test(ex.getMessage().equals("PluginInitializeFailException"));
        }
        test(communicator == null);
        printWriter.println("ok");

        System.gc();
        System.runFinalization();
        return 0;
    }

    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        return initData;
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }

    static class MyPlugin implements Ice.Plugin
    {
        public boolean isInitialized()
        {
            return _initialized;
        }

        public boolean isDestroyed()
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

        private boolean _initialized = false;
        private boolean _destroyed = false;
    }
}
