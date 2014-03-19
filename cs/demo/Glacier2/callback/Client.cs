// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    public class App : Glacier2.Application
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
                "r: restart the session\n" +
                "s: shutdown server\n" +
                "x: exit\n" +
                "?: help\n");
        }

        public override Glacier2.SessionPrx createSession()
        {
            Glacier2.SessionPrx session;
            while(true)
            {
                Console.WriteLine("This demo accepts any user-id / password combination.");

                String id;
                String pw;
                try
                {
                    Console.Write("user id: ");
                    Console.Out.Flush();
                    id = Console.In.ReadLine();
                    if(id == null)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }
                    id = id.Trim();

                    Console.Write("password: ");
                    Console.Out.Flush();
                    pw = Console.In.ReadLine().Trim();
                    if(pw == null)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }
                    pw = pw.Trim();
                }
                catch(System.IO.IOException ex)
                {
                    Console.WriteLine(ex.StackTrace.ToString());
                    continue;
                }

                try
                {
                    session = router().createSession(id, pw);
                    break;
                }
                catch(Glacier2.PermissionDeniedException ex)
                {
                    Console.WriteLine("permission denied:\n" + ex.reason);
                }
                catch(Glacier2.CannotCreateSessionException ex)
                {
                    Console.WriteLine("cannot create session:\n" + ex.reason);
                }
            }
            return session;
        }

        public override int runWithSession(string[] args)
        {
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }


            Ice.Identity callbackReceiverIdent = createCallbackIdentity("callbackReceiver");
            Ice.Identity callbackReceiverFakeIdent = new Ice.Identity("fake", "callbackReceiver");

            Ice.ObjectPrx @base = communicator().propertyToProxy("Callback.Proxy");
            CallbackPrx twoway = CallbackPrxHelper.checkedCast(@base);
            CallbackPrx oneway = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway());
            CallbackPrx batchOneway = CallbackPrxHelper.uncheckedCast(twoway.ice_batchOneway());

            objectAdapter().add(new CallbackReceiverI(), callbackReceiverFakeIdent);

            CallbackReceiverPrx twowayR = CallbackReceiverPrxHelper.uncheckedCast(
                                                    objectAdapter().add(new CallbackReceiverI(), callbackReceiverIdent));

            CallbackReceiverPrx onewayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway());

            menu();

            string line = null;
            string @override = null;
            bool fake = false;
            do
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
                else if(line.Equals("r"))
                {
                    restart();
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
            while(!line.Equals("x"));

            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
