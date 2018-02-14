// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.faultTolerance;

import java.io.PrintWriter;

public class Server extends test.Util.Application
{
    private static void
    usage()
    {
        System.err.println("Usage: Server port");
    }

    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        int port = 0;
        PrintWriter out = getWriter();
        for(String arg : args)
        {
            if(arg.charAt(0) == '-')
            {
                out.println("Server: unknown option `" + arg + "'");
                usage();
                return 1;
            }

            if(port > 0)
            {
                out.println("Server: only one port can be specified");
                usage();
                return 1;
            }

            try
            {
                port = Integer.parseInt(arg);
            }
            catch(NumberFormatException ex)
            {
                out.println("Server: invalid port");
                usage();
                return 1;
            }
        }

        if(port <= 0)
        {
            out.println("Server: no port specified");
            usage();
            return 1;
        }

        // Don't move this, it needs the port.
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(port));
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        com.zeroc.Ice.Object object = new TestI(port);
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("test"));
        adapter.activate();
        return WAIT;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.faultTolerance");
        // Two minutes.
        initData.properties.setProperty("Ice.ServerIdleTime", "120");
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
