// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
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
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = communicator.getProperties().getPropertyWithDefault("Test.Proxy", 
		"test:default -p 12345 -t 10000");
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

    	out.print("testing ice_communicator... ");
	out.flush();
	test(base.ice_communicator() == communicator);
	out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(base);
        test(cl != null);
        Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(cl);
        test(derived != null);
        test(cl.equals(base));
        test(derived.equals(base));
        test(cl.equals(derived));
        out.println("ok");

	out.print("testing checked cast with context... ");
	out.flush();
        String cref = communicator.getProperties().getPropertyWithDefault("Test.ProxyWithContext", 
		"test:default -p 12346 -t 10000");
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
	test(IceUtil.Hashtable.equals(c, c2));
	out.println("ok");
        out.print("testing twoway operations... ");
        out.flush();
        Twoways.twoways(communicator, cl);
        Twoways.twoways(communicator, derived);
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
