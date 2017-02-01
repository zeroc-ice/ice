// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + ":udp");
        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new MyDerivedClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));
        adapter.activate();
        return WAIT;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        //
        // It's possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        return initData;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);

        System.gc();
        System.exit(status);
    }
}
