// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Consumer : Ice.Application
{
    public class AMI_Queue_getI : AMI_Queue_get
    {
        public override void ice_response(string message)
	{
	    Console.Out.WriteLine(message);
	}

	public override void ice_exception(Ice.Exception ex)
	{
	    Console.Error.WriteLine(ex);
	}
    }

    private static void menu()
    {
        Console.Out.WriteLine(
            "usage:\n" +
            "g: get a message\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        string proxyProperty = "Queue.Proxy";
        string proxy = properties.getProperty(proxyProperty);
        if(proxy.Length == 0)
        {
            Console.Error.WriteLine("property `" + proxyProperty + "' not set");
            return 1;
        }

        QueuePrx queue = QueuePrxHelper.checkedCast(communicator().stringToProxy(proxy));
        if(queue == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }

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
                if(line.Equals("g"))
                {
                    queue.get_async(new AMI_Queue_getI());
                }
                else if(line.Equals("x"))
                {
                    // Nothing to do
                }
                else
                {
                    Console.Out.WriteLine("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(Ice.Exception ex)
            {
	        Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));

        return 0;
    }

    public static void Main(string[] args)
    {
        Consumer app = new Consumer();
        int status = app.main(args, "config.client");
	if(status != 0)
	{
	    System.Environment.Exit(status);
	}
    }
}
