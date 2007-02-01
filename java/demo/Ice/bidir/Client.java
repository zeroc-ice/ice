// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    public int
    run(String[] args)
    {
        CallbackSenderPrx server = 
            CallbackSenderPrxHelper.checkedCast(communicator().propertyToProxy("Callback.Client.CallbackServer"));
        if(server == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("");
        Ice.Identity ident = new Ice.Identity();
        ident.name = Ice.Util.generateUUID();
        ident.category = "";
        adapter.add(new CallbackReceiverI(), ident);
        adapter.activate();
        server.ice_getConnection().setAdapter(adapter);
        server.addClient(ident);
        communicator().waitForShutdown();

        return 0;
    }

    public static void
    main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }
}
