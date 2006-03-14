// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    private static void
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
            "S: switch secure mode on/off\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        final String proxyProperty = "Hello.Proxy";
        String proxy = properties.getProperty(proxyProperty);
        if(proxy.length() == 0)
        {
            System.err.println("property `" + proxyProperty + "' not set");
            return 1;
        }

        HelloPrx twoway = HelloPrxHelper.checkedCast(
	    communicator().stringToProxy(proxy).ice_twoway().ice_timeout(-1).ice_secure(false));
        if(twoway == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }
        HelloPrx oneway = HelloPrxHelper.uncheckedCast(twoway.ice_oneway());
        HelloPrx batchOneway = HelloPrxHelper.uncheckedCast(twoway.ice_batchOneway());
        HelloPrx datagram = HelloPrxHelper.uncheckedCast(twoway.ice_datagram());
        HelloPrx batchDatagram = HelloPrxHelper.uncheckedCast(twoway.ice_batchDatagram());

	boolean secure = false;
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
		    if(secure)
		    {
			System.out.println("secure datagrams are not supported");
		    }
		    else
		    {
			datagram.sayHello();
		    }
                }
                else if(line.equals("D"))
                {
		    if(secure)
		    {
			System.out.println("secure datagrams are not supported");
		    }
		    else
		    {
			batchDatagram.sayHello();
		    }
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
                else if(line.equals("S"))
                {
		    secure = !secure;

		    twoway = HelloPrxHelper.uncheckedCast(twoway.ice_secure(secure));
		    oneway = HelloPrxHelper.uncheckedCast(oneway.ice_secure(secure));
		    batchOneway = HelloPrxHelper.uncheckedCast(batchOneway.ice_secure(secure));
		    datagram = HelloPrxHelper.uncheckedCast(datagram.ice_secure(secure));
		    batchDatagram = HelloPrxHelper.uncheckedCast(batchDatagram.ice_secure(secure));

                    if(secure)
                    {
                        System.out.println("secure mode is now on");
                    }
                    else
                    {
                        System.out.println("secure mode is now off");
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
