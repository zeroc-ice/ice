// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Collocated
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object object = new MyDerivedClassI(adapter, Ice.Util.stringToIdentity("test"));
        adapter.add(object, Ice.Util.stringToIdentity("test"));
	adapter.activate();

	//
	// Make a separate adapter with a servant locator. We use this to test
	// that ::Ice::Context is correctly passed to checkedCast() operation.
	//
	communicator.getProperties().setProperty("CheckedCastAdapter.Endpoints", "default -p 12346 -t 10000");
	adapter = communicator.createObjectAdapter("CheckedCastAdapter");
	Ice.ServantLocator checkedCastLocator = new CheckedCastLocator();
	adapter.addServantLocator(checkedCastLocator, "");
	adapter.activate();

        AllTests.allTests(communicator);

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
