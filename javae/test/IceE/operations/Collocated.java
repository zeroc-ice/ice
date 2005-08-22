// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Collocated
{
    public static int
    run(String[] args, Ice.Communicator communicator, java.io.PrintStream out)
    {
	communicator.getProperties().setProperty("Test.Proxy", "test:default -p 12345 -t 10000");
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
	Ice.Identity id = Ice.Util.stringToIdentity("test");
        adapter.add(new MyDerivedClassI(adapter, id), id);
        adapter.add(new TestCheckedCastI(), Ice.Util.stringToIdentity("context"));
        adapter.activate();

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
            Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
            Ice.Properties properties = Ice.Util.getDefaultProperties(argsH);

            //
            // We must set MessageSizeMax to an explicit values,
            // because we run tests to check whether
            // Ice.MemoryLimitException is raised as expected.
            //
            properties.setProperty("Ice.MessageSizeMax", "100");

            communicator = Ice.Util.initialize(args);
            status = run(args, communicator, System.out);
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
