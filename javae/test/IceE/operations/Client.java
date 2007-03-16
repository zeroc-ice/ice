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
    public static int
    run(String[] args, Ice.Communicator communicator, Ice.InitializationData initData, java.io.PrintStream out)
    {
        Test.MyClassPrx myClass = AllTests.allTests(communicator, initData, out);

        out.print("testing server shutdown... ");
        out.flush();
        myClass.shutdown();
        try
        {
            myClass.opVoid();
            throw new RuntimeException();
        }
        catch(Ice.LocalException ex)
        {
            out.println("ok");
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
	    // We must set MessageSizeMax to an explicit value,
	    // because we run tests to check whether
	    // Ice.MemoryLimitException is raised as expected.
	    //
	    initData.properties.setProperty("Ice.MessageSizeMax", "100");

	    //
	    // We don't want connection warnings because of the timeout test.
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
