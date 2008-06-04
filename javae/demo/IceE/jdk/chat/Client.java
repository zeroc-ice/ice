// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

        _router = Glacier2.RouterPrxHelper.checkedCast(defaultRouter);
        if(_router == null)
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
                session = ChatSessionPrxHelper.uncheckedCast(_router.createSession(id, pw));
                break;
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                ex.printStackTrace();
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                System.out.println("permission denied:\n" + ex.reason);
            }
        }

        _ping = new SessionPingThread(session, _router.getSessionTimeout() / 2);
        _ping.start();

        Ice.Identity callbackReceiverIdent = new Ice.Identity();
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = _router.getCategoryForClient();

        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithRouter("Chat.Client", defaultRouter);
        ChatCallbackI cb = new ChatCallbackI();
        ChatCallbackPrx callback = ChatCallbackPrxHelper.uncheckedCast(adapter.add(cb, callbackReceiverIdent));
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
                    menu();
                }
                else
                {
                    session.say(line);
                }
            }
            while(true);

            cleanup();
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            cleanup();
            return 1;
        }

        return 0;
    }

    private static void
    cleanup()
    {
        try
        {
            _router.destroySession();
        }
        catch(Glacier2.SessionNotExistException ex)
        {
            ex.printStackTrace();
        }
        catch(Ice.ConnectionLostException ex)
        {
            //
            // Expected: the router closed the connection.
            //
        }

        _ping.destroy();
        try
        {
            _ping.join();
        }
        catch(java.lang.InterruptedException ex)
        {
        }
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

    private static Glacier2.RouterPrx _router;
    private static SessionPingThread _ping;
}
