// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class AllTests
{
    private static void
    test(boolean b)
    {
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    public static TestPrx
    allTests(Ice.Communicator communicator)
    {
	System.out.print("testing stringToProxy... ");
	System.out.flush();
	String ref = "test @ TestAdapter";
	Ice.ObjectPrx base = communicator.stringToProxy(ref);
	test(base != null);
	System.out.println("ok");

	System.out.print("testing checked cast... ");
	System.out.flush();
	TestPrx obj = TestPrxHelper.checkedCast(base);
	test(obj != null);
	test(obj.equals(base));
	System.out.println("ok");
	
	System.out.print("pinging server... ");
	System.out.flush();
	obj.ice_ping();
	System.out.println("ok");
	
	return obj;
    }
}
