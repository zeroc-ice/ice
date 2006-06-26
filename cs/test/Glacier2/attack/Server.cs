// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public sealed class ServantLocatorI : Ice.LocalObjectImpl, Ice.ServantLocator
{
    public ServantLocatorI()
    {
        _backend = new BackendI();
    }

    public Ice.Object locate(Ice.Current curr, out Ice.LocalObject cookie)
    {
        cookie = null;
        return _backend;
    }

    public void finished(Ice.Current curr, Ice.Object servant, Ice.LocalObject cookie)
    {
    }

    public void deactivate(string category)
    {
    }

    private Backend _backend;
}

class Server : Ice.Application
{
    public override int run(string[] args)
    {
	communicator().getProperties().setProperty("BackendAdapter.Endpoints", "tcp -p 12010 -t 10000");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("BackendAdapter");
	adapter.addServantLocator(new ServantLocatorI(), "");
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static void Main(string[] args)
    {
        Server app = new Server();
	int status = app.main(args);
	if(status != 0)
	{
	    System.Environment.Exit(status);
	}
    }
}
