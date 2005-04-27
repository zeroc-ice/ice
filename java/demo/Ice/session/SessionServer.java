// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionServer extends Ice.Application
{ 
    public int
    run(String[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionFactory");
	ReapThread reaper = new ReapThread();
	reaper.start();

        adapter.add(new SessionFactoryI(reaper), Ice.Util.stringToIdentity("SessionFactory"));
        adapter.activate();
        communicator().waitForShutdown();

    	reaper.terminate();
	try
	{
	    reaper.join();
	}
	catch(InterruptedException e)
	{
	}

        return 0;
    }
}
