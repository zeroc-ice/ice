// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    allTests(Ice.Communicator communicator, Ice.InitializationData initData, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

    	System.out.print("testing ice_getCommunicator... ");
	System.out.flush();
	test(base.ice_getCommunicator() == communicator);
	System.out.println("ok");

	System.out.print("testing proxy methods... ");
	System.out.flush();
	test(communicator.identityToString(
		 base.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()).equals("other"));
	test(base.ice_facet("facet").ice_getFacet().equals("facet"));
	test(base.ice_adapterId("id").ice_getAdapterId().equals("id"));
	test(base.ice_twoway().ice_isTwoway());
	test(base.ice_oneway().ice_isOneway());
	test(base.ice_batchOneway().ice_isBatchOneway());
	test(base.ice_datagram().ice_isDatagram());
	test(base.ice_batchDatagram().ice_isBatchDatagram());
	test(base.ice_secure(true).ice_isSecure());
	test(!base.ice_secure(false).ice_isSecure());
	test(base.ice_collocationOptimized(true).ice_isCollocationOptimized());
	test(!base.ice_collocationOptimized(false).ice_isCollocationOptimized());
	System.out.println("ok");

	System.out.print("testing proxy comparison... ");
	System.out.flush();

	test(communicator.stringToProxy("foo").equals(communicator.stringToProxy("foo")));
	test(!communicator.stringToProxy("foo").equals(communicator.stringToProxy("foo2")));

	Ice.ObjectPrx compObj = communicator.stringToProxy("foo");

	test(compObj.ice_facet("facet").equals(compObj.ice_facet("facet")));
	test(!compObj.ice_facet("facet").equals(compObj.ice_facet("facet1")));

	test(compObj.ice_oneway().equals(compObj.ice_oneway()));
	test(!compObj.ice_oneway().equals(compObj.ice_twoway()));

	test(compObj.ice_secure(true).equals(compObj.ice_secure(true)));
	test(!compObj.ice_secure(false).equals(compObj.ice_secure(true)));

	test(compObj.ice_collocationOptimized(true).equals(compObj.ice_collocationOptimized(true)));
	test(!compObj.ice_collocationOptimized(false).equals(compObj.ice_collocationOptimized(true)));

	test(compObj.ice_connectionCached(true).equals(compObj.ice_connectionCached(true)));
	test(!compObj.ice_connectionCached(false).equals(compObj.ice_connectionCached(true)));

	test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).equals(
		 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random)));
	test(!compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).equals(
		 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered)));

	test(compObj.ice_connectionId("id2").equals(compObj.ice_connectionId("id2")));
	test(!compObj.ice_connectionId("id1").equals(compObj.ice_connectionId("id2")));

	test(compObj.ice_compress(true).equals(compObj.ice_compress(true)));
	test(!compObj.ice_compress(false).equals(compObj.ice_compress(true)));

	test(compObj.ice_timeout(20).equals(compObj.ice_timeout(20)));
	test(!compObj.ice_timeout(10).equals(compObj.ice_timeout(20)));

	Ice.ObjectPrx compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
	Ice.ObjectPrx compObj2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
	test(!compObj1.equals(compObj2));

	compObj1 = communicator.stringToProxy("foo@MyAdapter1");
	compObj2 = communicator.stringToProxy("foo@MyAdapter2");
	test(!compObj1.equals(compObj2));

	test(compObj1.ice_locatorCacheTimeout(20).equals(compObj1.ice_locatorCacheTimeout(20)));
	test(!compObj1.ice_locatorCacheTimeout(10).equals(compObj1.ice_locatorCacheTimeout(20)));

	compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
	compObj2 = communicator.stringToProxy("foo@MyAdapter1");
	test(!compObj1.equals(compObj2));

	//
	// TODO: Ideally we should also test comparison of fixed proxies.
	//
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
	String cref = "context:default -p 12010 -t 10000";
	Ice.ObjectPrx cbase = communicator.stringToProxy(cref);
	test(cbase != null);

	Test.TestCheckedCastPrx tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase);
	java.util.Map c = tccp.getContext();
	test(c == null || c.size() == 0);

	c = new java.util.HashMap();
	c.put("one", "hello");
	c.put("two", "world");
	tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase, c);
	java.util.Map c2 = tccp.getContext();
	test(c.equals(c2));
	System.out.println("ok");

	if(!collocated)
	{
	    System.out.print("testing timeout... ");
	    System.out.flush();
	    try
	    {
		Test.MyClassPrx clTimeout = Test.MyClassPrxHelper.uncheckedCast(cl.ice_timeout(500));
		clTimeout.opSleep(1000);
		test(false);
	    }
	    catch(Ice.TimeoutException ex)
	    {
	    }
	    System.out.println("ok");
	}

        System.out.print("testing twoway operations... ");
        System.out.flush();
        Twoways.twoways(communicator, initData, cl);
        Twoways.twoways(communicator, initData, derived);
        derived.opDerived();
        System.out.println("ok");

	if(!collocated)
	{
	    System.out.print("testing twoway operations with AMI... ");
	    System.out.flush();
	    TwowaysAMI.twowaysAMI(communicator, initData, cl);
	    TwowaysAMI.twowaysAMI(communicator, initData, derived);
	    System.out.println("ok");

	    System.out.print("testing batch oneway operations... ");
	    System.out.flush();
	    BatchOneways.batchOneways(cl);
	    BatchOneways.batchOneways(derived);
	    System.out.println("ok");
	}

        return cl;
    }
}
