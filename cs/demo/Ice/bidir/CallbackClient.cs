// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

class CallbackClient : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
	string proxyProperty = "Callback.Client.CallbackServer";
        string proxy = properties.getProperty(proxyProperty);
        if(proxy.Length == 0)
        {
	    System.Console.Error.WriteLine("property `" + proxyProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx @base = communicator().stringToProxy(proxy);
        CallbackSenderPrx server = CallbackSenderPrxHelper.checkedCast(@base);
        if(server == null)
        {
            System.Console.Error.WriteLine("invalid proxy");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Client");
	Ice.Identity ident = new Ice.Identity();
	ident.name = Ice.Util.generateUUID();
	ident.category = "";
        adapter.add(new CallbackReceiverI(), ident);
        adapter.activate();
	server.ice_connection().setAdapter(adapter);
	server.addClient(ident);
	communicator().waitForShutdown();

        return 0;
    }
}
