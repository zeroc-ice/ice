// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class AllTests
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }
    
    public static Test.MyClassPrx allTests(Ice.Communicator communicator, bool collocated)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        string rf = "test:default -p 12010 -t 10000";
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        test(baseProxy != null);
        Console.Out.WriteLine("ok");
        Console.Out.Write("testing ice_getCommunicator... ");
        Console.Out.Flush();
        test(baseProxy.ice_getCommunicator() == communicator);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing proxy methods... ");
        test(communicator.identityToString(
                 baseProxy.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()).Equals("other"));
        test(baseProxy.ice_facet("facet").ice_getFacet().Equals("facet"));
        test(baseProxy.ice_adapterId("id").ice_getAdapterId().Equals("id"));
        test(baseProxy.ice_twoway().ice_isTwoway());
        test(baseProxy.ice_oneway().ice_isOneway());
        test(baseProxy.ice_batchOneway().ice_isBatchOneway());
        test(baseProxy.ice_datagram().ice_isDatagram());
        test(baseProxy.ice_batchDatagram().ice_isBatchDatagram());
        test(baseProxy.ice_secure(true).ice_isSecure());
        test(!baseProxy.ice_secure(false).ice_isSecure());
        test(baseProxy.ice_collocationOptimized(true).ice_isCollocationOptimized());
        test(!baseProxy.ice_collocationOptimized(false).ice_isCollocationOptimized());
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing proxy comparison... ");
        Console.Out.Flush();

        test(communicator.stringToProxy("foo").Equals(communicator.stringToProxy("foo")));
        test(!communicator.stringToProxy("foo").Equals(communicator.stringToProxy("foo2")));

        Ice.ObjectPrx compObj = communicator.stringToProxy("foo");

        test(compObj.ice_facet("facet").Equals(compObj.ice_facet("facet")));
        test(!compObj.ice_facet("facet").Equals(compObj.ice_facet("facet1")));

        test(compObj.ice_oneway().Equals(compObj.ice_oneway()));
        test(!compObj.ice_oneway().Equals(compObj.ice_twoway()));

        test(compObj.ice_secure(true).Equals(compObj.ice_secure(true)));
        test(!compObj.ice_secure(false).Equals(compObj.ice_secure(true)));

        test(compObj.ice_collocationOptimized(true).Equals(compObj.ice_collocationOptimized(true)));
        test(!compObj.ice_collocationOptimized(false).Equals(compObj.ice_collocationOptimized(true)));

        test(compObj.ice_connectionCached(true).Equals(compObj.ice_connectionCached(true)));
        test(!compObj.ice_connectionCached(false).Equals(compObj.ice_connectionCached(true)));

        test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).Equals(
                 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random)));
        test(!compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).Equals(
                 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered)));

        test(compObj.ice_connectionId("id2").Equals(compObj.ice_connectionId("id2")));
        test(!compObj.ice_connectionId("id1").Equals(compObj.ice_connectionId("id2")));

        test(compObj.ice_compress(true).Equals(compObj.ice_compress(true)));
        test(!compObj.ice_compress(false).Equals(compObj.ice_compress(true)));

        test(compObj.ice_timeout(20).Equals(compObj.ice_timeout(20)));
        test(!compObj.ice_timeout(10).Equals(compObj.ice_timeout(20)));

        Ice.ObjectPrx compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
        Ice.ObjectPrx compObj2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
        test(!compObj1.Equals(compObj2));

        compObj1 = communicator.stringToProxy("foo@MyAdapter1");
        compObj2 = communicator.stringToProxy("foo@MyAdapter2");
        test(!compObj1.Equals(compObj2));

        test(compObj1.ice_locatorCacheTimeout(20).Equals(compObj1.ice_locatorCacheTimeout(20)));
        test(!compObj1.ice_locatorCacheTimeout(10).Equals(compObj1.ice_locatorCacheTimeout(20)));

        compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
        compObj2 = communicator.stringToProxy("foo@MyAdapter1");
        test(!compObj1.Equals(compObj2));

        //
        // TODO: Ideally we should also test comparison of fixed proxies.
        //
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        test(cl != null);
        Test.MyDerivedClassPrx derivedProxy = Test.MyDerivedClassPrxHelper.checkedCast(cl);
        test(derivedProxy != null);
        test(cl.Equals(baseProxy));
        test(derivedProxy.Equals(baseProxy));
        test(cl.Equals(derivedProxy));
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast with context... ");
        Console.Out.Flush();
        string cref = "context:default -p 12010 -t 10000";
        Ice.ObjectPrx cbase = communicator.stringToProxy(cref);
        test(cbase != null);

        Test.TestCheckedCastPrx tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase);
        Ice.Context c = tccp.getContext();
        test(c == null || c.Count == 0);

        c = new Ice.Context();
        c["one"] = "hello";
        c["two"] = "world";
        tccp = Test.TestCheckedCastPrxHelper.checkedCast(cbase, c);
        Ice.Context c2 = tccp.getContext();
        test(c.Equals(c2));
        Console.Out.WriteLine("ok");

        if(!collocated)
        {
            Console.Out.Write("testing timeout... ");
            Console.Out.Flush();
            try
            {
                Test.MyClassPrx clTimeout = Test.MyClassPrxHelper.uncheckedCast(cl.ice_timeout(500));
                clTimeout.opSleep(1000);
                test(false);
            }
            catch(Ice.TimeoutException)
            {
            }
            Console.Out.WriteLine("ok");
        }
        
        Console.Out.Write("testing twoway operations... ");
        Console.Out.Flush();
        Twoways.twoways(communicator, cl);
        Twoways.twoways(communicator, derivedProxy);
        derivedProxy.opDerived();
        Console.Out.WriteLine("ok");
        
        if(!collocated)
        {
            Console.Out.Write("testing twoway operations with AMI... ");
            Console.Out.Flush();
            TwowaysAMI.twowaysAMI(communicator, cl);
            TwowaysAMI.twowaysAMI(communicator, derivedProxy);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing batch oneway operations... ");
            Console.Out.Flush();
            BatchOneways.batchOneways(cl);
            BatchOneways.batchOneways(derivedProxy);
            Console.Out.WriteLine("ok");
        }
        
        return cl;
    }
}
