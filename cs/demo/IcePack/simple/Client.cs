// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client
{
    private static void menu()
    {
        Console.WriteLine(
            "usage:\n" +
            "t: send greeting as twoway\n" +
            "o: send greeting as oneway\n" +
            "O: send greeting as batch oneway\n" +
            "d: send greeting as datagram\n" +
            "D: send greeting as batch datagram\n" +
            "f: flush all batch requests\n" +
            "T: set a timeout\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();

	IcePack.QueryPrx query = IcePack.QueryPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Query"));

	Ice.ObjectPrx @base = null;
	try
	{
	    @base = query.findObjectByType("::Demo::Hello");
	}
	catch(IcePack.ObjectNotExistException)
	{
	}
        HelloPrx twoway = HelloPrxHelper.checkedCast(@base.ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            Console.WriteLine("invalid proxy");
            return 1;
        }
        HelloPrx oneway = HelloPrxHelper.uncheckedCast(twoway.ice_oneway());
        HelloPrx batchOneway = HelloPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        HelloPrx datagram = HelloPrxHelper.uncheckedCast(twoway.ice_datagram());
        HelloPrx batchDatagram = HelloPrxHelper.uncheckedCast(twoway.ice_batchDatagram());

        int timeout = -1;

        menu();

        string line = null;
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
                    datagram.sayHello();
                }
                else if(line.Equals("D"))
                {
                    batchDatagram.sayHello();
                }
                else if(line.Equals("f"))
                {
		    communicator.flushBatchRequests();
                }
                else if(line.Equals("T"))
                {
                    if(timeout == -1)
                    {
                        timeout = 2000;
                    }
                    else
                    {
                        timeout = -1;
                    }

                    twoway = HelloPrxHelper.uncheckedCast(twoway.ice_timeout(timeout));
                    oneway = HelloPrxHelper.uncheckedCast(oneway.ice_timeout(timeout));
                    batchOneway = HelloPrxHelper.uncheckedCast(batchOneway.ice_timeout(timeout));

                    if(timeout == -1)
                    {
                        Console.WriteLine("timeout is now switched off");
                    }
                    else
                    {
                        Console.WriteLine("timeout is now set to 2000ms");
                    }
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
            catch(Ice.LocalException ex)
            {
                Console.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));

        return 0;
    }

    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties();
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(ref args, properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            Console.WriteLine(ex);
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
                Console.WriteLine(ex);
                status = 1;
            }
        }

        Environment.Exit(status);
    }
}
