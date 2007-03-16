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

    public static Test.RetryPrx
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = communicator.getProperties().getPropertyWithDefault("Retry.Proxy", 
		"retry:default -p 12010 -t 10000");
        Ice.ObjectPrx base1 = communicator.stringToProxy(ref);
        test(base1 != null);
	Ice.ObjectPrx base2 = communicator.stringToProxy(ref);
        test(base2 != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        Test.RetryPrx retry1 = Test.RetryPrxHelper.checkedCast(base1);
        test(retry1 != null);
	test(retry1.equals(base1));
        Test.RetryPrx retry2 = Test.RetryPrxHelper.checkedCast(base2);
        test(retry2 != null);
	test(retry2.equals(base2));
        out.println("ok");

	out.print("calling regular operation with first proxy... ");
	out.flush();
	retry1.op(false);
	out.println("ok");

	out.print("calling operation to kill connection with second proxy... ");
	out.flush();
	try
	{
	    retry2.op(true);
	    test(false);
	}
	catch(Ice.ConnectionLostException ex)
	{
	    out.println("ok");
	}

        out.print("calling regular operation with first proxy again... ");
        out.flush();
	retry1.op(false);
	out.println("ok");

        return retry1;
    }
}
