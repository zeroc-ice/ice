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

public class Publisher : Ice.Application
{
    private static void menu()
    {
        Console.Out.WriteLine("Enter /quit to exit.");
    }

    public override int run(string[] args)
    {
        QueuePrx queue = QueuePrxHelper.checkedCast(communicator().propertyToProxy("Queue.Proxy"));
        if(queue == null)
        {
            Console.Error.WriteLine("invalid proxy");
            return 1;
        }

	Console.Out.WriteLine("Type a message and hit return to queue a message.");
        menu();

        string line = null;
	try
	{
            while(true)
            {
                Console.Out.Write("==> ");
		Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null)
                {
                    break;
                }
                if(line.Length != 0)
                {
		    if(line[0] == '/')
		    {
		        if(line.Equals("/quit"))
			{
			    break;
			}
		        menu();
		    }
		    else
		    {
		        queue.add(line);
		    }
                }
            }
	}
        catch(System.Exception ex)
        {
	    Console.Error.WriteLine(ex);
        }

        return 0;
    }

    public static void Main(string[] args)
    {
        Publisher app = new Publisher();
        int status = app.main(args, "config.client");
	if(status != 0)
	{
	    System.Environment.Exit(status);
	}
    }
}
