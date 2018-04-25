// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin;

import java.io.PrintWriter;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        // Under Android the class comes from the communicators classloader which
        // is setup in the android test driver.
        String jarFile;
        if(isAndroid())
        {
            jarFile = "";
        }
        else
        {
            jarFile = "../../../../../../../lib/IceTestPlugins.jar";
        }

        Ice.Communicator communicator = communicator();
        PrintWriter printWriter = getWriter();
        printWriter.print("testing a simple plug-in... ");
        printWriter.flush();
        try
        {
            Ice.InitializationData initData = createInitData();
            initData.properties.setProperty("Ice.Plugin.Test",
                jarFile + ":test.Ice.plugin.plugins.PluginFactory " +
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
            Ice.InitializationData initData = createInitData();
            initData.properties.setProperty("Ice.Plugin.Test",
                                            jarFile + ":test.Ice.plugin.plugins.PluginInitializeFailFactory");
            communicator = Ice.Util.initialize(args, initData);
            test(false);
        }
        catch(Ice.PluginInitializationException ex)
        {
            test(ex.getCause().getMessage().equals("PluginInitializeFailException"));
        }
        test(communicator == null);
        printWriter.println("ok");

        printWriter.print("testing plug-in load order... ");
        printWriter.flush();
        try
        {
            Ice.InitializationData initData = createInitData();
            initData.properties.setProperty("Ice.Plugin.PluginOne",
                                            jarFile + ":test.Ice.plugin.plugins.PluginOneFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwo",
                                            jarFile + ":test.Ice.plugin.plugins.PluginTwoFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThree",
                                            jarFile + ":test.Ice.plugin.plugins.PluginThreeFactory");
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
            Ice.InitializationData initData = createInitData();
            initData.properties.setProperty("Ice.Plugin.PluginOne",
                                            jarFile + ":test.Ice.plugin.plugins.PluginOneFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwo",
                                            jarFile + ":test.Ice.plugin.plugins.PluginTwoFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThree",
                                            jarFile + ":test.Ice.plugin.plugins.PluginThreeFactory");
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
            Ice.InitializationData initData = createInitData();
            initData.properties.setProperty("Ice.Plugin.PluginOneFail",
                                            jarFile + ":test.Ice.plugin.plugins.PluginOneFailFactory");
            initData.properties.setProperty("Ice.Plugin.PluginTwoFail",
                                            jarFile + ":test.Ice.plugin.plugins.PluginTwoFailFactory");
            initData.properties.setProperty("Ice.Plugin.PluginThreeFail",
                                            jarFile + ":test.Ice.plugin.plugins.PluginThreeFailFactory");
            initData.properties.setProperty("Ice.PluginLoadOrder", "PluginOneFail, PluginTwoFail, PluginThreeFail");
            communicator = Ice.Util.initialize(args, initData);
        }
        catch(Ice.PluginInitializationException ex)
        {
            test(ex.getCause().getMessage().equals("PluginInitializeFailException"));
        }
        test(communicator == null);
        printWriter.println("ok");

        System.gc();
        System.runFinalization();
        return 0;
    }

    private Ice.InitializationData createInitData()
    {
        Ice.InitializationData initData = createInitializationData() ;
        if(classLoader() != null)
        {
            initData.classLoader = classLoader();
        }
        initData.properties = Ice.Util.createProperties();
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

        @Override
        public void initialize()
        {
            _initialized = true;
        }

        @Override
        public void destroy()
        {
            _destroyed = true;
        }

        private boolean _initialized = false;
        private boolean _destroyed = false;
    }
}
