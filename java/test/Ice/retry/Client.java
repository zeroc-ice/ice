// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
        Test.RetryPrx retry = AllTests.allTests(communicator, out);
        retry.shutdown();
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
	    Ice.Properties properties = Ice.Util.getDefaultProperties(argsH);

	    //
	    // For this test, we want to disable retries.
	    //
	    properties.setProperty("Ice.RetryIntervals", "-1");

            communicator = Ice.Util.initialize(argsH);

	    //
	    // We don't want connection warnings because of the timeout test.
	    //
	    properties.setProperty("Ice.Warn.Connections", "0");
	    
            status = run(argsH.value, communicator, System.out);
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
