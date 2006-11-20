// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Consumer extends Ice.Application
{
    public class AMI_Queue_getI extends AMI_Queue_get
    {
        public AMI_Queue_getI(String id)
	{
	    _id = id;

	    synchronized(_requestMutex)
	    {
	        _requests.add(id);
	    }
	}

        public void ice_response(String message)
	{
	    synchronized(_requestMutex)
	    {
	        _requests.remove(_id);
	    }

	    System.out.println(message);
	}

	public void ice_exception(Ice.LocalException ex)
	{
	    synchronized(_requestMutex)
	    {
	        _requests.remove(_id);
	    }

	    ex.printStackTrace();
	}

	public void ice_exception(Ice.UserException ex)
	{
	    if(ex instanceof Demo.RequestCanceledException)
	    {
	        System.out.println("Request canceled");
	    }
	    else
	    {
	        ex.printStackTrace();
	    }
	}

	private String _id;
    }

    private static void
    menu()
    {
        System.out.println(
            "usage:\n" +
            "g: get a message\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        final String proxyProperty = "Queue.Proxy";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
        {
            System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        QueuePrx queue = QueuePrxHelper.checkedCast(communicator().stringToProxy(proxy));
        if(queue == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        menu();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        String line = null;
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
                if(line.equals("g"))
                {
		    String id = Ice.Util.generateUUID();
                    queue.get_async(new AMI_Queue_getI(id), id);
                }
                else if(line.equals("x"))
                {
                    // Nothing to do
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

	synchronized(_requestMutex)
	{
	    if(_requests.size() != 0)
	    {
	        try
		{
		    queue.cancel((String[])_requests.toArray(new String[0]));
		}
		catch(Ice.LocalException ex)
		{
		    // Igmore
		}
	    }
	}

        return 0;
    }

    public static void
    main(String[] args)
    {
        Consumer app = new Consumer();
        int status = app.main("Consumer", args, "config.client");
        System.exit(status);
    }

    private java.lang.Object _requestMutex = new java.lang.Object();
    private java.util.HashSet _requests = new java.util.HashSet();
}
