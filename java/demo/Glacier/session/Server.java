// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


public class Server
{
    static class PermissionsVerifierI extends Glacier._PermissionsVerifierDisp
    {
        PermissionsVerifierI(Ice.ObjectAdapter adapter)
        {
            _adapter = adapter;
        }

        public boolean
        checkPermissions(String userId, String password, Ice.StringHolder reason, Ice.Current current)
        {
            return userId.equals(password);
        }

        private Ice.ObjectAdapter _adapter;
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("HelloSessionManager");

        //
        // Create the SessionManager.
        //
        Ice.Object object = new HelloSessionManagerI(adapter);
        adapter.add(object, Ice.Util.stringToIdentity("HelloSessionManager"));

        //
        // Create the PermissionsVerifier.
        //
        object = new PermissionsVerifierI(adapter);
        adapter.add(object, Ice.Util.stringToIdentity("PermissionsVerifier"));

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
        } finally {
	    if(communicator != null)
	    {
                communicator.destroy();
            }
	}

        System.exit(status);
    }
}
