
// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Collocated
{
    public static int
    run(String[] args, Ice.Communicator communicator, 
	Ice.InitializationData initData, java.io.PrintStream out)
    {
	communicator.getProperties().setProperty("Test.Proxy", "test:default -p 12010 -t 10000");
	communicator.getProperties().setProperty("Test.ProxyWithContext", "context:default -p 12010 -t 10000");
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
	Ice.Identity id = communicator.stringToIdentity("test");
        adapter.add(new MyDerivedClassI(adapter, id), id);
        adapter.add(new TestCheckedCastI(), communicator.stringToIdentity("context"));
        adapter.activate();

        AllTests.allTests(communicator, initData, out);

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
            // We must set MessageSizeMax to an explicit values,
            // because we run tests to check whether
            // Ice.MemoryLimitException is raised as expected.
            //
            initData.properties.setProperty("Ice.MessageSizeMax", "100");

            communicator = Ice.Util.initialize(args, initData);
            status = run(args, communicator, initData, System.out);
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
