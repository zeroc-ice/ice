// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        final String proxyProperty = "Callback.Client.CallbackServer";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
        {
            System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator.stringToProxy(proxy);
        CallbackSenderPrx server = CallbackSenderPrxHelper.checkedCast(base);
        if(server == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Callback.Client");
	Ice.Identity ident = new Ice.Identity();
	ident.name = Ice.Util.generateUUID();
	ident.category = "";
        adapter.add(new CallbackReceiverI(), ident);
        adapter.activate();
	server.ice_connection().setAdapter(adapter);
	server.addClient(ident);
	communicator.waitForShutdown();

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(args, properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}
