// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    private static void
    usage(java.io.PrintStream out)
    {
        out.println("Usage: Client port...");
    }

    public static int
    run(String[] args, Ice.Communicator communicator, Ice.InitializationData data, java.io.PrintStream out)
    {
        java.util.Vector ports = new java.util.Vector(args.length);
        for(int i = 0; i < args.length; i++)
        {
            if(args[i].charAt(0) == '-')
            {
                //
                // TODO: Arguments recognized by the communicator are not
                // removed from the argument list.
                //
                // out.println("Client: unknown option `" + args[i] + "'");
                // usage();
                // return 1;
                continue;
            }

            int port = 0;
            try
            {
                port = Integer.parseInt(args[i]);
            }
            catch(NumberFormatException ex)
            {
                ex.printStackTrace();
                return 1;
            }
            ports.addElement(new Integer(port));
        }

        if(ports.isEmpty())
        {
	    //
	    // MIDlets won't have command line options, but they can
	    // configure a port range by specificying a start port and a
	    // number of ports to configure.
	    //
	    String startPort = communicator.getProperties().getProperty("Test.FirstPort");
	    String nPorts = communicator.getProperties().getProperty("Test.ServerCount");

	    int firstPort = 0;
	    int n = 0;
	    try
	    {
		firstPort = Integer.parseInt(startPort);
		n = Integer.parseInt(nPorts);
	    }
            catch(NumberFormatException ex)
            {
                ex.printStackTrace();
                return 1;
            }

	    for(int i = 0; i < n; ++i)
	    {
		ports.addElement(new Integer(firstPort++));
	    }
        }

        if(ports.isEmpty())
        {
            out.println("Client: no ports specified");
            usage(out);
            return 1;
	}

        int[] arr = new int[ports.size()];
        for(int i = 0; i < arr.length; i++)
        {
            arr[i] = ((Integer)ports.elementAt(i)).intValue();
        }

	try
	{
	    AllTests.allTests(communicator, arr, out);
	}
	catch(Ice.LocalException ex)
	{
	    ex.printStackTrace();
	    AllTests.test(false);
	}

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
	    Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	    Ice.InitializationData initData = new Ice.InitializationData();
	    initData.properties = Ice.Util.createProperties(argsH);
	
	    //
	    // This test aborts servers, so we don't want warnings.
	    //
	    initData.properties.setProperty("Ice.Warn.Connections", "0");

            communicator = Ice.Util.initialize(argsH, initData);
            status = run(argsH.value, communicator, initData, System.out);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
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
                ex.printStackTrace();
                status = 1;
            }
        }

	System.gc();
        System.exit(status);
    }
}
