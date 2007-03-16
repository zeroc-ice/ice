// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client
{
    private static void
    menu()
    {
        System.out.println("enter /quit to exit.");
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.RouterPrx defaultRouter = communicator.getDefaultRouter();
	if(defaultRouter == null)
	{
	    System.err.println("no default router set");
	    return 1;
	}

	Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.checkedCast(defaultRouter);
	if(router == null)
	{
	    System.err.println("configured router is not a Glacier2 router");
	    return 1;
	}

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

	ChatSessionPrx session = null;
	while(true)
	{
	    System.out.println("This demo accepts any user-id / password combination.");

	    String id = null;
	    String pw = null;

	    try
	    {
	        System.out.print("user id: ");
                System.out.flush();
	        id = in.readLine();
	        id = id.trim();

	        System.out.print("password: ");
                System.out.flush();
	        pw = in.readLine();
	        pw = pw.trim();
	    }
	    catch(java.io.IOException ex)
	    {
	        ex.printStackTrace();
	        return 1;
	    }

	    try
	    {
	        session = ChatSessionPrxHelper.uncheckedCast(router.createSession(id, pw));
		break;
	    }
	    catch(Glacier2.CannotCreateSessionException ex)
	    {
	        ex.printStackTrace();
	    }
	    catch(Glacier2.PermissionDeniedException ex)
	    {
	        ex.printStackTrace();
	    }
	}

	SessionPingThread ping = new SessionPingThread(session, router.getSessionTimeout() / 2);
	ping.start();

	String category = router.getServerProxy().ice_getIdentity().category;
	Ice.Identity callbackReceiverIdent = new Ice.Identity();
	callbackReceiverIdent.name = "callbackReceiver";
	callbackReceiverIdent.category = category;

	Ice.ObjectAdapter adapter = communicator.createObjectAdapter("Chat.Client");
	ChatCallbackPrx callback = ChatCallbackPrxHelper.uncheckedCast(
	    adapter.add(new ChatCallbackI(), callbackReceiverIdent));
	adapter.activate();

	session.setCallback(callback);

        menu();

	try
	{
            String line = null;
            do
            {
                System.out.print("==> ");
                System.out.flush();
                line = in.readLine();
                if(line == null)
                {
                    break;
                }
		line = line.trim();

                if(line.startsWith("/"))
		{
		    if(line.equals("/quit"))
                    {
		        break;
		    }
		    else
		    {
		        menu();
		    }
                }
                else
                {
                    session.say(line);
                }
            }
            while(true);

	    try
	    {
		router.destroySession();
	    }
	    catch(Ice.ConnectionLostException ex)
	    {
		//
		// Expected: the router closed the connection.
		//
	    }
	}
	catch(Exception ex)
	{
	    ex.printStackTrace();
	}

	ping.destroy();
	try
	{
	    ping.join();
	}
	catch(java.lang.InterruptedException ex)
	{
	}
        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
	    Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties();
            initData.properties.load("config");
            communicator = Ice.Util.initialize(args, initData);
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
