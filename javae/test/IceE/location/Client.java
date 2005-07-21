// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    public static int
    run(String[] args, Ice.Communicator communicator, java.io.PrintStream out)
    {
	AllTests.allTests(communicator, out);
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
	    properties.setProperty("Ice.Default.Locator", "locator:default -p 12345");
	    //
	    // This test requires an extra retry interval because it is possible for
	    // a proxy to encounter two CloseConnectionExceptions in a row during
	    // retries. The first is raised by an already-closed connection, and the
	    // second occurs when a CloseConnection message is pending on the next
	    // connection that is tried.
	    //
	    properties.setProperty("Ice.RetryIntervals", "0 0");
            communicator = Ice.Util.initializeWithProperties(args, properties);
            status = run(args, communicator, System.out);
        }
        catch (Ice.LocalException ex)
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
            catch (Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

	System.gc();
        System.exit(status);
    }
}
