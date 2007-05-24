// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;
using Test;

public class Client : Ice.Application
{
    public override int run(string[] args)
    {
        Console.Out.Write("getting router... ");
        Console.Out.Flush();
        Ice.ObjectPrx routerBase = communicator().stringToProxy("Glacier2/router:default -p 12347 -t 10000");
        Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.checkedCast(routerBase);
        test(router != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("creating session... ");
        Console.Out.Flush();
        router.createSession("userid", "abc123");
        communicator().setDefaultRouter(router);
        Console.Out.WriteLine("ok");

        Console.Out.Write("making thousands of invocations on proxies... ");
        Console.Out.Flush();
        Ice.ObjectPrx backendBase = communicator().stringToProxy("dummy:tcp -p 12010 -t 10000");
        BackendPrx backend = BackendPrxHelper.uncheckedCast(backendBase);
        backend.ice_ping();

        Hashtable backends = new Hashtable();
        Random rand = new Random(unchecked((int)DateTime.Now.Ticks));

        String msg = "";
        for(int i = 1; i <= 10000; ++i)
        {
            if(i % 100 == 0)
            {
                for(int j = 0; j < msg.Length; ++j)
                {
                    Console.Out.Write('\b');
                }

                msg = "" + i;
                Console.Out.Write(i);
                Console.Out.Flush();
            }

            Ice.Identity ident = new Ice.Identity("", "");
            ident.name += (char)('A' + rand.Next() % 26);

            int len = rand.Next() % 2;
            for(int j = 0; j < len; ++j)
            {
                ident.category += (char)('a' + rand.Next() % 26);
            }

            BackendPrx newBackend = BackendPrxHelper.uncheckedCast(backendBase.ice_identity(ident));
            
            if(!backends.ContainsKey(newBackend))
            {
                backends.Add(newBackend, newBackend);
                backend = newBackend;
            }
            else
            {
                backend = (BackendPrx)backends[newBackend];
            }

            backend.ice_ping();
        }

        for(int j = 0; j < msg.Length; ++j)
        {
            Console.Out.Write('\b');
        }
        for(int j = 0; j < msg.Length; ++j)
        {
            Console.Out.Write(' ');
        }
        for(int j = 0; j < msg.Length; ++j)
        {
            Console.Out.Write('\b');
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing server and router shutdown... ");
        Console.Out.Flush();
        backend.shutdown();
        communicator().setDefaultRouter(null);
        Ice.ObjectPrx adminBase = communicator().stringToProxy("Glacier2/admin:tcp -h 127.0.0.1 -p 12348 -t 10000");
        Glacier2.AdminPrx admin = Glacier2.AdminPrxHelper.checkedCast(adminBase);
        test(admin != null);
        admin.shutdown();
        try
        {
           admin.ice_ping();
           test(false);
        }
        catch(Ice.LocalException)
        {
            Console.Out.WriteLine("ok");
        }

        return 0;
    }

    private static void
    test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public static void Main(string[] args)
    {
        Debug.Listeners.Add(new ConsoleTraceListener());

        //
        // We want to check whether the client retries for evicted
        // proxies, even with regular retries disabled.
        //
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(ref args);
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        Client app = new Client();
        int status = app.main(args, initData);

        if(status != 0)
        {
            Environment.Exit(status);
        }
    }
}
