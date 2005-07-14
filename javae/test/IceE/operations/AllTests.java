// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    allTests(Ice.Communicator communicator)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test:default -p 12345 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(base);
        test(cl != null);
        Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(cl);
        test(derived != null);
        test(cl.equals(base));
        test(derived.equals(base));
        test(cl.equals(derived));
        System.out.println("ok");

	System.out.print("testing checked cast with context... ");
	System.out.flush();
	String cref = "test:default -p 12346 -t 10000";
	Ice.ObjectPrx cbase = communicator.stringToProxy(cref);
	test(cbase != null);

	Test.TestCheckedCastPrx tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase);
	java.util.Hashtable c = tccp.getContext();
	test(c == null || c.size() == 0);

	c = new java.util.Hashtable();
	c.put("one", "hello");
	c.put("two", "world");
	tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase, c);
	java.util.Hashtable c2 = tccp.getContext();
	test(c.equals(c2));
	System.out.println("ok");

        System.out.print("testing twoway operations... ");
        System.out.flush();
        Twoways.twoways(communicator, cl);
        Twoways.twoways(communicator, derived);
        derived.opDerived();
        System.out.println("ok");

	System.out.print("testing batch oneway operations... ");
	System.out.flush();
	BatchOneways.batchOneways(cl);
	BatchOneways.batchOneways(derived);
	System.out.println("ok");

        return cl;
    }
}
