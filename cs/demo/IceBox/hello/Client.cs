// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client : Ice.Application
{
    private static void menu()
    {
        Console.Write(
            "usage:\n" +
            "t: send greeting as twoway\n" +
            "o: send greeting as oneway\n" +
            "O: send greeting as batch oneway\n" +
            "d: send greeting as datagram\n" +
            "D: send greeting as batch datagram\n" +
            "f: flush all batch requests\n");
        if(_haveSSL)
        {
            Console.Write("\nS: switch secure mode on/off");
        }
        Console.WriteLine(
            "\nx: exit\n" +
            "?: help\n");
    }
    
    public override int run(string[] args)
    {
        try
        {
            communicator().getPluginManager().getPlugin("IceSSL");
            _haveSSL = true;
        }
        catch(Ice.NotRegisteredException)
        {
        }

        HelloPrx twoway = HelloPrxHelper.checkedCast(
            communicator().propertyToProxy("Hello.Proxy").ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }
        HelloPrx oneway = HelloPrxHelper.uncheckedCast(twoway.ice_oneway());
        HelloPrx batchOneway = HelloPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        HelloPrx datagram = HelloPrxHelper.uncheckedCast(twoway.ice_datagram());
        HelloPrx batchDatagram = HelloPrxHelper.uncheckedCast(twoway.ice_batchDatagram());
        
        bool secure = false;
        
        menu();
        
        string line = null;
        do 
        {
            try
            {
                Console.Out.Write("==> ");
                Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null)
                {
                    break;
                }
                if(line.Equals("t"))
                {
                    twoway.sayHello();
                }
                else if(line.Equals("o"))
                {
                    oneway.sayHello();
                }
                else if(line.Equals("O"))
                {
                    batchOneway.sayHello();
                }
                else if(line.Equals("d"))
                {
                    if(secure)
                    {
                        Console.WriteLine("secure datagrams are not supported");
                    }
                    else
                    {
                        datagram.sayHello();
                    }
                }
                else if(line.Equals("D"))
                {
                    if(secure)
                    {
                        Console.WriteLine("secure datagrams are not supported");
                    }
                    else
                    {
                        batchDatagram.sayHello();
                    }
                }
                else if(line.Equals("f"))
                {
                    communicator().flushBatchRequests();
                }
                else if(_haveSSL && line.Equals("S"))
                {
                    secure = !secure;

                    twoway = HelloPrxHelper.uncheckedCast(twoway.ice_secure(secure));
                    oneway = HelloPrxHelper.uncheckedCast(oneway.ice_secure(secure));
                    batchOneway = HelloPrxHelper.uncheckedCast(batchOneway.ice_secure(secure));
                    datagram = HelloPrxHelper.uncheckedCast(datagram.ice_secure(secure));
                    batchDatagram = HelloPrxHelper.uncheckedCast(batchDatagram.ice_secure(secure));
                    
                    if(secure)
                    {
                        Console.WriteLine("secure mode is now on");
                    }
                    else
                    {
                        Console.WriteLine("secure mode is now off");
                    }
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
        while (!line.Equals("x"));
        
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

    private static bool _haveSSL = false;
}
