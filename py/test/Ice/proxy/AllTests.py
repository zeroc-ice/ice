#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, collocated):
    print "testing stringToProxy...",
    ref = "test:default -p 12010 -t 10000"
    base = communicator.stringToProxy(ref)
    test(base)

    b1 = communicator.stringToProxy("test")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getAdapterId()) == 0 and len(b1.ice_getFacet()) == 0)
    b1 = communicator.stringToProxy("test ")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getFacet()) == 0)
    b1 = communicator.stringToProxy(" test ")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getFacet()) == 0)
    b1 = communicator.stringToProxy(" test")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getFacet()) == 0)
    b1 = communicator.stringToProxy("'test -f facet'")
    test(b1.ice_getIdentity().name == "test -f facet" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getFacet()) == 0)
    try:
        b1 = communicator.stringToProxy("\"test -f facet'")
        test(False)
    except Ice.ProxyParseException:
        pass
    b1 = communicator.stringToProxy("\"test -f facet\"")
    test(b1.ice_getIdentity().name == "test -f facet" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getFacet()) == 0)
    b1 = communicator.stringToProxy("\"test -f facet@test\"")
    test(b1.ice_getIdentity().name == "test -f facet@test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getFacet()) == 0)
    b1 = communicator.stringToProxy("\"test -f facet@test @test\"")
    test(b1.ice_getIdentity().name == "test -f facet@test @test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getFacet()) == 0)
    try:
        b1 = communicator.stringToProxy("test test")
        test(False)
    except Ice.ProxyParseException:
        pass
    b1 = communicator.stringToProxy("test\\040test")
    test(b1.ice_getIdentity().name == "test test" and len(b1.ice_getIdentity().category) == 0)
    try:
        b1 = communicator.stringToProxy("test\\777")
        test(False)
    except Ice.IdentityParseException:
        pass
    b1 = communicator.stringToProxy("test\\40test")
    test(b1.ice_getIdentity().name == "test test")

    # Test some octal and hex corner cases.
    b1 = communicator.stringToProxy("test\\4test")
    test(b1.ice_getIdentity().name == "test\4test")
    b1 = communicator.stringToProxy("test\\04test")
    test(b1.ice_getIdentity().name == "test\4test")
    b1 = communicator.stringToProxy("test\\004test")
    test(b1.ice_getIdentity().name == "test\4test")
    b1 = communicator.stringToProxy("test\\1114test")
    test(b1.ice_getIdentity().name == "test\1114test")

    b1 = communicator.stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test")
    test(b1.ice_getIdentity().name == "test\b\f\n\r\t\'\"\\test" and len(b1.ice_getIdentity().category) == 0)

    b1 = communicator.stringToProxy("category/test")
    test(b1.ice_getIdentity().name == "test" and b1.ice_getIdentity().category == "category" and \
         len(b1.ice_getAdapterId()) == 0)

    b1 = communicator.stringToProxy("test@adapter")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getAdapterId() == "adapter")
    try:
        b1 = communicator.stringToProxy("id@adapter test")
        test(False)
    except Ice.ProxyParseException:
        pass
    b1 = communicator.stringToProxy("category/test@adapter")
    test(b1.ice_getIdentity().name == "test" and b1.ice_getIdentity().category == "category" and \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("category/test@adapter:tcp")
    test(b1.ice_getIdentity().name == "test" and b1.ice_getIdentity().category == "category" and \
         b1.ice_getAdapterId() == "adapter:tcp")
    b1 = communicator.stringToProxy("'category 1/test'@adapter")
    test(b1.ice_getIdentity().name == "test" and b1.ice_getIdentity().category == "category 1" and \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("'category/test 1'@adapter")
    test(b1.ice_getIdentity().name == "test 1" and b1.ice_getIdentity().category == "category" and \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("'category/test'@'adapter 1'")
    test(b1.ice_getIdentity().name == "test" and b1.ice_getIdentity().category == "category" and \
         b1.ice_getAdapterId() == "adapter 1")
    b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@adapter")
    test(b1.ice_getIdentity().name == "test" and b1.ice_getIdentity().category == "category /test@foo" and \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"")
    test(b1.ice_getIdentity().name == "test" and b1.ice_getIdentity().category == "category /test@foo" and \
         b1.ice_getAdapterId() == "adapter:tcp")

    b1 = communicator.stringToProxy("id -f facet")
    test(b1.ice_getIdentity().name == "id" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet")
    b1 = communicator.stringToProxy("id -f 'facet x'")
    test(b1.ice_getIdentity().name == "id" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet x")
    b1 = communicator.stringToProxy("id -f \"facet x\"")
    test(b1.ice_getIdentity().name == "id" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet x")
    try:
        b1 = communicator.stringToProxy("id -f \"facet x")
        test(False)
    except Ice.ProxyParseException:
        pass
    try:
        b1 = communicator.stringToProxy("id -f \'facet x")
        test(False)
    except Ice.ProxyParseException:
        pass
    b1 = communicator.stringToProxy("test -f facet:tcp")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet" and len(b1.ice_getAdapterId()) == 0)
    b1 = communicator.stringToProxy("test -f \"facet:tcp\"")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet:tcp" and len(b1.ice_getAdapterId()) == 0)
    b1 = communicator.stringToProxy("test -f facet@test")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet" and b1.ice_getAdapterId() == "test")
    b1 = communicator.stringToProxy("test -f 'facet@test'")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet@test" and len(b1.ice_getAdapterId()) == 0)
    b1 = communicator.stringToProxy("test -f 'facet@test'@test")
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         b1.ice_getFacet() == "facet@test" and b1.ice_getAdapterId() == "test")
    try:
        b1 = communicator.stringToProxy("test -f facet@test @test")
        test(False)
    except Ice.ProxyParseException:
        pass
    b1 = communicator.stringToProxy("test")
    test(b1.ice_isTwoway())
    b1 = communicator.stringToProxy("test -t")
    test(b1.ice_isTwoway())
    b1 = communicator.stringToProxy("test -o")
    test(b1.ice_isOneway())
    b1 = communicator.stringToProxy("test -O")
    test(b1.ice_isBatchOneway())
    b1 = communicator.stringToProxy("test -d")
    test(b1.ice_isDatagram())
    b1 = communicator.stringToProxy("test -D")
    test(b1.ice_isBatchDatagram())
    b1 = communicator.stringToProxy("test")
    test(not b1.ice_isSecure())
    b1 = communicator.stringToProxy("test -s")
    test(b1.ice_isSecure())

    try:
        b1 = communicator.stringToProxy("test:tcp@adapterId")
        test(False)
    except Ice.EndpointParseException:
        pass
    # This is an unknown endpoint warning, not a parse exception.
    #
    #try:
    #   b1 = communicator.stringToProxy("test -f the:facet:tcp")
    #   test(False)
    #except Ice.EndpointParseException:
    #   pass
    try:
        b1 = communicator.stringToProxy("test::tcp")
        test(False)
    except Ice.EndpointParseException:
        pass
    print "ok"

    print "testing propertyToProxy... ",
    prop = communicator.getProperties()
    propertyPrefix = "Foo.Proxy"
    prop.setProperty(propertyPrefix, "test:default -p 12010 -t 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getAdapterId()) == 0 and len(b1.ice_getFacet()) == 0)

    # These two properties don't do anything to direct proxies so
    # first we test that.
    property = propertyPrefix + ".Locator"
    test(not b1.ice_getLocator())
    prop.setProperty(property, "locator:default -p 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(not b1.ice_getLocator())
    prop.setProperty(property, "")

    property = propertyPrefix + ".LocatorCacheTimeout"
    test(b1.ice_getLocatorCacheTimeout() == 0)
    prop.setProperty(property, "1")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getLocatorCacheTimeout() == 0)
    prop.setProperty(property, "")

    # Now retest with an indirect proxy.
    prop.setProperty(propertyPrefix, "test")
    property = propertyPrefix + ".Locator"
    prop.setProperty(property, "locator:default -p 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getLocator() and b1.ice_getLocator().ice_getIdentity().name == "locator")
    prop.setProperty(property, "")

    property = propertyPrefix + ".LocatorCacheTimeout"
    test(b1.ice_getLocatorCacheTimeout() == -1)
    prop.setProperty(property, "1")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getLocatorCacheTimeout() == 1)
    prop.setProperty(property, "")

    # This cannot be tested so easily because the property is cached
    # on communicator initialization.
    #
    #prop.setProperty("Ice.Default.LocatorCacheTimeout", "60")
    #b1 = communicator.propertyToProxy(propertyPrefix)
    #test(b1.ice_getLocatorCacheTimeout() == 60)
    #prop.setProperty("Ice.Default.LocatorCacheTimeout", "")

    prop.setProperty(propertyPrefix, "test:default -p 12010 -t 10000")

    property = propertyPrefix + ".Router"
    test(not b1.ice_getRouter())
    prop.setProperty(property, "router:default -p 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getRouter() and b1.ice_getRouter().ice_getIdentity().name == "router")
    prop.setProperty(property, "")

    property = propertyPrefix + ".PreferSecure"
    test(not b1.ice_isPreferSecure())
    prop.setProperty(property, "1")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_isPreferSecure())
    prop.setProperty(property, "")

    property = propertyPrefix + ".ConnectionCached"
    test(b1.ice_isConnectionCached())
    prop.setProperty(property, "0")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(not b1.ice_isConnectionCached())
    prop.setProperty(property, "")

    property = propertyPrefix + ".EndpointSelection"
    test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
    prop.setProperty(property, "Random")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
    prop.setProperty(property, "Ordered")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered)
    prop.setProperty(property, "")

    #
    # isCollocationOptimized is not implemented because the
    # collocation optimization is permanently disabled with IcePy.
    #
    #property = propertyPrefix + ".CollocationOptimization"
    #test(b1.ice_isCollocationOptimized())
    #prop.setProperty(property, "0")
    #b1 = communicator.propertyToProxy(propertyPrefix)
    #test(not b1.ice_isCollocationOptimized())
    #prop.setProperty(property, "")

    property = propertyPrefix + ".ThreadPerConnection"
    test(not b1.ice_isThreadPerConnection())
    prop.setProperty(property, "1")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_isThreadPerConnection())
    prop.setProperty(property, "")

    print "ok"

    print "testing ice_getCommunicator...",
    test(base.ice_getCommunicator() == communicator)
    print "ok"

    print "testing proxy methods... ",
    test(communicator.identityToString(base.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()) \
         == "other")
    test(base.ice_facet("facet").ice_getFacet() == "facet")
    test(base.ice_adapterId("id").ice_getAdapterId() == "id")
    test(base.ice_twoway().ice_isTwoway())
    test(base.ice_oneway().ice_isOneway())
    test(base.ice_batchOneway().ice_isBatchOneway())
    test(base.ice_datagram().ice_isDatagram())
    test(base.ice_batchDatagram().ice_isBatchDatagram())
    test(base.ice_secure(True).ice_isSecure())
    test(not base.ice_secure(False).ice_isSecure())
    #test(base.ice_collocationOptimized(true)->ice_isCollocationOptimized());
    #test(!base.ice_collocationOptimized(false)->ice_isCollocationOptimized());
    print "ok"

    print "testing proxy comparison... ",

    test(communicator.stringToProxy("foo") == communicator.stringToProxy("foo"));
    test(communicator.stringToProxy("foo") != communicator.stringToProxy("foo2"));
    test(communicator.stringToProxy("foo") < communicator.stringToProxy("foo2"));
    test(not (communicator.stringToProxy("foo2") < communicator.stringToProxy("foo")));

    compObj = communicator.stringToProxy("foo");

    test(compObj.ice_facet("facet") == compObj.ice_facet("facet"));
    test(compObj.ice_facet("facet") != compObj.ice_facet("facet1"));
    test(compObj.ice_facet("facet") < compObj.ice_facet("facet1"));
    test(not (compObj.ice_facet("facet") < compObj.ice_facet("facet")));

    test(compObj.ice_oneway() == compObj.ice_oneway());
    test(compObj.ice_oneway() != compObj.ice_twoway());
    test(compObj.ice_twoway() < compObj.ice_oneway());
    test(not (compObj.ice_oneway() < compObj.ice_twoway()));

    test(compObj.ice_secure(True) == compObj.ice_secure(True));
    test(compObj.ice_secure(False) != compObj.ice_secure(True));
    test(compObj.ice_secure(False) < compObj.ice_secure(True));
    test(not (compObj.ice_secure(True) < compObj.ice_secure(False)));

    #test(compObj.ice_collocationOptimized(True) == compObj.ice_collocationOptimized(True));
    #test(compObj.ice_collocationOptimized(False) != compObj.ice_collocationOptimized(True));
    #test(compObj.ice_collocationOptimized(False) < compObj.ice_collocationOptimized(True));
    #test(!(compObj.ice_collocationOptimized(True) < compObj.ice_collocationOptimized(False)));

    test(compObj.ice_connectionCached(True) == compObj.ice_connectionCached(True));
    test(compObj.ice_connectionCached(False) != compObj.ice_connectionCached(True));
    test(compObj.ice_connectionCached(False) < compObj.ice_connectionCached(True));
    test(not (compObj.ice_connectionCached(True) < compObj.ice_connectionCached(False)));

    test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) == \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random));
    test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) != \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
    test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) < \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
    test(not (compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered) < \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random)));

    test(compObj.ice_connectionId("id2") == compObj.ice_connectionId("id2"));
    test(compObj.ice_connectionId("id1") != compObj.ice_connectionId("id2"));
    test(compObj.ice_connectionId("id1") < compObj.ice_connectionId("id2"));
    test(not (compObj.ice_connectionId("id2") < compObj.ice_connectionId("id1")));

    test(compObj.ice_compress(True) == compObj.ice_compress(True));
    test(compObj.ice_compress(False) != compObj.ice_compress(True));
    test(compObj.ice_compress(False) < compObj.ice_compress(True));
    test(not (compObj.ice_compress(True) < compObj.ice_compress(False)));

    test(compObj.ice_timeout(20) == compObj.ice_timeout(20));
    test(compObj.ice_timeout(10) != compObj.ice_timeout(20));
    test(compObj.ice_timeout(10) < compObj.ice_timeout(20));
    test(not (compObj.ice_timeout(20) < compObj.ice_timeout(10)));

    compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
    compObj2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(not (compObj2 < compObj1));

    compObj1 = communicator.stringToProxy("foo@MyAdapter1");
    compObj2 = communicator.stringToProxy("foo@MyAdapter2");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(not (compObj2 < compObj1));

    test(compObj1.ice_locatorCacheTimeout(20) == compObj1.ice_locatorCacheTimeout(20));
    test(compObj1.ice_locatorCacheTimeout(10) != compObj1.ice_locatorCacheTimeout(20));
    test(compObj1.ice_locatorCacheTimeout(10) < compObj1.ice_locatorCacheTimeout(20));
    test(not (compObj1.ice_locatorCacheTimeout(20) < compObj1.ice_locatorCacheTimeout(10)));

    compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
    compObj2 = communicator.stringToProxy("foo@MyAdapter1");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(not (compObj2 < compObj1));

    #
    # TODO: Ideally we should also test comparison of fixed proxies.
    #

    print "ok"

    print "testing checked cast...",
    cl = Test.MyClassPrx.checkedCast(base)
    test(cl)

    derived = Test.MyDerivedClassPrx.checkedCast(cl)
    test(derived)
    test(cl == base)
    test(derived == base)
    test(cl == derived)

    loc = Ice.LocatorPrx.checkedCast(base)
    test(loc == None)

    #
    # Upcasting
    #
    cl2 = Test.MyClassPrx.checkedCast(derived)
    obj = Ice.ObjectPrx.checkedCast(derived)
    test(cl2)
    test(obj)
    test(cl2 == obj)
    test(cl2 == derived)

    print "ok"

    print "testing checked cast with context...",
    tccp = Test.MyClassPrx.checkedCast(base)
    c = tccp.getContext()
    test(c == None or len(c) == 0)

    c = { }
    c["one"] = "hello"
    c["two"] =  "world"
    tccp = Test.MyClassPrx.checkedCast(base, c)
    c2 = tccp.getContext()
    test(c == c2)
    print "ok"

    print "testing opaque endpoints... ",

    try:
        # Invalid -x option
        p = communicator.stringToProxy("id:opaque -t 99 -v abc -x abc");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Missing -t and -v
        p = communicator.stringToProxy("id:opaque");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Repeated -t
        p = communicator.stringToProxy("id:opaque -t 1 -t 1 -v abc");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Repeated -v
        p = communicator.stringToProxy("id:opaque -t 1 -v abc -v abc");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Missing -t
        p = communicator.stringToProxy("id:opaque -v abc");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Missing -v
        p = communicator.stringToProxy("id:opaque -t 1");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Missing arg for -t
        p = communicator.stringToProxy("id:opaque -t -v abc");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Missing arg for -v
        p = communicator.stringToProxy("id:opaque -t 1 -v");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Not a number for -t
        p = communicator.stringToProxy("id:opaque -t x -v abc");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # < 0 for -t
        p = communicator.stringToProxy("id:opaque -t -1 -v abc");
        test(false);
    except Ice.EndpointParseException:
        pass

    try:
        # Invalid char for -v
        p = communicator.stringToProxy("id:opaque -t 99 -v x?c");
        test(false);
    except Ice.EndpointParseException:
        pass

    # Legal TCP endpoint expressed as opaque endpoint
    p1 = communicator.stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    pstr = communicator.proxyToString(p1);
    test(pstr == "test -t:tcp -h 127.0.0.1 -p 12010 -t 10000");
    
    # Working?
    p1.ice_ping();

    # Two legal TCP endpoints expressed as opaque endpoints
    p1 = communicator.stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
    pstr = communicator.proxyToString(p1);
    test(pstr == "test -t:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

    #
    # Test that an SSL endpoint and a nonsense endpoint get written
    # back out as an opaque endpoint.
    #
    p1 = communicator.stringToProxy("test:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");
    pstr = communicator.proxyToString(p1);
    test(pstr == "test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");

    #
    # Try to invoke on the SSL endpoint to verify that we get a
    # NoEndpointException.
    #
    try:
        p1.ice_ping();
        test(false);
    except Ice.NoEndpointException:
        pass

    #
    # Test that the proxy with an SSL endpoint and a nonsense
    # endpoint (which the server doesn't understand either) can be
    # sent over the wire and returned by the server without losing
    # the opaque endpoints.
    #
    p2 = derived.echo(p1);
    pstr = communicator.proxyToString(p2);
    test(pstr == "test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");

    print "ok"

    return cl
