// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

import test.Ice.background.PluginFactory.PluginI;
import test.Ice.background.Test.BackgroundPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        Configuration configuration = new Configuration();
        PluginI plugin = (PluginI)communicator().getPluginManager().getPlugin("Test");
        plugin.setConfiguration(configuration);
        communicator().getPluginManager().initializePlugins();

        BackgroundPrx background = AllTests.allTests(configuration, communicator(), getWriter());
        background.shutdown();
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);

        // For this test, we want to disable retries.
        //
        r.initData.properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.MessageSizeMax", "50000");

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        r.initData.properties.setProperty("Ice.TCP.SndSize", "50000");

        //
        // Setup the test transport plug-in.
        //
        r.initData.properties.setProperty("Ice.Plugin.Test", "test.Ice.background.PluginFactory");
        String defaultProtocol = r.initData.properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        r.initData.properties.setProperty("Ice.Default.Protocol", "test-" + defaultProtocol);

        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.background");

        // Don't initialize the plugin until I've set the configuration.
        r.initData.properties.setProperty("Ice.InitPlugins", "0");

        return r;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
