// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class CallbackClient extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        final String proxyProperty = "Callback.Client.CallbackServer";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
        {
            System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator().stringToProxy(proxy);
        CallbackSenderPrx server = CallbackSenderPrxHelper.checkedCast(base);
        if(server == null)
        {
            System.err.println("invalid proxy");
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
