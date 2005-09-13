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

    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();

	IcePack.QueryPrx query = IcePack.QueryPrxHelper.checkedCast(communicator().stringToProxy("IcePack/Query"));

	Ice.ObjectPrx base = null;
	try
	{
	    base = query.findObjectByType("::Demo::Hello");
	}
	catch(IcePack.ObjectNotExistException e)
	{
	}
        HelloPrx twoway = HelloPrxHelper.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }
        HelloPrx oneway = HelloPrxHelper.uncheckedCast(twoway.ice_oneway());
        HelloPrx batchOneway = HelloPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        HelloPrx datagram = HelloPrxHelper.uncheckedCast(twoway.ice_datagram());
        HelloPrx batchDatagram = HelloPrxHelper.uncheckedCast(twoway.ice_batchDatagram());

        int timeout = -1;

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
                else if(line.equals("o"))
                {
                    oneway.sayHello();
                }
                else if(line.equals("O"))
                {
                    batchOneway.sayHello();
                }
                else if(line.equals("d"))
                {
                    datagram.sayHello();
                }
                else if(line.equals("D"))
                {
                    batchDatagram.sayHello();
                }
                else if(line.equals("f"))
                {
		    communicator().flushBatchRequests();
                }
                else if(line.equals("T"))
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
                        System.out.println("timeout is now switched off");
                    }
                    else
                    {
                        System.out.println("timeout is now set to 2000ms");
                    }
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
