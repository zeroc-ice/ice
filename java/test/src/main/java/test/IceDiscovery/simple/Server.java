// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceDiscovery.simple;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Properties properties = communicator().getProperties();

        int num = 0;
        try
        {
            num = Integer.parseInt(args[0]);
        }
        catch(NumberFormatException ex)
        {
            assert(false);
        }

        properties.setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
        properties.setProperty("ControlAdapter.AdapterId", "control" + num);
        properties.setProperty("ControlAdapter.ThreadPool.Size", "1");

        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("ControlAdapter");
        adapter.add(new ControllerI(), com.zeroc.Ice.Util.stringToIdentity("controller" + num));
        adapter.activate();

        return WAIT;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);
        System.gc();
        System.exit(status);
    }
}
