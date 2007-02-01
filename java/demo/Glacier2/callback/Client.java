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
    class ShutdownHook extends Thread
    {
        public void
        run()
        {
            try
            {
                communicator().destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
    }

    private static void
    menu()
    {
        System.out.println(
            "usage:\n" +
            "t: send callback as twoway\n" +
            "o: send callback as oneway\n" +
            "O: send callback as batch oneway\n" +
            "f: flush all batch requests\n" +
            "v: set/reset override context field\n" +
            "F: set/reset fake category\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        //
        // Since this is an interactive demo we want to clear the
        // Application installed interrupt callback and install our
        // own shutdown hook.
        //
        setInterruptHook(new ShutdownHook());

        Ice.RouterPrx defaultRouter = communicator().getDefaultRouter();
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
        while(true)
        {
            System.out.println("This demo accepts any user-id / password combination.");

            try
            {
                String id;
                System.out.print("user id: ");
                System.out.flush();
                id = in.readLine();
                
                String pw;
                System.out.print("password: ");
                System.out.flush();
                pw = in.readLine();
                
                try
                {
                    router.createSession(id, pw);
                    break;
                }
                catch(Glacier2.PermissionDeniedException ex)
                {
                    System.out.println("permission denied:\n" + ex.reason);
                }
                catch(Glacier2.CannotCreateSessionException ex)
                {
                    System.out.println("cannot create session:\n" + ex.reason);
                }
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
        }

        String category = router.getCategoryForClient();
        Ice.Identity callbackReceiverIdent = new Ice.Identity();
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = category;
        Ice.Identity callbackReceiverFakeIdent = new Ice.Identity();
        callbackReceiverFakeIdent.name = "callbackReceiver";
        callbackReceiverFakeIdent.category = "fake";

        Ice.ObjectPrx base = communicator().propertyToProxy("Callback.Proxy");
        CallbackPrx twoway = CallbackPrxHelper.checkedCast(base);
        CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
        CallbackPrx batchOneway = CallbackPrxHelper.uncheckedCast(twoway.ice_batchOneway());

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Client");
        adapter.add(new CallbackReceiverI(), callbackReceiverIdent);
        adapter.add(new CallbackReceiverI(), callbackReceiverFakeIdent);
        adapter.activate();

        CallbackReceiverPrx twowayR = CallbackReceiverPrxHelper.uncheckedCast(
            adapter.createProxy(callbackReceiverIdent));
        CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());

        menu();

        String line = null;
        String override = null;
        boolean fake = false;
        do
        {
            try
            {
                System.out.print("==> ");
                System.out.flush();
                line = in.readLine();
                if(line == null)
                {
                    break;
                }
                if(line.equals("t"))
                {
                    java.util.Map context = new java.util.HashMap();
                    context.put("_fwd", "t");
                    if(override != null)
                    {
                        context.put("_ovrd", override);
                    }
                    twoway.initiateCallback(twowayR, context);
                }
                else if(line.equals("o"))
                {
                    java.util.Map context = new java.util.HashMap();
                    context.put("_fwd", "o");
                    if(override != null)
                    {
                        context.put("_ovrd", override);
                    }
                    oneway.initiateCallback(onewayR, context);
                }
                else if(line.equals("O"))
                {
                    java.util.Map context = new java.util.HashMap();
                    context.put("_fwd", "O");
                    if(override != null)
                    {
                        context.put("_ovrd", override);
                    } 
                    batchOneway.initiateCallback(onewayR, context);
                }
                else if(line.equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(line.equals("v"))
                {
                    if(override == null)
                    {
                        override = "some_value";
                        System.out.println("override context field is now `" + override + "'");
                    }
                    else
                    {
                        override = null;
                        System.out.println("override context field is empty");
                    }
                }
                else if(line.equals("F"))
                {
                    fake = !fake;
                    
                    if(fake)
                    {
                        twowayR = CallbackReceiverPrxHelper.uncheckedCast(
                            twowayR.ice_identity(callbackReceiverFakeIdent));
                        onewayR = CallbackReceiverPrxHelper.uncheckedCast(
                            onewayR.ice_identity(callbackReceiverFakeIdent));
                    }
                    else
                    {
                        twowayR = CallbackReceiverPrxHelper.uncheckedCast(
                            twowayR.ice_identity(callbackReceiverIdent));
                        onewayR = CallbackReceiverPrxHelper.uncheckedCast(
                            onewayR.ice_identity(callbackReceiverIdent));
                    }
                    
                    System.out.println("callback receiver identity: " + 
                                       communicator().identityToString(twowayR.ice_getIdentity()));
                }
                else if(line.equals("s"))
                {
                    twoway.shutdown();
                }
                else if(line.equals("x"))
                {
                    // Nothing to do
                }
                else if(line.equals("?"))
                {
                    menu();
                }
                else
                {
                    System.out.println("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
        while(!line.equals("x"));

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
