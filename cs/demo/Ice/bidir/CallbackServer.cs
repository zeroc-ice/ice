// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System.Threading;

class CallbackServer : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Server");
        CallbackSenderI sender = new CallbackSenderI();
        adapter.add(sender, Ice.Util.stringToIdentity("sender"));
        adapter.activate();

	Thread t = new Thread(new ThreadStart(sender.Run));
	t.Start();

	try
	{
	    communicator().waitForShutdown();
	}
	finally
	{
	    sender.destroy();
	    t.Join();
	}

        return 0;
    }
}
