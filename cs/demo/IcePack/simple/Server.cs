// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Hello");
	string id = communicator().getProperties().getProperty("Identity");

        Ice.Object @object = new HelloI();
        adapter.add(@object, Ice.Util.stringToIdentity(id));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    static public void Main(string[] args)
    {
	Server app = new Server();
	app.main(args);
    }
}
