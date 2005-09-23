// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    private void
    menu()
    {
        System.out.println(
            "usage:\n" +
            "t: send greeting as twoway\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();

	properties.parseCommandLineOptions("", args);

	//
	// First we try to connect to the object with the `hello'
	// identity. If it's not registered with the registry, we
	// search for an object with the ::Demo::Hello type.
	//
	Ice.ObjectPrx base = communicator().stringToProxy(properties.getPropertyWithDefault("Identity", "hello"));
	HelloPrx twoway = null;
	try
	{
	    twoway = HelloPrxHelper.checkedCast(base);
	}
	catch(Ice.NotRegisteredException ex)
	{
	    IceGrid.QueryPrx query =
		IceGrid.QueryPrxHelper.uncheckedCast(communicator().stringToProxy("IceGrid/Query"));
	    try
	    {
		twoway = HelloPrxHelper.checkedCast(query.findObjectByType("::Demo::Hello"));
	    }
	    catch(IceGrid.ObjectNotExistException ex1)
	    {
	    }
	}
	if(twoway == null)
	{
            System.err.println(": couldn't find a `::Demo::Hello' object.");
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
                if(line.equals("t"))
                {
                    twoway.sayHello();
                }
                else if(line.equals("s"))
                {
                    twoway.shutdown();
                }
                else if(line.equals("x"))
                {
                    // Nothing to do
                }
                else if(line.equals("?"))
                {
                    menu();
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

        return 0;
    }

    public static void
    main(String[] args)
    {
	Client app = new Client();
	int status = app.main("Client", args, "config");
	System.exit(status);
    }
}
