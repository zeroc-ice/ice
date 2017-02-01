// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("communicator");
        adapter.add(new RemoteCommunicatorI(this), id);
        adapter.activate();

        return WAIT;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.binding");
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0) + ":udp");
        initData.logger = new com.zeroc.Ice.Logger() {
            @Override public void print(String message)
            {
            }

            @Override public void trace(String category, String message)
            {
            }

            @Override public void warning(String message)
            {
            }

            @Override public void error(String message)
            {
            }

            @Override public String getPrefix()
            {
                return "NullLogger";
            }

            @Override public com.zeroc.Ice.Logger cloneWithPrefix(String prefix)
            {
                return this;
            }
        };
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
