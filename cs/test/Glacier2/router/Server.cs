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
	communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "tcp -p 12345 -t 10000");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
	adapter.add(new CallbackI(),
		    Ice.Util.stringToIdentity("c1/callback")); // The test allows "c1" as category.
	adapter.add(new CallbackI(),
		    Ice.Util.stringToIdentity("c2/callback")); // The test allows "c2" as category.
	adapter.add(new CallbackI(),
		    Ice.Util.stringToIdentity("c3/callback")); // The test rejects "c3" as category.
	adapter.add(new CallbackI(),
		    Ice.Util.stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
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
