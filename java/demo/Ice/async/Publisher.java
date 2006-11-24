// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Publisher extends Ice.Application
{
    private static void
    menu()
    {
        System.out.println("Enter /quit to exit.");
    }

    public int
    run(String[] args)
    {
        QueuePrx queue = QueuePrxHelper.checkedCast(communicator().propertyToProxy("Queue.Proxy"));
        if(queue == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

	System.out.println("Type a message and hit return to queue a message.");
        menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        String line = null;
	try
	{
            while(true)
            {
                System.out.print("==> ");
                System.out.flush();
                line = in.readLine().trim();
                if(line == null)
                {
                    break;
                }
                if(line.length() != 0)
                {
		    if(line.charAt(0) == '/')
		    {
		        if(line.equals("/quit"))
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
        catch(java.io.IOException ex)
        {
            ex.printStackTrace();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
        }

        return 0;
    }

    public static void
    main(String[] args)
    {
        Publisher app = new Publisher();
        int status = app.main("Publisher", args, "config.client");
        System.exit(status);
    }
}
