// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");

        String id = communicator().getProperties().getProperty("Identity");

        adapter.add(new HelloFactoryI(), Ice.Util.stringToIdentity(id));
        adapter.activate();

        communicator().waitForShutdown();
        return 0;
    }

    public static void Main(string[] args)
    {
        Server app = new Server();
        int status = app.main(args);
        Environment.Exit(status);
    }
}
