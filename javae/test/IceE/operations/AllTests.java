// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static Test.MyClassPrx
    allTests(Ice.Communicator communicator, Ice.InitializationData initData, java.io.PrintStream out)
    {
        out.flush();
        String ref = communicator.getProperties().getPropertyWithDefault("Test.Proxy", 
		"test:default -p 12010 -t 10000");
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(base);
        Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(cl);

	out.print("testing timeout... ");
	out.flush();
	try
	{
	    Test.MyClassPrx clTimeout = Test.MyClassPrxHelper.uncheckedCast(cl.ice_timeout(500));
	    clTimeout.opSleep(1000);
	    test(false);
	}
	catch(Ice.TimeoutException ex)
	{
	}
	out.println("ok");

        out.print("testing twoway operations... ");
        out.flush();
        Twoways.twoways(communicator, initData, cl);
        Twoways.twoways(communicator, initData, derived);
        derived.opDerived();
        out.println("ok");
	out.print("testing batch oneway operations... ");
	out.flush();
	BatchOneways.batchOneways(cl);
	BatchOneways.batchOneways(derived);
	out.println("ok");

        return cl;
    }
}
