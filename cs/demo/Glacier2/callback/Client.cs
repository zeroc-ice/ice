// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Threading;
using System.Collections.Generic;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("Glacier2CallbackClient")]
[assembly: AssemblyDescription("Glacier2 callback demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        class SessionRefreshThread
        {
            public SessionRefreshThread(Glacier2.RouterPrx router, int timeout)
            {
                _router = router;
                _timeout = timeout;
                _terminated = false;
            }

            public void run()
            {
                lock(this)
                {
                    while(!_terminated)
                    {
                        Monitor.Wait(this, _timeout);
                        if(!_terminated)
                        {
                            try
                            {
                                _router.ice_ping();
                            }
                            catch(Ice.Exception)
                            {
                            }
                        }
                    }
                }
            }

            public void terminate()
            {
                lock(this)
                {
                    _terminated = true;
                    Monitor.Pulse(this);
                }
            }

            private Glacier2.RouterPrx _router;
            private int _timeout;
            private bool _terminated;
        }
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
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

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

            SessionRefreshThread refresh = new SessionRefreshThread(router, (int)router.getSessionTimeout() * 500);
            Thread refreshThread = new Thread(new ThreadStart(refresh.run));
            refreshThread.Start();

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

            Ice.ObjectAdapter adapter = communicator().createObjectAdapterWithRouter("Callback.Client", defaultRouter);
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
                        Dictionary<string, string> context = new Dictionary<string, string>();
                        context["_fwd"] = "t";
                        if(@override != null)
                        {
                            context["_ovrd"] = @override;
                        }
                        twoway.initiateCallback(twowayR, context);
                    }
                    else if(line.Equals("o"))
                    {
                        Dictionary<string, string> context = new Dictionary<string, string>();
                        context["_fwd"] = "o";
                        if(@override != null)
                        {
                            context["_ovrd"] = @override;
                        }
                        oneway.initiateCallback(onewayR, context);
                    }
                    else if(line.Equals("O"))
                    {
                        Dictionary<string, string> context = new Dictionary<string, string>();
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

            //
            // The refresher thread must be terminated before destroy is
            // called, otherwise it might get ObjectNotExistException. refresh
            // is set to 0 so that if session.destroy() raises an exception
            // the thread will not be re-terminated and re-joined.
            //
            refresh.terminate();
            refreshThread.Join();
            refresh = null;

            try
            {
                router.destroySession();
            }
            catch(Glacier2.SessionNotExistException ex)
            {
                Console.Error.WriteLine(ex);
            }
            catch(Ice.ConnectionLostException)
            {
                //
                // Expected: the router closed the connection.
                //
            }

            return 0;
        }
    }

    public static void Main(string[] args)
    {
        App app = new App();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
