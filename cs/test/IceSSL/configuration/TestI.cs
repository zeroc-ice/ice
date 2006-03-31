// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Collections;

internal sealed class ServerI : ServerDisp_
{
    internal ServerI(Ice.Communicator communicator)
    {
	communicator_ = communicator;
    }

    internal void destroy()
    {
	communicator_.destroy();
    }

    private Ice.Communicator communicator_;
}

internal sealed class ServerFactoryI : ServerFactoryDisp_
{
    private static void test(bool b)
    {
	if (!b)
	{
	    throw new Exception();
	}
    }

    public override ServerPrx createServer(Properties props, Ice.Current current)
    {
	Ice.Properties properties = Ice.Util.createProperties();
	foreach(string key in props.Keys)
	{
	    properties.setProperty(key, props[key]);
	}

	string[] args = new string[0];
	Ice.Communicator communicator = Ice.Util.initializeWithProperties(ref args, properties);
	Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
	ServerI server = new ServerI(communicator);
	Ice.ObjectPrx obj = adapter.addWithUUID(server);
	servers_[obj.ice_getIdentity()] = server;
	adapter.activate();

	return ServerPrxHelper.uncheckedCast(obj);
    }

    public override void destroyServer(ServerPrx srv, Ice.Current current)
    {
	Ice.Identity key = srv.ice_getIdentity();
	if(servers_.Contains(key))
	{
	    ServerI server = servers_[key] as ServerI;
	    server.destroy();
	    servers_.Remove(key);
	}
    }

    public override void shutdown(Ice.Current current)
    {
	test(servers_.Count == 0);
	current.adapter.getCommunicator().shutdown();
    }

    private Hashtable servers_ = new Hashtable();
}
