// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceGrid.simple;

public class Server extends test.Util.Application
{
    @Override
    public int
    run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        argsH.value = communicator().getProperties().parseCommandLineOptions("TestAdapter", argsH.value);

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.Object object = new TestI();
        String id = communicator().getProperties().getPropertyWithDefault("Identity", "test");
        adapter.add(object, Ice.Util.stringToIdentity(id));
        //shutdownOnInterrupt();
        try
        {
            adapter.activate();
        }
        catch(Ice.ObjectAdapterDeactivatedException ex)
        {
        }
        communicator().waitForShutdown();
        //defaultInterrupt();
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
        //
        // Its possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we supress this warning.
        //
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        return initData;
    }

    public static void
    main(String[] args)
    {
        Server c = new Server();
        int status = c.main("test.IceGrid.simple.Server", args);

        System.gc();
        System.exit(status);
    }
}
