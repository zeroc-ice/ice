// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.exceptions;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.Object object = new TestI();
        adapter.add(object, Ice.Util.stringToIdentity("Test"));
        adapter.activate();
        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.slicing.exceptions.server");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010 -t 2000");
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
