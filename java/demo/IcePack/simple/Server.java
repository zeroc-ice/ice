// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
	String id = communicator().getProperties().getProperty("Identity");

        Ice.Object object = new HelloI();
        adapter.add(object, Ice.Util.stringToIdentity(id));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    static public void
    main(String[] args)
    {
	Server app = new Server();
	app.main("demo.IcePack.hello.Server", args);
    }
}
