// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Collections;

public class Client : Ice.Application
{
    private static void menu()
    {
        Console.WriteLine(
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

    public override int run(string[] args)
    {
        Ice.RouterPrx defaultRouter = communicator().getDefaultRouter();
        if(defaultRouter == null)
        {
            Console.Error.WriteLine("no default router set");
            return 1;
        }
        
        Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.checkedCast(defaultRouter);
        if(router == null)
        {
            Console.Error.WriteLine("configured router is not a Glacier2 router");
            return 1;
        }

        while(true)
        {
            Console.WriteLine("This demo accepts any user-id / password combination.");

            String id;
            Console.Write("user id: ");
            Console.Out.Flush();
            id = Console.In.ReadLine();
                
            String pw;
            Console.Write("password: ");
            Console.Out.Flush();
            pw = Console.In.ReadLine();
                
            try
            {
                router.createSession(id, pw);
                break;
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                Console.Write("permission denied:\n" + ex.reason);
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                Console.Write("cannot create session:\n" + ex.reason);
            }
        }

        String category = router.getCategoryForClient();
        Ice.Identity callbackReceiverIdent = new Ice.Identity();
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = category;
        Ice.Identity callbackReceiverFakeIdent = new Ice.Identity();
        callbackReceiverFakeIdent.name = "callbackReceiver";
        callbackReceiverFakeIdent.category = "fake";

        Ice.ObjectPrx @base = communicator().propertyToProxy("Callback.Proxy");
        CallbackPrx twoway = CallbackPrxHelper.checkedCast(@base);
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

        string line = null;
        string @override = null;
        bool fake = false;
        do
        {
            try
            {
                Console.Write("==> ");
                Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null)
                {
                    break;
                }
                if(line.Equals("t"))
                {
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "t";
                    if(@override != null)
                    {
                        context["_ovrd"] = @override;
                    }
                    twoway.initiateCallback(twowayR, context);
                }
                else if(line.Equals("o"))
                {
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "o";
                    if(@override != null)
                    {
                        context["_ovrd"] = @override;
                    }
                    oneway.initiateCallback(onewayR, context);
                }
                else if(line.Equals("O"))
                {
                    Ice.Context context = new Ice.Context();
                    context["_fwd"] = "O";
                    if(@override != null)
                    {
                        context["_ovrd"] = @override;
                    } 
                    batchOneway.initiateCallback(onewayR, context);
                }
                else if(line.Equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(line.Equals("v"))
                {
                    if(@override == null)
                    {
                        @override = "some_value";
                        Console.WriteLine("override context field is now `" + @override + "'");
                    }
                    else
                    {
                        @override = null;
                        Console.WriteLine("override context field is empty");
                    }
                }
                else if(line.Equals("F"))
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
                    
                    Console.WriteLine("callback receiver identity: " + 
                                      communicator().identityToString(twowayR.ice_getIdentity()));
                }
                else if(line.Equals("s"))
                {
                    twoway.shutdown();
                }
                else if(line.Equals("x"))
                {
                    // Nothing to do
                }
                else if(line.Equals("?"))
                {
                    menu();
                }
                else
                {
                    Console.WriteLine("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(System.Exception ex)
            {
                Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));

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
