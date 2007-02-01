// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        argsH.value = communicator().getProperties().parseCommandLineOptions("TestAdapter", argsH.value);

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.Object object = new TestI(adapter);
        adapter.add(object, communicator().stringToIdentity("test"));
        shutdownOnInterrupt();
        try
        {
            adapter.activate();
        }
        catch(Ice.ObjectAdapterDeactivatedException ex)
        {
        }
        communicator().waitForShutdown();
        defaultInterrupt();
        return 0;
    }

    public static void
    main(String[] args)
    {
        Server server = new Server();
        int status = server.main("test.IceGrid.simple.Server", args);
        System.gc();
        System.exit(status);
    }
}
