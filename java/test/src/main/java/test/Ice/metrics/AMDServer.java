// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

public class AMDServer extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new AMDMetricsI(), com.zeroc.Ice.Util.stringToIdentity("metrics"));
        adapter.activate();

        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        com.zeroc.Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("controller"));
        controllerAdapter.activate();

        return WAIT;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        r.initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(r.initData.properties, 0));
        r.initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        r.initData.properties.setProperty("Ice.Admin.InstanceName", "server");
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        r.initData.properties.setProperty("Ice.MessageSizeMax", "50000");
        return r;
    }

    public static void main(String[] args)
    {
        AMDServer app = new AMDServer();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
