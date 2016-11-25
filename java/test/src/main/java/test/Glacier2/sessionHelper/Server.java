// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.sessionHelper;

public class Server extends test.Util.Application
{
    public int run(String[] args)
    {
        communicator().getProperties().setProperty("DeactivatedAdapter.Endpoints", getTestEndpoint(1));
        communicator().createObjectAdapter("DeactivatedAdapter");

        communicator().getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
        adapter.add(new CallbackI(), com.zeroc.Ice.Util.stringToIdentity("callback"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Glacier2.router");
        return r;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);

        System.gc();
        System.exit(status);
    }
}
