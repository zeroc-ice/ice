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
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Test.MyClassPrx myClass = AllTests.allTests(communicator, false);

        System.out.print("testing server shutdown... ");
        System.out.flush();
        myClass.shutdown();
        try
        {
            myClass.opVoid();
            throw new RuntimeException();
        }
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
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
	    //
	    // In this test, we need at least two threads in the
	    // client side thread pool for nested AMI.
	    //
	    Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	    Ice.Properties properties = Ice.Util.getDefaultProperties(argsH);
	    properties.setProperty("Ice.ThreadPool.Client.Size", "2");
	    properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

	    //
	    // We must set MessageSizeMax to an explicit values,
	    // because we run tests to check whether
	    // Ice.MemoryLimitException is raised as expected.
	    //
	    properties.setProperty("Ice.MessageSizeMax", "100");

            communicator = Ice.Util.initialize(argsH);
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
