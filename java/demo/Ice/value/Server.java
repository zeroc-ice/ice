// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Server
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter =
            communicator.createObjectAdapter("ValueAdapter");
        Ice.Object object = new InitialI(adapter);
        adapter.add(object, Ice.Util.stringToIdentity("initial"));
        adapter.activate();
        communicator.waitForShutdown();
        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties =
                Ice.Util.createPropertiesFromFile(args, "config");
            communicator = Ice.Util.initializeWithProperties(properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if (communicator != null)
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
