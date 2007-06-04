// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        String ref = "test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);

        Ice.ObjectPrx b1 = communicator.stringToProxy("test");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getAdapterId().length() == 0 && b1.ice_getFacet().length() == 0);
        b1 = communicator.stringToProxy("test ");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().length() == 0);
        b1 = communicator.stringToProxy(" test ");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().length() == 0);
        b1 = communicator.stringToProxy(" test");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().length() == 0);
        b1 = communicator.stringToProxy("'test -f facet'");
        test(b1.ice_getIdentity().name.equals("test -f facet") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().length() == 0);
        try
        {
            b1 = communicator.stringToProxy("\"test -f facet'");
            test(false);
        }
        catch(Ice.ProxyParseException ex)
        {
        }
        b1 = communicator.stringToProxy("\"test -f facet\"");
        test(b1.ice_getIdentity().name.equals("test -f facet") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().length() == 0);
        b1 = communicator.stringToProxy("\"test -f facet@test\"");
        test(b1.ice_getIdentity().name.equals("test -f facet@test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().length() == 0);
        b1 = communicator.stringToProxy("\"test -f facet@test @test\"");
        test(b1.ice_getIdentity().name.equals("test -f facet@test @test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().length() == 0);
        try
        {
            b1 = communicator.stringToProxy("test test");
            test(false);
        }
        catch(Ice.ProxyParseException ex)
        {
        }
        b1 = communicator.stringToProxy("test\\040test");
        test(b1.ice_getIdentity().name.equals("test test") && b1.ice_getIdentity().category.length() == 0);
        try
        {
            b1 = communicator.stringToProxy("test\\777");
            test(false);
        }
        catch(Ice.IdentityParseException ex)
        {
        }
        b1 = communicator.stringToProxy("test\\40test");
        test(b1.ice_getIdentity().name.equals("test test"));

        // Test some octal and hex corner cases.
        b1 = communicator.stringToProxy("test\\4test");
        test(b1.ice_getIdentity().name.equals("test\4test"));
        b1 = communicator.stringToProxy("test\\04test");
        test(b1.ice_getIdentity().name.equals("test\4test"));
        b1 = communicator.stringToProxy("test\\004test");
        test(b1.ice_getIdentity().name.equals("test\4test"));
        b1 = communicator.stringToProxy("test\\1114test");
        test(b1.ice_getIdentity().name.equals("test\1114test"));

        b1 = communicator.stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test");
        test(b1.ice_getIdentity().name.equals("test\b\f\n\r\t\'\"\\test") && b1.ice_getIdentity().category.length() == 0);

        b1 = communicator.stringToProxy("category/test");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.equals("category") &&
             b1.ice_getAdapterId().length() == 0);
             
        b1 = communicator.stringToProxy("test@adapter");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getAdapterId().equals("adapter"));
        try
        {
            b1 = communicator.stringToProxy("id@adapter test");
            test(false);
        }
        catch(Ice.ProxyParseException ex)
        {
        }
        b1 = communicator.stringToProxy("category/test@adapter");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.equals("category") &&
             b1.ice_getAdapterId().equals("adapter"));
        b1 = communicator.stringToProxy("category/test@adapter:tcp");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.equals("category") &&
             b1.ice_getAdapterId().equals("adapter:tcp"));
        b1 = communicator.stringToProxy("'category 1/test'@adapter");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.equals("category 1") &&
             b1.ice_getAdapterId().equals("adapter"));
        b1 = communicator.stringToProxy("'category/test 1'@adapter");
        test(b1.ice_getIdentity().name.equals("test 1") && b1.ice_getIdentity().category.equals("category") &&
             b1.ice_getAdapterId().equals("adapter"));
        b1 = communicator.stringToProxy("'category/test'@'adapter 1'");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.equals("category") &&
             b1.ice_getAdapterId().equals("adapter 1"));
        b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@adapter");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.equals("category /test@foo") &&
             b1.ice_getAdapterId().equals("adapter"));
        b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.equals("category /test@foo") &&
             b1.ice_getAdapterId().equals("adapter:tcp"));

        b1 = communicator.stringToProxy("id -f facet");
        test(b1.ice_getIdentity().name.equals("id") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet"));
        b1 = communicator.stringToProxy("id -f 'facet x'");
        test(b1.ice_getIdentity().name.equals("id") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet x"));
        b1 = communicator.stringToProxy("id -f \"facet x\"");
        test(b1.ice_getIdentity().name.equals("id") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet x"));
        try
        {
            b1 = communicator.stringToProxy("id -f \"facet x");
            test(false);
        }
        catch(Ice.ProxyParseException ex)
        {
        }
        try
        {
            b1 = communicator.stringToProxy("id -f \'facet x");
            test(false);
        }
        catch(Ice.ProxyParseException ex)
        {
        }
        b1 = communicator.stringToProxy("test -f facet:tcp");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet") && b1.ice_getAdapterId().length() == 0);
        b1 = communicator.stringToProxy("test -f \"facet:tcp\"");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet:tcp") && b1.ice_getAdapterId().length() == 0);
        b1 = communicator.stringToProxy("test -f facet@test");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet") && b1.ice_getAdapterId().equals("test"));
        b1 = communicator.stringToProxy("test -f 'facet@test'");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet@test") && b1.ice_getAdapterId().length() == 0);
        b1 = communicator.stringToProxy("test -f 'facet@test'@test");
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getFacet().equals("facet@test") && b1.ice_getAdapterId().equals("test"));
        try
        {
            b1 = communicator.stringToProxy("test -f facet@test @test");
            test(false);
        }
        catch(Ice.ProxyParseException ex)
        {
        }
        b1 = communicator.stringToProxy("test");
        test(b1.ice_isTwoway());
        b1 = communicator.stringToProxy("test -t");
        test(b1.ice_isTwoway());
        b1 = communicator.stringToProxy("test -o");
        test(b1.ice_isOneway());
        b1 = communicator.stringToProxy("test -O");
        test(b1.ice_isBatchOneway());
        b1 = communicator.stringToProxy("test -d");
        test(b1.ice_isDatagram());
        b1 = communicator.stringToProxy("test -D");
        test(b1.ice_isBatchDatagram());
        b1 = communicator.stringToProxy("test");
        test(!b1.ice_isSecure());
        b1 = communicator.stringToProxy("test -s");
        test(b1.ice_isSecure());

        try
        {
            b1 = communicator.stringToProxy("test:tcp@adapterId");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }
        // This is an unknown endpoint warning, not a parse exception.
        //
        //try
        //{
        //   b1 = communicator.stringToProxy("test -f the:facet:tcp");
        //   test(false);
        //}
        //catch(Ice.EndpointParseException ex)
        //{
        //}
        try
        {
            b1 = communicator.stringToProxy("test::tcp");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }
        System.out.println("ok");

        System.out.print("testing propertyToProxy... ");
        System.out.flush();
        Ice.Properties prop = communicator.getProperties();
        String propertyPrefix = "Foo.Proxy";
        prop.setProperty(propertyPrefix, "test:default -p 12010 -t 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getIdentity().name.equals("test") && b1.ice_getIdentity().category.length() == 0 &&
             b1.ice_getAdapterId().length() == 0 && b1.ice_getFacet().length() == 0);

        String property;

        // These two properties don't do anything to direct proxies so
        // first we test that.
        /*
         *  Commented out because setting a locator or locator cache
         *  timeout on a direct proxy causes warning.
         *
        property = propertyPrefix + ".Locator";
        test(b1.ice_getLocator() == null);
        prop.setProperty(property, "locator:default -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocator() == null);
        prop.setProperty(property, "");

        property = propertyPrefix + ".LocatorCacheTimeout";
        test(b1.ice_getLocatorCacheTimeout() == 0);
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocatorCacheTimeout() == 0);
        prop.setProperty(property, "");
        */

        // Now retest with an indirect proxy.
        prop.setProperty(propertyPrefix, "test");
        property = propertyPrefix + ".Locator";
        prop.setProperty(property, "locator:default -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocator() != null && b1.ice_getLocator().ice_getIdentity().name.equals("locator"));
        prop.setProperty(property, "");

        property = propertyPrefix + ".LocatorCacheTimeout";
        test(b1.ice_getLocatorCacheTimeout() == -1);
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocatorCacheTimeout() == 1);
        prop.setProperty(property, "");

        // This cannot be tested so easily because the property is cached
        // on communicator initialization.
        //
        //prop.setProperty("Ice.Default.LocatorCacheTimeout", "60");
        //b1 = communicator.propertyToProxy(propertyPrefix);
        //test(b1.ice_getLocatorCacheTimeout() == 60);
        //prop.setProperty("Ice.Default.LocatorCacheTimeout", "");

        prop.setProperty(propertyPrefix, "test:default -p 12010 -t 10000");

        property = propertyPrefix + ".Router";
        test(b1.ice_getRouter() == null);
        prop.setProperty(property, "router:default -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getRouter() != null && b1.ice_getRouter().ice_getIdentity().name.equals("router"));
        prop.setProperty(property, "");

        property = propertyPrefix + ".PreferSecure";
        test(!b1.ice_isPreferSecure());
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_isPreferSecure());
        prop.setProperty(property, "");

        property = propertyPrefix + ".ConnectionCached";
        test(b1.ice_isConnectionCached());
        prop.setProperty(property, "0");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(!b1.ice_isConnectionCached());
        prop.setProperty(property, "");

        property = propertyPrefix + ".EndpointSelection";
        test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);
        prop.setProperty(property, "Random");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);
        prop.setProperty(property, "Ordered");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
        prop.setProperty(property, "");

        property = propertyPrefix + ".CollocationOptimization";
        test(b1.ice_isCollocationOptimized());
        prop.setProperty(property, "0");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(!b1.ice_isCollocationOptimized());
        prop.setProperty(property, "");

        property = propertyPrefix + ".ThreadPerConnection";
        test(!b1.ice_isThreadPerConnection());
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_isThreadPerConnection());
        prop.setProperty(property, "");

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

        java.util.Map c = cl.getContext();
        test(c == null || c.size() == 0);

        c = new java.util.HashMap();
        c.put("one", "hello");
        c.put("two", "world");
        cl = Test.MyClassPrxHelper.checkedCast(base, c);
        java.util.Map c2 = cl.getContext();
        test(c.equals(c2));
        System.out.println("ok");

        System.out.print("testing opaque endpoints... ");
        System.out.flush();

        try
        {
            // Invalid -x option
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t 99 -v abc -x abc");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Missing -t and -v
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Repeated -t
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t 1 -t 1 -v abc");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Repeated -v
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t 1 -v abc -v abc");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Missing -t
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -v abc");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Missing -v
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t 1");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Missing arg for -t
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t -v abc");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Missing arg for -v
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t 1 -v");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Not a number for -t
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t x -v abc");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // < 0 for -t
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t -1 -v abc");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        try
        {
            // Invalid char for -v
            Ice.ObjectPrx p = communicator.stringToProxy("id:opaque -t 99 -v x?c");
            test(false);
        }
        catch(Ice.EndpointParseException ex)
        {
        }

        // Legal TCP endpoint expressed as opaque endpoint
        Ice.ObjectPrx p1 = communicator.stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
        String pstr = communicator.proxyToString(p1);
        test(pstr.equals("test -t:tcp -h 127.0.0.1 -p 12010 -t 10000"));
        
        // Working?
        p1.ice_ping();

        // Two legal TCP endpoints expressed as opaque endpoints
        p1 = communicator.stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
        pstr = communicator.proxyToString(p1);
        test(pstr.equals("test -t:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000"));

        //
        // Test that an SSL endpoint and a nonsense endpoint get
        // written back out as an opaque endpoint.
        //
        p1 = communicator.stringToProxy("test:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");
        pstr = communicator.proxyToString(p1);
        test(pstr.equals("test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch"));

        //
        // Try to invoke on the SSL endpoint to verify that we get a
        // NoEndpointException.
        //
        try
        {
            p1.ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }

        //
        // Test that the proxy with an SSL endpoint and a nonsense
        // endpoint (which the server doesn't understand either) can
        // be sent over the wire and returned by the server without
        // losing the opaque endpoints.
        //
        Ice.ObjectPrx p2 = derived.echo(p1);
        pstr = communicator.proxyToString(p2);
        test(pstr.equals("test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch"));

        System.out.println("ok");

        return cl;
    }
}
