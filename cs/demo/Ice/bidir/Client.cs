// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class Client : Ice.Application
{
    public override int run(string[] args)
    {
        CallbackSenderPrx server = 
            CallbackSenderPrxHelper.checkedCast(communicator().propertyToProxy("Callback.Client.CallbackServer"));
        if(server == null)
        {
            System.Console.Error.WriteLine("invalid proxy");
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

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
