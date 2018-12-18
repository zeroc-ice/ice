// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.faultTolerance;

import java.io.PrintWriter;

public class Server extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        properties.setProperty("Ice.Package.Test", "test.Ice.faultTolerance");
        properties.setProperty("Ice.ServerIdleTime", "120");

        try(Ice.Communicator communicator = initialize(properties))
        {
            int port = 0;
            PrintWriter out = getWriter();
            for(String arg : argsH.value)
            {
                if(arg.charAt(0) == '-')
                {
                    throw new RuntimeException("Server: unknown option `" + arg + "'");
                }

                if(port > 0)
                {
                    throw new RuntimeException("Server: only one port can be specified");
                }

                port = Integer.parseInt(arg);
            }

            if(port <= 0)
            {
                throw new RuntimeException("Server: no port specified");
            }

            // Don't move this, it needs the port.
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(port));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object object = new TestI(port);
            adapter.add(object, Ice.Util.stringToIdentity("test"));

            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
