// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
        String endpts = "default -p 12345 -t 2000";
        Ice.ObjectAdapter adapter =
            communicator.createObjectAdapterWithEndpoints("TestAdapter",
                                                          endpts);
        Ice.Object d = new DI();
        adapter.add(d, Ice.Util.stringToIdentity("d"));
        d.ice_addFacet(d, "facetABCD");
        d.ice_addFacet(new FI(), "facetEF");
        d.ice_addFacet(new GI(communicator), "facetG");

        adapter.activate();
        communicator.waitForShutdown();

        d.ice_removeAllFacets(); // Break cyclic dependencies
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
