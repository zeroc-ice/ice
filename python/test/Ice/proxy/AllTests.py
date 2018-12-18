# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(helper, communicator, collocated):
    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()

    #
    # Test nil proxies.
    #
    p = communicator.stringToProxy('')
    test(p == None)
    p = communicator.propertyToProxy('bogus')
    test(p == None)

    ref = "test:{0}".format(helper.getTestEndpoint())
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
    print("ok")

    sys.stdout.write("testing propertyToProxy... ")
    sys.stdout.flush()
    prop = communicator.getProperties()
    propertyPrefix = "Foo.Proxy"
    prop.setProperty(propertyPrefix, "test:{0}".format(helper.getTestEndpoint()))
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getIdentity().name == "test" and len(b1.ice_getIdentity().category) == 0 and \
         len(b1.ice_getAdapterId()) == 0 and len(b1.ice_getFacet()) == 0)

    property = propertyPrefix + ".Locator"
    test(not b1.ice_getLocator())
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

    prop.setProperty(propertyPrefix, "test:{0}".format(helper.getTestEndpoint()))

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

    property = propertyPrefix + ".InvocationTimeout";
    test(b1.ice_getInvocationTimeout() == -1);
    prop.setProperty(property, "1000");
    b1 = communicator.propertyToProxy(propertyPrefix);
    test(b1.ice_getInvocationTimeout() == 1000);
    prop.setProperty(property, "");

    property = propertyPrefix + ".EndpointSelection"
    test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
    prop.setProperty(property, "Random")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random)
    prop.setProperty(property, "Ordered")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered)
    prop.setProperty(property, "")
    property = propertyPrefix + ".CollocationOptimized"
    test(b1.ice_isCollocationOptimized())
    prop.setProperty(property, "0")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(not b1.ice_isCollocationOptimized())
    prop.setProperty(property, "")

    print("ok")

    sys.stdout.write("testing proxyToProperty... ")
    sys.stdout.flush()

    b1 = communicator.stringToProxy("test")
    b1 = b1.ice_collocationOptimized(True)
    b1 = b1.ice_connectionCached(True)
    b1 = b1.ice_preferSecure(False)
    b1 = b1.ice_endpointSelection(Ice.EndpointSelectionType.Ordered)
    b1 = b1.ice_locatorCacheTimeout(100)
    b1 = b1.ice_invocationTimeout(1234);
    b1 = b1.ice_encodingVersion(Ice.EncodingVersion(1, 0))

    router = communicator.stringToProxy("router")
    router = router.ice_collocationOptimized(False)
    router = router.ice_connectionCached(True)
    router = router.ice_preferSecure(True)
    router = router.ice_endpointSelection(Ice.EndpointSelectionType.Random)
    router = router.ice_locatorCacheTimeout(200)
    router = router.ice_invocationTimeout(1500);

    locator = communicator.stringToProxy("locator")
    locator = locator.ice_collocationOptimized(True)
    locator = locator.ice_connectionCached(False)
    locator = locator.ice_preferSecure(True)
    locator = locator.ice_endpointSelection(Ice.EndpointSelectionType.Random)
    locator = locator.ice_locatorCacheTimeout(300)
    locator = locator.ice_invocationTimeout(1500);

    locator = locator.ice_router(Ice.RouterPrx.uncheckedCast(router))
    b1 = b1.ice_locator(Ice.LocatorPrx.uncheckedCast(locator))

    proxyProps = communicator.proxyToProperty(b1, "Test")
    test(len(proxyProps) == 21)

    test(proxyProps["Test"] == "test -t -e 1.0")
    test(proxyProps["Test.CollocationOptimized"] == "1")
    test(proxyProps["Test.ConnectionCached"] == "1")
    test(proxyProps["Test.PreferSecure"] == "0")
    test(proxyProps["Test.EndpointSelection"] == "Ordered")
    test(proxyProps["Test.LocatorCacheTimeout"] == "100")
    test(proxyProps["Test.InvocationTimeout"] == "1234");

    test(proxyProps["Test.Locator"] == "locator -t -e " + Ice.encodingVersionToString(Ice.currentEncoding()))
    test(proxyProps["Test.Locator.CollocationOptimized"] == "1")
    test(proxyProps["Test.Locator.ConnectionCached"] == "0")
    test(proxyProps["Test.Locator.PreferSecure"] == "1")
    test(proxyProps["Test.Locator.EndpointSelection"] == "Random")
    test(proxyProps["Test.Locator.LocatorCacheTimeout"] == "300")
    test(proxyProps["Test.Locator.InvocationTimeout"] == "1500");

    test(proxyProps["Test.Locator.Router"] == "router -t -e " + Ice.encodingVersionToString(Ice.currentEncoding()))
    test(proxyProps["Test.Locator.Router.CollocationOptimized"] == "0")
    test(proxyProps["Test.Locator.Router.ConnectionCached"] == "1")
    test(proxyProps["Test.Locator.Router.PreferSecure"] == "1")
    test(proxyProps["Test.Locator.Router.EndpointSelection"] == "Random")
    test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200")
    test(proxyProps["Test.Locator.Router.InvocationTimeout"] == "1500");

    print("ok")

    sys.stdout.write("testing ice_getCommunicator... ")
    sys.stdout.flush()

    test(base.ice_getCommunicator() == communicator)

    print("ok")

    sys.stdout.write("testing proxy methods... ")
    sys.stdout.flush()

    test(communicator.identityToString(base.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()) \
         == "other")

    #
    # Verify that ToStringMode is passed correctly
    #
    euroStr = "\xE2\x82\xAC" if sys.version_info[0] < 3 else "\u20ac"
    ident = Ice.Identity("test", "\x7F{}".format(euroStr))

    idStr = Ice.identityToString(ident, Ice.ToStringMode.Unicode)
    test(idStr == "\\u007f{}/test".format(euroStr))
    ident2 = Ice.stringToIdentity(idStr)
    test(ident == ident2)
    test(Ice.identityToString(ident) == idStr)

    idStr = Ice.identityToString(ident, Ice.ToStringMode.ASCII)
    test(idStr == "\\u007f\\u20ac/test")
    ident2 = Ice.stringToIdentity(idStr)
    test(ident == ident2)

    idStr = Ice.identityToString(ident, Ice.ToStringMode.Compat)
    test(idStr == "\\177\\342\\202\\254/test")
    ident2 = Ice.stringToIdentity(idStr)
    test(ident == ident2)

    ident2 = Ice.stringToIdentity(communicator.identityToString(ident))
    test(ident == ident2)

    test(base.ice_facet("facet").ice_getFacet() == "facet")
    test(base.ice_adapterId("id").ice_getAdapterId() == "id")
    test(base.ice_twoway().ice_isTwoway())
    test(base.ice_oneway().ice_isOneway())
    test(base.ice_batchOneway().ice_isBatchOneway())
    test(base.ice_datagram().ice_isDatagram())
    test(base.ice_batchDatagram().ice_isBatchDatagram())
    test(base.ice_secure(True).ice_isSecure())
    test(not base.ice_secure(False).ice_isSecure())
    test(base.ice_collocationOptimized(True).ice_isCollocationOptimized())
    test(not base.ice_collocationOptimized(False).ice_isCollocationOptimized())
    test(base.ice_preferSecure(True).ice_isPreferSecure())
    test(not base.ice_preferSecure(False).ice_isPreferSecure())
    test(base.ice_encodingVersion(Ice.Encoding_1_0).ice_getEncodingVersion() == Ice.Encoding_1_0)
    test(base.ice_encodingVersion(Ice.Encoding_1_1).ice_getEncodingVersion() == Ice.Encoding_1_1)
    test(base.ice_encodingVersion(Ice.Encoding_1_0).ice_getEncodingVersion() != Ice.Encoding_1_1)

    try:
        base.ice_timeout(0)
        test(False)
    except RuntimeError:
        pass

    try:
        base.ice_timeout(-1)
    except RuntimeError:
        test(False)

    try:
        base.ice_timeout(-2)
        test(False)
    except RuntimeError:
        pass

    try:
        base.ice_invocationTimeout(0)
        test(False)
    except RuntimeError:
        pass

    try:
        base.ice_invocationTimeout(-1)
    except RuntimeError:
        test(False)

    try:
        base.ice_invocationTimeout(-2)
        test(False)
    except RuntimeError:
        pass

    try:
        base.ice_locatorCacheTimeout(0)
    except RuntimeError:
        test(False)

    try:
        base.ice_locatorCacheTimeout(-1)
    except RuntimeError:
        test(False)

    try:
        base.ice_locatorCacheTimeout(-2)
        test(False)
    except RuntimeError:
        pass

    print("ok")

    sys.stdout.write("testing proxy comparison... ")
    sys.stdout.flush()

    test(communicator.stringToProxy("foo") == communicator.stringToProxy("foo"))
    test(communicator.stringToProxy("foo") != communicator.stringToProxy("foo2"))
    test(communicator.stringToProxy("foo") < communicator.stringToProxy("foo2"))
    test(not (communicator.stringToProxy("foo2") < communicator.stringToProxy("foo")))

    compObj = communicator.stringToProxy("foo")

    test(compObj.ice_facet("facet") == compObj.ice_facet("facet"))
    test(compObj.ice_facet("facet") != compObj.ice_facet("facet1"))
    test(compObj.ice_facet("facet") < compObj.ice_facet("facet1"))
    test(not (compObj.ice_facet("facet") < compObj.ice_facet("facet")))

    test(compObj.ice_oneway() == compObj.ice_oneway())
    test(compObj.ice_oneway() != compObj.ice_twoway())
    test(compObj.ice_twoway() < compObj.ice_oneway())
    test(not (compObj.ice_oneway() < compObj.ice_twoway()))

    test(compObj.ice_secure(True) == compObj.ice_secure(True))
    test(compObj.ice_secure(False) != compObj.ice_secure(True))
    test(compObj.ice_secure(False) < compObj.ice_secure(True))
    test(not (compObj.ice_secure(True) < compObj.ice_secure(False)))

    test(compObj.ice_collocationOptimized(True) == compObj.ice_collocationOptimized(True))
    test(compObj.ice_collocationOptimized(False) != compObj.ice_collocationOptimized(True))
    test(compObj.ice_collocationOptimized(False) < compObj.ice_collocationOptimized(True))
    test(not (compObj.ice_collocationOptimized(True) < compObj.ice_collocationOptimized(False)))

    test(compObj.ice_connectionCached(True) == compObj.ice_connectionCached(True))
    test(compObj.ice_connectionCached(False) != compObj.ice_connectionCached(True))
    test(compObj.ice_connectionCached(False) < compObj.ice_connectionCached(True))
    test(not (compObj.ice_connectionCached(True) < compObj.ice_connectionCached(False)))

    test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) == \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random))
    test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) != \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered))
    test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random) < \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered))
    test(not (compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered) < \
         compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random)))

    test(compObj.ice_connectionId("id2") == compObj.ice_connectionId("id2"))
    test(compObj.ice_connectionId("id1") != compObj.ice_connectionId("id2"))
    test(compObj.ice_connectionId("id1") < compObj.ice_connectionId("id2"))
    test(not (compObj.ice_connectionId("id2") < compObj.ice_connectionId("id1")))
    test(compObj.ice_connectionId("id1").ice_getConnectionId() == "id1")
    test(compObj.ice_connectionId("id2").ice_getConnectionId() == "id2")

    test(compObj.ice_compress(True) == compObj.ice_compress(True))
    test(compObj.ice_compress(False) != compObj.ice_compress(True))
    test(compObj.ice_compress(False) < compObj.ice_compress(True))
    test(not (compObj.ice_compress(True) < compObj.ice_compress(False)))

    test(compObj.ice_getCompress() == Ice.Unset);
    test(compObj.ice_compress(True).ice_getCompress() == True);
    test(compObj.ice_compress(False).ice_getCompress() == False);

    test(compObj.ice_timeout(20) == compObj.ice_timeout(20))
    test(compObj.ice_timeout(10) != compObj.ice_timeout(20))
    test(compObj.ice_timeout(10) < compObj.ice_timeout(20))
    test(not (compObj.ice_timeout(20) < compObj.ice_timeout(10)))

    test(compObj.ice_getTimeout() == Ice.Unset);
    test(compObj.ice_timeout(10).ice_getTimeout() == 10);
    test(compObj.ice_timeout(20).ice_getTimeout() == 20);

    loc1 = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy("loc1:default -p 10000"))
    loc2 = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy("loc2:default -p 10000"))
    test(compObj.ice_locator(None) == compObj.ice_locator(None))
    test(compObj.ice_locator(loc1) == compObj.ice_locator(loc1))
    test(compObj.ice_locator(loc1) != compObj.ice_locator(None))
    test(compObj.ice_locator(None) != compObj.ice_locator(loc2))
    test(compObj.ice_locator(loc1) != compObj.ice_locator(loc2))
    test(compObj.ice_locator(None) < compObj.ice_locator(loc1))
    test(not (compObj.ice_locator(loc1) < compObj.ice_locator(None)))
    test(compObj.ice_locator(loc1) < compObj.ice_locator(loc2))
    test(not (compObj.ice_locator(loc2) < compObj.ice_locator(loc1)))

    rtr1 = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("rtr1:default -p 10000"))
    rtr2 = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("rtr2:default -p 10000"))
    test(compObj.ice_router(None) == compObj.ice_router(None))
    test(compObj.ice_router(rtr1) == compObj.ice_router(rtr1))
    test(compObj.ice_router(rtr1) != compObj.ice_router(None))
    test(compObj.ice_router(None) != compObj.ice_router(rtr2))
    test(compObj.ice_router(rtr1) != compObj.ice_router(rtr2))
    test(compObj.ice_router(None) < compObj.ice_router(rtr1))
    test(not (compObj.ice_router(rtr1) < compObj.ice_router(None)))
    test(compObj.ice_router(rtr1) < compObj.ice_router(rtr2))
    test(not (compObj.ice_router(rtr2) < compObj.ice_router(rtr1)))

    ctx1 = { }
    ctx1["ctx1"] = "v1"
    ctx2 = { }
    ctx2["ctx2"] = "v2"
    test(compObj.ice_context({ }) == compObj.ice_context({ }))
    test(compObj.ice_context(ctx1) == compObj.ice_context(ctx1))
    test(compObj.ice_context(ctx1) != compObj.ice_context({ }))
    test(compObj.ice_context({ }) != compObj.ice_context(ctx2))
    test(compObj.ice_context(ctx1) != compObj.ice_context(ctx2))
    test(compObj.ice_context(ctx1) < compObj.ice_context(ctx2))
    test(not (compObj.ice_context(ctx2) < compObj.ice_context(ctx1)))

    test(compObj.ice_preferSecure(True) == compObj.ice_preferSecure(True))
    test(compObj.ice_preferSecure(True) != compObj.ice_preferSecure(False))
    test(compObj.ice_preferSecure(False) < compObj.ice_preferSecure(True))
    test(not (compObj.ice_preferSecure(True) < compObj.ice_preferSecure(False)))

    compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")
    compObj2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001")
    test(compObj1 != compObj2)
    test(compObj1 < compObj2)
    test(not (compObj2 < compObj1))

    compObj1 = communicator.stringToProxy("foo@MyAdapter1")
    compObj2 = communicator.stringToProxy("foo@MyAdapter2")
    test(compObj1 != compObj2)
    test(compObj1 < compObj2)
    test(not (compObj2 < compObj1))

    test(compObj1.ice_locatorCacheTimeout(20) == compObj1.ice_locatorCacheTimeout(20))
    test(compObj1.ice_locatorCacheTimeout(10) != compObj1.ice_locatorCacheTimeout(20))
    test(compObj1.ice_locatorCacheTimeout(10) < compObj1.ice_locatorCacheTimeout(20))
    test(not (compObj1.ice_locatorCacheTimeout(20) < compObj1.ice_locatorCacheTimeout(10)))

    test(compObj1.ice_invocationTimeout(20) == compObj1.ice_invocationTimeout(20));
    test(compObj1.ice_invocationTimeout(10) != compObj1.ice_invocationTimeout(20));
    test(compObj1.ice_invocationTimeout(10) < compObj1.ice_invocationTimeout(20));
    test(not (compObj1.ice_invocationTimeout(20) < compObj1.ice_invocationTimeout(10)));

    compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000")
    compObj2 = communicator.stringToProxy("foo@MyAdapter1")
    test(compObj1 != compObj2)
    test(compObj1 < compObj2)
    test(not (compObj2 < compObj1))

    endpts1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000").ice_getEndpoints()
    endpts2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001").ice_getEndpoints()
    test(endpts1 != endpts2)
    test(endpts1 < endpts2)
    test(not (endpts2 < endpts1))
    test(endpts1 == communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000").ice_getEndpoints())

    test(compObj1.ice_encodingVersion(Ice.Encoding_1_0) == compObj1.ice_encodingVersion(Ice.Encoding_1_0))
    test(compObj1.ice_encodingVersion(Ice.Encoding_1_0) != compObj1.ice_encodingVersion(Ice.Encoding_1_1))
    test(compObj.ice_encodingVersion(Ice.Encoding_1_0) < compObj.ice_encodingVersion(Ice.Encoding_1_1))
    test(not (compObj.ice_encodingVersion(Ice.Encoding_1_1) < compObj.ice_encodingVersion(Ice.Encoding_1_0)))

    baseConnection = base.ice_getConnection();
    if baseConnection:
        baseConnection2 = base.ice_connectionId("base2").ice_getConnection();
        compObj1 = compObj1.ice_fixed(baseConnection);
        compObj2 = compObj2.ice_fixed(baseConnection2);
        test(compObj1 != compObj2);

    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
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

    print("ok")

    sys.stdout.write("testing checked cast with context... ")
    sys.stdout.flush()
    tccp = Test.MyClassPrx.checkedCast(base)
    c = tccp.getContext()
    test(c == None or len(c) == 0)

    c = { }
    c["one"] = "hello"
    c["two"] =  "world"
    tccp = Test.MyClassPrx.checkedCast(base, c)
    c2 = tccp.getContext()
    test(c == c2)
    print("ok")

    sys.stdout.write("testing ice_fixed... ")
    sys.stdout.flush()
    connection = cl.ice_getConnection()
    if connection != None:
        cl.ice_fixed(connection).getContext()
        test(cl.ice_secure(True).ice_fixed(connection).ice_isSecure())
        test(cl.ice_facet("facet").ice_fixed(connection).ice_getFacet() == "facet")
        test(cl.ice_oneway().ice_fixed(connection).ice_isOneway())
        ctx = { }
        ctx["one"] = "hello"
        ctx["two"] =  "world"
        test(len(cl.ice_fixed(connection).ice_getContext()) == 0);
        test(len(cl.ice_context(ctx).ice_fixed(connection).ice_getContext()) == 2);
        test(cl.ice_fixed(connection).ice_getInvocationTimeout() == -1);
        test(cl.ice_invocationTimeout(10).ice_fixed(connection).ice_getInvocationTimeout() == 10);
        test(cl.ice_fixed(connection).ice_getConnection() == connection)
        test(cl.ice_fixed(connection).ice_fixed(connection).ice_getConnection() == connection)
        test(cl.ice_fixed(connection).ice_getTimeout() == Ice.Unset)
        fixedConnection = cl.ice_connectionId("ice_fixed").ice_getConnection()
        test(cl.ice_fixed(connection).ice_fixed(fixedConnection).ice_getConnection() == fixedConnection)
        try:
            cl.ice_secure(not connection.getEndpoint().getInfo().secure()).ice_fixed(connection).ice_ping();
        except Ice.NoEndpointException:
            pass
        try:
            cl.ice_datagram().ice_fixed(connection).ice_ping();
        except Ice.NoEndpointException:
            pass
    else:
        try:
            cl.ice_fixed(connection)
            test(False)
        except:
            # Expected with null connection.
            pass
    print("ok")

    sys.stdout.write("testing encoding versioning... ")
    sys.stdout.flush()
    ref20 = "test -e 2.0:{0}".format(helper.getTestEndpoint())
    cl20 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref20));
    try:
        cl20.ice_ping()
        test(False)
    except Ice.UnsupportedEncodingException:
        # Server 2.0 endpoint doesn't support 1.1 version.
        pass

    ref10 = "test -e 1.0:{0}".format(helper.getTestEndpoint())
    cl10 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref10))
    cl10.ice_ping()
    cl10.ice_encodingVersion(Ice.Encoding_1_0).ice_ping()
    cl.ice_encodingVersion(Ice.Encoding_1_0).ice_ping()

    # 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
    # call will use the 1.1 encoding
    ref13 = "test -e 1.3:{0}".format(helper.getTestEndpoint())
    cl13 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref13))
    cl13.ice_ping()

    print("ok")

    sys.stdout.write("testing opaque endpoints... ")
    sys.stdout.flush()

    try:
        # Invalid -x option
        p = communicator.stringToProxy("id:opaque -t 99 -v abc -x abc")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Missing -t and -v
        p = communicator.stringToProxy("id:opaque")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Repeated -t
        p = communicator.stringToProxy("id:opaque -t 1 -t 1 -v abc")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Repeated -v
        p = communicator.stringToProxy("id:opaque -t 1 -v abc -v abc")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Missing -t
        p = communicator.stringToProxy("id:opaque -v abc")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Missing -v
        p = communicator.stringToProxy("id:opaque -t 1")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Missing arg for -t
        p = communicator.stringToProxy("id:opaque -t -v abc")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Missing arg for -v
        p = communicator.stringToProxy("id:opaque -t 1 -v")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Not a number for -t
        p = communicator.stringToProxy("id:opaque -t x -v abc")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # < 0 for -t
        p = communicator.stringToProxy("id:opaque -t -1 -v abc")
        test(False)
    except Ice.EndpointParseException:
        pass

    try:
        # Invalid char for -v
        p = communicator.stringToProxy("id:opaque -t 99 -v x?c")
        test(False)
    except Ice.EndpointParseException:
        pass

    # Legal TCP endpoint expressed as opaque endpoint
    p1 = communicator.stringToProxy("test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==")
    pstr = communicator.proxyToString(p1)
    test(pstr == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000")

    # Opaque endpoint encoded with 1.1 encoding.
    p2 = communicator.stringToProxy("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==")
    test(communicator.proxyToString(p2) == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000")

    if communicator.getProperties().getPropertyAsInt("Ice.IPv6") == 0:
        # Working?
        ssl = communicator.getProperties().getProperty("Ice.Default.Protocol") == "ssl"
        tcp = communicator.getProperties().getProperty("Ice.Default.Protocol") == "tcp"

        # Two legal TCP endpoints expressed as opaque endpoints
        p1 = communicator.stringToProxy("test -e 1.0:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMusuAAAQJwAAAA==")
        pstr = communicator.proxyToString(p1)
        test(pstr == "test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000")

        #
        # Test that an SSL endpoint and a nonsense endpoint get written
        # back out as an opaque endpoint.
        #
        p1 = communicator.stringToProxy("test -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch")
        pstr = communicator.proxyToString(p1)
        if ssl:
            test(pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch")
        elif tcp:
            test(pstr == "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch")

        #
        # Try to invoke on the SSL endpoint to verify that we get a
        # NoEndpointException (or ConnectionRefusedException when
        # running with SSL).
        #
        try:
            p1.ice_encodingVersion(Ice.Encoding_1_0).ice_ping()
            test(False)
        except Ice.NoEndpointException:
            test(not ssl)
        except Ice.ConnectFailedException:
            test(not tcp)

        #
        # Test that the proxy with an SSL endpoint and a nonsense
        # endpoint (which the server doesn't understand either) can be
        # sent over the wire and returned by the server without losing
        # the opaque endpoints.
        #
        p2 = derived.echo(p1)
        pstr = communicator.proxyToString(p2)
        if ssl:
            test(pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch")
        elif tcp:
            test(pstr == "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch")

    print("ok")

    sys.stdout.write("testing communicator shutdown/destroy... ");
    sys.stdout.flush()
    c = Ice.initialize();
    c.shutdown();
    test(c.isShutdown());
    c.waitForShutdown();
    c.destroy();
    c.shutdown();
    test(c.isShutdown());
    c.waitForShutdown();
    c.destroy();
    print("ok");

    return cl
