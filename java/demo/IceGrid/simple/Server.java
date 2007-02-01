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
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
        Ice.Properties properties = communicator().getProperties();
        Ice.Identity id = communicator().stringToIdentity(properties.getProperty("Identity"));
        adapter.add(new HelloI(properties.getProperty("Ice.ServerId")), id);
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    static public void
    main(String[] args)
    {
        Server app = new Server();
        int status = app.main("Server", args);
        System.exit(status);
    }
}
