// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceGrid.simple;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        communicator().getProperties().parseCommandLineOptions("TestAdapter", args);

        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        com.zeroc.Ice.Object object = new TestI();
        String id = communicator().getProperties().getPropertyWithDefault("Identity", "test");
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity(id));
        //shutdownOnInterrupt();
        try
        {
            adapter.activate();
        }
        catch(com.zeroc.Ice.ObjectAdapterDeactivatedException ex)
        {
        }
        communicator().waitForShutdown();
        //defaultInterrupt();
        return 0;
    }
    
    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        //
        // It's possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        r.initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        r.initData.properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        return r;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("test.IceGrid.simple.Server", args);
        
        System.gc();
        System.exit(status);
    }
}
