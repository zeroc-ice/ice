// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.networkProxy;

import test.Ice.networkProxy.Test._TestIntfDisp;

public class Server extends test.Util.Application
{
    static public class TestI extends _TestIntfDisp
    {
        public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
        adapter.activate();
        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.networkProxy");
        initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        return initData;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
