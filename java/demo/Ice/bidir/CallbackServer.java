// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class CallbackServer extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Server");
        CallbackSenderI sender = new CallbackSenderI();
        adapter.add(sender, Ice.Util.stringToIdentity("sender"));
        adapter.activate();

	Thread t = new Thread(sender);
	t.start();

	try
	{
	    communicator().waitForShutdown();
	}
	finally
	{
	    sender.destroy();
	    try
	    {
		t.join();
	    }
	    catch(java.lang.InterruptedException ex)
	    {
	    }
	}

        return 0;
    }
}
