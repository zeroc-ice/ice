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
    static class PasswordVerifierI extends Glacier._PasswordVerifierDisp
    {
        PasswordVerifierI(Ice.ObjectAdapter adapter)
        {
            _adapter = adapter;
        }

        public boolean
        checkPassword(String userId, String password, Ice.Current current)
        {
            return userId.equals(password);
        }

        private Ice.ObjectAdapter _adapter;
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("HelloSessionManagerAdapter");

        //
        // Create the SessionManager.
        //
        Ice.Object object = new HelloSessionManagerI(adapter);
        adapter.add(object, Ice.Util.stringToIdentity("HelloSessionManager"));

        //
        // Create the PasswordVerifier.
        //
        object = new PasswordVerifierI(adapter);
        adapter.add(object, Ice.Util.stringToIdentity("PasswordVerifier"));

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
            Ice.Properties properties = Ice.Util.createProperties(args);
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(args, properties);
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
