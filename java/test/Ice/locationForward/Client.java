// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Client
{
    private static void
    usage()
    {
        System.err.println("Usage: Client port port");
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        int port = 0;
        int lastPort = 0;
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

            if(port == 0)
            {
                try
                {
                    port = Integer.parseInt(args[i]);
                }
                catch(NumberFormatException ex)
                {
                    ex.printStackTrace();
                    return 1;
                }
            }
            else if(lastPort == 0)
            {
                try
                {
                    lastPort = Integer.parseInt(args[i]);
                }
                catch(NumberFormatException ex)
                {
                    ex.printStackTrace();
                    return 1;
                }
            }
            else
            {
                System.err.println("Client: only two ports can be specified");
                usage();
                return 1;
            }
        }

        if(port == 0 || lastPort == 0)
        {
            System.err.println("Client: no ports specified");
            usage();
            return 1;
        }

        AllTests.allTests(communicator, port, lastPort);
        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator);
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

        System.exit(status);
    }
}
