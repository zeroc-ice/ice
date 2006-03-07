// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
	    properties.setProperty("Ice.Default.Locator", "locator:default -p 12010");

	    if(properties.getPropertyAsInt("Ice.Blocking") > 0)
	    {
		properties.setProperty("Ice.RetryIntervals", "0 0");
		properties.setProperty("Ice.Warn.Connections", "0");
	    }

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
