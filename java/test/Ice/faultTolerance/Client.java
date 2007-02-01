// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    private static void
    usage()
    {
        System.err.println("Usage: Client port...");
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        java.util.ArrayList ports = new java.util.ArrayList(args.length);
        for(int i = 0; i < args.length; i++)
        {
            if(args[i].charAt(0) == '-')
            {
                //
                // TODO: Arguments recognized by the communicator are not
                // removed from the argument list.
                //
                //System.err.println("Client: unknown option `" + args[i] + "'");
                //usage();
                //return 1;
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
            ports.add(new Integer(port));
        }

        if(ports.isEmpty())
        {
            System.err.println("Client: no ports specified");
            usage();
            return 1;
        }

        int[] arr = new int[ports.size()];
        for(int i = 0; i < arr.length; i++)
        {
            arr[i] = ((Integer)ports.get(i)).intValue();
        }

        try
        {
            AllTests.allTests(communicator, arr);
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
           initData. properties.setProperty("Ice.Warn.Connections", "0");

           communicator = Ice.Util.initialize(argsH, initData);
            status = run(argsH.value, communicator);
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
