# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def allTests(communicator)
    print "testing stringToProxy... "
    STDOUT.flush
    ref = "test:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)

    b1 = communicator.stringToProxy("test")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getAdapterId().empty? && b1.ice_getFacet().empty?)
    b1 = communicator.stringToProxy("test ")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet().empty?)
    b1 = communicator.stringToProxy(" test ")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet().empty?)
    b1 = communicator.stringToProxy(" test")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet().empty?)
    b1 = communicator.stringToProxy("'test -f facet'")
    test(b1.ice_getIdentity().name == "test -f facet" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet().empty?)
    begin
        b1 = communicator.stringToProxy("\"test -f facet'")
        test(false)
    rescue Ice::ProxyParseException
    end
    b1 = communicator.stringToProxy("\"test -f facet\"")
    test(b1.ice_getIdentity().name == "test -f facet" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet().empty?)
    b1 = communicator.stringToProxy("\"test -f facet@test\"")
    test(b1.ice_getIdentity().name == "test -f facet@test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet().empty?)
    b1 = communicator.stringToProxy("\"test -f facet@test @test\"")
    test(b1.ice_getIdentity().name == "test -f facet@test @test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet().empty?)
    begin
        b1 = communicator.stringToProxy("test test")
        test(false)
    rescue Ice::ProxyParseException
    end
    b1 = communicator.stringToProxy("test\\040test")
    test(b1.ice_getIdentity().name == "test test" && b1.ice_getIdentity().category.empty?)
    begin
        b1 = communicator.stringToProxy("test\\777")
        test(false)
    rescue Ice::IdentityParseException
    end
    b1 = communicator.stringToProxy("test\\40test")
    test(b1.ice_getIdentity().name == "test test")

    # Test some octal && hex corner cases.
    b1 = communicator.stringToProxy("test\\4test")
    test(b1.ice_getIdentity().name == "test\4test")
    b1 = communicator.stringToProxy("test\\04test")
    test(b1.ice_getIdentity().name == "test\4test")
    b1 = communicator.stringToProxy("test\\004test")
    test(b1.ice_getIdentity().name == "test\4test")
    b1 = communicator.stringToProxy("test\\1114test")
    test(b1.ice_getIdentity().name == "test\1114test")

    b1 = communicator.stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test")
    test(b1.ice_getIdentity().name == "test\b\f\n\r\t\'\"\\test" && b1.ice_getIdentity().category.empty?)

    b1 = communicator.stringToProxy("category/test")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" && \
         b1.ice_getAdapterId().empty?)

    b1 = communicator.stringToProxy("test@adapter")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getAdapterId() == "adapter")
    begin
        b1 = communicator.stringToProxy("id@adapter test")
        test(false)
    rescue Ice::ProxyParseException
    end
    b1 = communicator.stringToProxy("category/test@adapter")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" && \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("category/test@adapter:tcp")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" && \
         b1.ice_getAdapterId() == "adapter:tcp")
    b1 = communicator.stringToProxy("'category 1/test'@adapter")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category 1" && \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("'category/test 1'@adapter")
    test(b1.ice_getIdentity().name == "test 1" && b1.ice_getIdentity().category == "category" && \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("'category/test'@'adapter 1'")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" && \
         b1.ice_getAdapterId() == "adapter 1")
    b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@adapter")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category /test@foo" && \
         b1.ice_getAdapterId() == "adapter")
    b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category /test@foo" && \
         b1.ice_getAdapterId() == "adapter:tcp")

    b1 = communicator.stringToProxy("id -f facet")
    test(b1.ice_getIdentity().name == "id" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet")
    b1 = communicator.stringToProxy("id -f 'facet x'")
    test(b1.ice_getIdentity().name == "id" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet x")
    b1 = communicator.stringToProxy("id -f \"facet x\"")
    test(b1.ice_getIdentity().name == "id" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet x")
    begin
        b1 = communicator.stringToProxy("id -f \"facet x")
        test(false)
    rescue Ice::ProxyParseException
    end
    begin
        b1 = communicator.stringToProxy("id -f \'facet x")
        test(false)
    rescue Ice::ProxyParseException
    end
    b1 = communicator.stringToProxy("test -f facet:tcp")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet" && b1.ice_getAdapterId().empty?)
    b1 = communicator.stringToProxy("test -f \"facet:tcp\"")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet:tcp" && b1.ice_getAdapterId().empty?)
    b1 = communicator.stringToProxy("test -f facet@test")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet" && b1.ice_getAdapterId() == "test")
    b1 = communicator.stringToProxy("test -f 'facet@test'")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet@test" && b1.ice_getAdapterId().empty?)
    b1 = communicator.stringToProxy("test -f 'facet@test'@test")
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getFacet() == "facet@test" && b1.ice_getAdapterId() == "test")
    begin
        b1 = communicator.stringToProxy("test -f facet@test @test")
        test(false)
    rescue Ice::ProxyParseException
    end
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
    test(!b1.ice_isSecure())
    b1 = communicator.stringToProxy("test -s")
    test(b1.ice_isSecure())

    test(b1.ice_getEncodingVersion() == Ice::currentEncoding());

    b1 = communicator.stringToProxy("test -e 1.0");
    test(b1.ice_getEncodingVersion().major == 1 && b1.ice_getEncodingVersion().minor == 0);

    b1 = communicator.stringToProxy("test -e 6.5");
    test(b1.ice_getEncodingVersion().major == 6 && b1.ice_getEncodingVersion().minor == 5);

    begin
        b1 = communicator.stringToProxy("test:tcp@adapterId")
        test(false)
    rescue Ice::EndpointParseException
    end
    # This is an unknown endpoint warning, not a parse rescueion.
    #
    #begin
    #   b1 = communicator.stringToProxy("test -f the:facet:tcp")
    #   test(false)
    #rescue Ice::EndpointParseException
    #end
    begin
        b1 = communicator.stringToProxy("test::tcp")
        test(false)
    rescue Ice::EndpointParseException
    end
    puts "ok"

    print "testing propertyToProxy... "
    STDOUT.flush
    prop = communicator.getProperties()
    propertyPrefix = "Foo.Proxy"
    prop.setProperty(propertyPrefix, "test:default -p 12010")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getAdapterId().empty? && b1.ice_getFacet().empty?)

    property = propertyPrefix + ".Locator"
    test(!b1.ice_getLocator())
    prop.setProperty(property, "locator:default -p 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getLocator() && b1.ice_getLocator().ice_getIdentity().name == "locator")
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
    test(b1.ice_getLocator() && b1.ice_getLocator().ice_getIdentity().name == "locator")
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
    #prop.setProperty("Ice::Default.LocatorCacheTimeout", "60")
    #b1 = communicator.propertyToProxy(propertyPrefix)
    #test(b1.ice_getLocatorCacheTimeout() == 60)
    #prop.setProperty("Ice::Default.LocatorCacheTimeout", "")

    prop.setProperty(propertyPrefix, "test:default -p 12010")

    property = propertyPrefix + ".Router"
    test(!b1.ice_getRouter())
    prop.setProperty(property, "router:default -p 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getRouter() && b1.ice_getRouter().ice_getIdentity().name == "router")
    prop.setProperty(property, "")

    property = propertyPrefix + ".PreferSecure"
    test(!b1.ice_isPreferSecure())
    prop.setProperty(property, "1")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_isPreferSecure())
    prop.setProperty(property, "")

    property = propertyPrefix + ".ConnectionCached"
    test(b1.ice_isConnectionCached())
    prop.setProperty(property, "0")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(!b1.ice_isConnectionCached())
    prop.setProperty(property, "")

    property = propertyPrefix + ".InvocationTimeout";
    test(b1.ice_getInvocationTimeout() == -1);
    prop.setProperty(property, "1000");
    b1 = communicator.propertyToProxy(propertyPrefix);
    test(b1.ice_getInvocationTimeout() == 1000);
    prop.setProperty(property, "");

    property = propertyPrefix + ".EndpointSelection"
    test(b1.ice_getEndpointSelection() == Ice::EndpointSelectionType::Random)
    prop.setProperty(property, "Random")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getEndpointSelection() == Ice::EndpointSelectionType::Random)
    prop.setProperty(property, "Ordered")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getEndpointSelection() == Ice::EndpointSelectionType::Ordered)
    prop.setProperty(property, "")

    #
    # isCollocationOptimized is not implemented because the
    # collocation optimization is permanently disabled with IcePy.
    #
    #property = propertyPrefix + ".CollocationOptimized"
    #test(b1.ice_isCollocationOptimized())
    #prop.setProperty(property, "0")
    #b1 = communicator.propertyToProxy(propertyPrefix)
    #test(!b1.ice_isCollocationOptimized())
    #prop.setProperty(property, "")

    puts "ok"

    print "testing proxyToProperty... "
    STDOUT.flush

    b1 = communicator.stringToProxy("test")
    #b1 = b1.ice_collocationOptimized(true)
    b1 = b1.ice_connectionCached(true)
    b1 = b1.ice_preferSecure(false)
    b1 = b1.ice_endpointSelection(Ice::EndpointSelectionType::Ordered)
    b1 = b1.ice_locatorCacheTimeout(100)
    b1 = b1.ice_invocationTimeout(1234);
    b1 = b1.ice_encodingVersion(Ice::EncodingVersion.new(1, 0))

    router = communicator.stringToProxy("router")
    #router = router.ice_collocationOptimized(false)
    router = router.ice_connectionCached(true)
    router = router.ice_preferSecure(true)
    router = router.ice_endpointSelection(Ice::EndpointSelectionType::Random)
    router = router.ice_locatorCacheTimeout(200)
    router = router.ice_invocationTimeout(1500);

    locator = communicator.stringToProxy("locator")
    #locator = locator.ice_collocationOptimized(true)
    locator = locator.ice_connectionCached(false)
    locator = locator.ice_preferSecure(true)
    locator = locator.ice_endpointSelection(Ice::EndpointSelectionType::Random)
    locator = locator.ice_locatorCacheTimeout(300)
    locator = locator.ice_invocationTimeout(1500);

    locator = locator.ice_router(Ice::RouterPrx::uncheckedCast(router))
    b1 = b1.ice_locator(Ice::LocatorPrx::uncheckedCast(locator))

    proxyProps = communicator.proxyToProperty(b1, "Test")
    test(proxyProps.length() == 21)

    test(proxyProps["Test"] == "test -t -e 1.0")
    #test(proxyProps["Test.CollocationOptimized"] == "1")
    test(proxyProps["Test.ConnectionCached"] == "1")
    test(proxyProps["Test.PreferSecure"] == "0")
    test(proxyProps["Test.EndpointSelection"] == "Ordered")
    test(proxyProps["Test.LocatorCacheTimeout"] == "100")
    test(proxyProps["Test.InvocationTimeout"] == "1234");

    test(proxyProps["Test.Locator"] == "locator -t -e " + Ice::encodingVersionToString(Ice::currentEncoding()))
    #test(proxyProps["Test.Locator.CollocationOptimized"] == "1")
    test(proxyProps["Test.Locator.ConnectionCached"] == "0")
    test(proxyProps["Test.Locator.PreferSecure"] == "1")
    test(proxyProps["Test.Locator.EndpointSelection"] == "Random")
    test(proxyProps["Test.Locator.LocatorCacheTimeout"] == "300")
    test(proxyProps["Test.Locator.InvocationTimeout"] == "1500");

    test(proxyProps["Test.Locator.Router"] == "router -t -e " + Ice::encodingVersionToString(Ice::currentEncoding()));
    #test(proxyProps["Test.Locator.Router.CollocationOptimized"] == "0")
    test(proxyProps["Test.Locator.Router.ConnectionCached"] == "1")
    test(proxyProps["Test.Locator.Router.PreferSecure"] == "1")
    test(proxyProps["Test.Locator.Router.EndpointSelection"] == "Random")
    test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200")
    test(proxyProps["Test.Locator.Router.InvocationTimeout"] == "1500");

    puts "ok"

    print "testing ice_getCommunicator... "
    STDOUT.flush
    test(base.ice_getCommunicator() == communicator)
    puts "ok"

    print "testing proxy methods... "
    STDOUT.flush
    test(communicator.identityToString(base.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()) == "other")

    #
    # Verify that ToStringMode is passed correctly
    #
    ident = Ice::Identity.new("test", "\x7f\u20ac")

    idStr = Ice::identityToString(ident, Ice::ToStringMode::Unicode)
    test(idStr == "\\u007f\u20ac/test")
    ident2 = Ice::stringToIdentity(idStr)
    test(Ice::identityToString(ident) == idStr)

    idStr = Ice::identityToString(ident, Ice::ToStringMode::ASCII)
    test(idStr == "\\u007f\\u20ac/test")
    ident2 = Ice::stringToIdentity(idStr)
    test(ident.name == ident2.name)

    test(ident.category == ident2.category)

    idStr = Ice::identityToString(ident, Ice::ToStringMode::Compat)
    test(idStr == "\\177\\342\\202\\254/test")
    ident2 = Ice::stringToIdentity(idStr)
    test(ident == ident2)

    ident2 = Ice::stringToIdentity(communicator.identityToString(ident))
    test(ident == ident2)

    test(base.ice_facet("facet").ice_getFacet() == "facet")
    test(base.ice_adapterId("id").ice_getAdapterId() == "id")
    test(base.ice_twoway().ice_isTwoway())
    test(base.ice_oneway().ice_isOneway())
    test(base.ice_batchOneway().ice_isBatchOneway())
    test(base.ice_datagram().ice_isDatagram())
    test(base.ice_batchDatagram().ice_isBatchDatagram())
    test(base.ice_secure(true).ice_isSecure())
    test(!base.ice_secure(false).ice_isSecure())
    test(base.ice_preferSecure(true).ice_isPreferSecure())
    test(!base.ice_preferSecure(false).ice_isPreferSecure())
    test(base.ice_encodingVersion(Ice::Encoding_1_0).ice_getEncodingVersion() == Ice::Encoding_1_0)
    test(base.ice_encodingVersion(Ice::Encoding_1_1).ice_getEncodingVersion() == Ice::Encoding_1_1)
    test(base.ice_encodingVersion(Ice::Encoding_1_0).ice_getEncodingVersion() != Ice::Encoding_1_1)

    begin
        base.ice_timeout(0)
        test(false)
    rescue
    end

    begin
        base.ice_timeout(-1)
    rescue
        test(false)
    end

    begin
        base.ice_timeout(-2)
        test(false)
    rescue
    end

    begin
        base.ice_invocationTimeout(0)
        test(false)
    rescue
    end

    begin
        base.ice_invocationTimeout(-1)
    rescue
        test(false)
    end

    begin
        base.ice_invocationTimeout(-2)
        test(false)
    rescue
    end

    begin
        base.ice_locatorCacheTimeout(0)
    rescue
        test(false)
    end

    begin
        base.ice_locatorCacheTimeout(-1)
    rescue
        test(false)
    end

    begin
        base.ice_locatorCacheTimeout(-2)
        test(false)
    rescue
    end

    puts "ok"

    print "testing proxy comparison... "
    STDOUT.flush

    test(communicator.stringToProxy("foo") == communicator.stringToProxy("foo"))
    test(communicator.stringToProxy("foo") != communicator.stringToProxy("foo2"))
    #test(communicator.stringToProxy("foo") < communicator.stringToProxy("foo2"))
    #test(!(communicator.stringToProxy("foo2") < communicator.stringToProxy("foo")))

    compObj = communicator.stringToProxy("foo")

    test(compObj.ice_facet("facet") == compObj.ice_facet("facet"))
    test(compObj.ice_facet("facet") != compObj.ice_facet("facet1"))
    #test(compObj.ice_facet("facet") < compObj.ice_facet("facet1"))
    #test(!(compObj.ice_facet("facet") < compObj.ice_facet("facet")))

    test(compObj.ice_oneway() == compObj.ice_oneway())
    test(compObj.ice_oneway() != compObj.ice_twoway())
    #test(compObj.ice_twoway() < compObj.ice_oneway())
    #test(!(compObj.ice_oneway() < compObj.ice_twoway()))

    test(compObj.ice_secure(true) == compObj.ice_secure(true))
    test(compObj.ice_secure(false) != compObj.ice_secure(true))
    #test(compObj.ice_secure(false) < compObj.ice_secure(true))
    #test(!(compObj.ice_secure(true) < compObj.ice_secure(false)))

    #test(compObj.ice_collocationOptimized(true) == compObj.ice_collocationOptimized(true))
    #test(compObj.ice_collocationOptimized(false) != compObj.ice_collocationOptimized(true))
    #test(compObj.ice_collocationOptimized(false) < compObj.ice_collocationOptimized(true))
    #test(!(compObj.ice_collocationOptimized(true) < compObj.ice_collocationOptimized(false)))

    test(compObj.ice_connectionCached(true) == compObj.ice_connectionCached(true))
    test(compObj.ice_connectionCached(false) != compObj.ice_connectionCached(true))
    #test(compObj.ice_connectionCached(false) < compObj.ice_connectionCached(true))
    #test(!(compObj.ice_connectionCached(true) < compObj.ice_connectionCached(false)))

    test(compObj.ice_endpointSelection(Ice::EndpointSelectionType::Random) == \
         compObj.ice_endpointSelection(Ice::EndpointSelectionType::Random))
    test(compObj.ice_endpointSelection(Ice::EndpointSelectionType::Random) != \
         compObj.ice_endpointSelection(Ice::EndpointSelectionType::Ordered))
    #test(compObj.ice_endpointSelection(Ice::EndpointSelectionType::Random) < \
    #     compObj.ice_endpointSelection(Ice::EndpointSelectionType::Ordered))
    #test(!(compObj.ice_endpointSelection(Ice::EndpointSelectionType::Ordered) < \
    #     compObj.ice_endpointSelection(Ice::EndpointSelectionType::Random)))

    test(compObj.ice_connectionId("id2") == compObj.ice_connectionId("id2"))
    test(compObj.ice_connectionId("id1") != compObj.ice_connectionId("id2"))
    test(compObj.ice_connectionId("id1").ice_getConnectionId() == "id1")
    test(compObj.ice_connectionId("id2").ice_getConnectionId() == "id2")
    #test(compObj.ice_connectionId("id1") < compObj.ice_connectionId("id2"))
    #test(!(compObj.ice_connectionId("id2") < compObj.ice_connectionId("id1")))

    test(compObj.ice_compress(true) == compObj.ice_compress(true))
    test(compObj.ice_compress(false) != compObj.ice_compress(true))
    #test(compObj.ice_compress(false) < compObj.ice_compress(true))
    #test(!(compObj.ice_compress(true) < compObj.ice_compress(false)))

    test(compObj.ice_timeout(20) == compObj.ice_timeout(20))
    test(compObj.ice_timeout(10) != compObj.ice_timeout(20))
    #test(compObj.ice_timeout(10) < compObj.ice_timeout(20))
    #test(!(compObj.ice_timeout(20) < compObj.ice_timeout(10)))

    loc1 = Ice::LocatorPrx::uncheckedCast(communicator.stringToProxy("loc1:default -p 10000"))
    loc2 = Ice::LocatorPrx::uncheckedCast(communicator.stringToProxy("loc2:default -p 10000"))
    test(compObj.ice_locator(nil) == compObj.ice_locator(nil))
    test(compObj.ice_locator(loc1) == compObj.ice_locator(loc1))
    test(compObj.ice_locator(loc1) != compObj.ice_locator(nil))
    test(compObj.ice_locator(nil) != compObj.ice_locator(loc2))
    test(compObj.ice_locator(loc1) != compObj.ice_locator(loc2))
    #test(compObj.ice_locator(nil) < compObj.ice_locator(loc1))
    #test(!(compObj.ice_locator(loc1) < compObj.ice_locator(nil)))
    #test(compObj.ice_locator(loc1) < compObj.ice_locator(loc2))
    #test(!(compObj.ice_locator(loc2) < compObj.ice_locator(loc1)))

    rtr1 = Ice::RouterPrx::uncheckedCast(communicator.stringToProxy("rtr1:default -p 10000"))
    rtr2 = Ice::RouterPrx::uncheckedCast(communicator.stringToProxy("rtr2:default -p 10000"))
    test(compObj.ice_router(nil) == compObj.ice_router(nil))
    test(compObj.ice_router(rtr1) == compObj.ice_router(rtr1))
    test(compObj.ice_router(rtr1) != compObj.ice_router(nil))
    test(compObj.ice_router(nil) != compObj.ice_router(rtr2))
    test(compObj.ice_router(rtr1) != compObj.ice_router(rtr2))
    #test(compObj.ice_router(nil) < compObj.ice_router(rtr1))
    #test(!(compObj.ice_router(rtr1) < compObj.ice_router(nil)))
    #test(compObj.ice_router(rtr1) < compObj.ice_router(rtr2))
    #test(!(compObj.ice_router(rtr2) < compObj.ice_router(rtr1)))

    ctx1 = { }
    ctx1["ctx1"] = "v1"
    ctx2 = { }
    ctx2["ctx2"] = "v2"
    test(compObj.ice_context({ }) == compObj.ice_context({ }))
    test(compObj.ice_context(ctx1) == compObj.ice_context(ctx1))
    test(compObj.ice_context(ctx1) != compObj.ice_context({ }))
    test(compObj.ice_context({ }) != compObj.ice_context(ctx2))
    test(compObj.ice_context(ctx1) != compObj.ice_context(ctx2))
    #test(compObj.ice_context(ctx1) < compObj.ice_context(ctx2))
    #test(!(compObj.ice_context(ctx2) < compObj.ice_context(ctx1)))

    test(compObj.ice_preferSecure(true) == compObj.ice_preferSecure(true))
    test(compObj.ice_preferSecure(true) != compObj.ice_preferSecure(false))
    #test(compObj.ice_preferSecure(false) < compObj.ice_preferSecure(true))
    #test(!(compObj.ice_preferSecure(true) < compObj.ice_preferSecure(false)))

    compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")
    compObj2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001")
    test(compObj1 != compObj2)
    #test(compObj1 < compObj2)
    #test(!(compObj2 < compObj1))

    compObj1 = communicator.stringToProxy("foo@MyAdapter1")
    compObj2 = communicator.stringToProxy("foo@MyAdapter2")
    test(compObj1 != compObj2)
    #test(compObj1 < compObj2)
    #test(!(compObj2 < compObj1))

    test(compObj1.ice_locatorCacheTimeout(20) == compObj1.ice_locatorCacheTimeout(20))
    test(compObj1.ice_locatorCacheTimeout(10) != compObj1.ice_locatorCacheTimeout(20))
    #test(compObj1.ice_locatorCacheTimeout(10) < compObj1.ice_locatorCacheTimeout(20))
    #test(!(compObj1.ice_locatorCacheTimeout(20) < compObj1.ice_locatorCacheTimeout(10)))

    test(compObj1.ice_invocationTimeout(20) == compObj1.ice_invocationTimeout(20));
    test(compObj1.ice_invocationTimeout(10) != compObj1.ice_invocationTimeout(20));
    #test(compObj1.ice_invocationTimeout(10) < compObj1.ice_invocationTimeout(20));
    #test(not (compObj1.ice_invocationTimeout(20) < compObj1.ice_invocationTimeout(10)));

    compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000")
    compObj2 = communicator.stringToProxy("foo@MyAdapter1")
    test(compObj1 != compObj2)
    #test(compObj1 < compObj2)
    #test(!(compObj2 < compObj1))

    endpts1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000").ice_getEndpoints()
    endpts2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001").ice_getEndpoints()
    test(endpts1 != endpts2)
    #test(endpts1 < endpts2)
    #test(!(endpts2 < endpts1))
    test(endpts1 == communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000").ice_getEndpoints())

    test(compObj1.ice_encodingVersion(Ice::Encoding_1_0) == compObj1.ice_encodingVersion(Ice::Encoding_1_0))
    test(compObj1.ice_encodingVersion(Ice::Encoding_1_0) != compObj1.ice_encodingVersion(Ice::Encoding_1_1))
    #test(compObj.ice_encodingVersion(Ice::Encoding_1_0) < compObj.ice_encodingVersion(Ice::Encoding_1_1))
    #test(! (compObj.ice_encodingVersion(Ice::Encoding_1_1) < compObj.ice_encodingVersion(Ice::Encoding_1_0)))

    #
    # TODO: Ideally we should also test comparison of fixed proxies.
    #

    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    cl = Test::MyClassPrx::checkedCast(base)
    test(cl)

    derived = Test::MyDerivedClassPrx::checkedCast(cl)
    test(derived)
    test(cl == base)
    test(derived == base)
    test(cl == derived)

    loc = Ice::LocatorPrx::checkedCast(base)
    test(loc == nil)

    #
    # Upcasting
    #
    cl2 = Test::MyClassPrx::checkedCast(derived)
    obj = Ice::ObjectPrx::checkedCast(derived)
    test(cl2)
    test(obj)
    test(cl2 == obj)
    test(cl2 == derived)

    puts "ok"

    print "testing checked cast with context... "
    STDOUT.flush
    tccp = Test::MyClassPrx::checkedCast(base)
    c = tccp.getContext()
    test(c == nil || c.length == 0)

    c = { }
    c["one"] = "hello"
    c["two"] =  "world"
    tccp = Test::MyClassPrx::checkedCast(base, c)
    c2 = tccp.getContext()
    test(c == c2)
    puts "ok"

    print "testing encoding versioning... "
    STDOUT.flush
    ref20 = "test -e 2.0:default -p 12010";
    cl20 = Test::MyClassPrx::uncheckedCast(communicator.stringToProxy(ref20));
    begin
        cl20.ice_ping();
        test(false);
    rescue Ice::UnsupportedEncodingException
        # Server 2.0 endpoint doesn't support 1.1 version.
    end

    ref10 = "test -e 1.0:default -p 12010"
    cl10 = Test::MyClassPrx::uncheckedCast(communicator.stringToProxy(ref10))
    cl10.ice_ping()
    cl10.ice_encodingVersion(Ice::Encoding_1_0).ice_ping()
    cl.ice_encodingVersion(Ice::Encoding_1_0).ice_ping()

    # 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
    # call will use the 1.1 encoding
    ref13 = "test -e 1.3:default -p 12010"
    cl13 = Test::MyClassPrx::uncheckedCast(communicator.stringToProxy(ref13))
    cl13.ice_ping()

    puts "ok"

    print "testing opaque endpoints... "
    STDOUT.flush

    begin
        # Invalid -x option
        p = communicator.stringToProxy("id:opaque -t 99 -v abc -x abc")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Missing -t and -v
        p = communicator.stringToProxy("id:opaque")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Repeated -t
        p = communicator.stringToProxy("id:opaque -t 1 -t 1 -v abc")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Repeated -v
        p = communicator.stringToProxy("id:opaque -t 1 -v abc -v abc")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Missing -t
        p = communicator.stringToProxy("id:opaque -v abc")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Missing -v
        p = communicator.stringToProxy("id:opaque -t 1")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Missing arg for -t
        p = communicator.stringToProxy("id:opaque -t -v abc")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Missing arg for -v
        p = communicator.stringToProxy("id:opaque -t 1 -v")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Not a number for -t
        p = communicator.stringToProxy("id:opaque -t x -v abc")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # < 0 for -t
        p = communicator.stringToProxy("id:opaque -t -1 -v abc")
        test(false)
    rescue Ice::EndpointParseException
    end

    begin
        # Invalid char for -v
        p = communicator.stringToProxy("id:opaque -t 99 -v x?c")
        test(false)
    rescue Ice::EndpointParseException
    end

    # Legal TCP endpoint expressed as opaque endpoint.
    p1 = communicator.stringToProxy("test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==")
    pstr = communicator.proxyToString(p1)
    test(pstr == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000")

    # Opaque endpoint encoded with 1.1 encoding.
    p2 = communicator.stringToProxy("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==")
    test(communicator.proxyToString(p2) == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000")

    # Working?
    if communicator.getProperties().getPropertyAsInt("Ice.IPv6") == 0
        ssl = communicator.getProperties().getProperty("Ice.Default.Protocol") == "ssl"
        tcp = communicator.getProperties().getProperty("Ice.Default.Protocol") == "tcp"
        if tcp
            p1.ice_encodingVersion(Ice::Encoding_1_0).ice_ping()
        end

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
        if ssl
            test(pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch")
        elsif tcp
            test(pstr == "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch")
        end

        #
        # Try to invoke on the SSL endpoint to verify that we get a
        # NoEndpointException (or ConnectionRefusedException when
        # running with SSL).
        #
        begin
            p1.ice_encodingVersion(Ice::Encoding_1_0).ice_ping()
            test(false)
        rescue Ice::NoEndpointException
            test(!ssl)
        rescue Ice::ConnectionRefusedException
            test(!tcp)
        end

        #
        # Test that the proxy with an SSL endpoint and a nonsense
        # endpoint (which the server doesn't understand either) can be
        # sent over the wire and returned by the server without losing
        # the opaque endpoints.
        #
        p2 = derived.echo(p1)
        pstr = communicator.proxyToString(p2)
        if ssl
            test(pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch")
        elsif tcp
            test(pstr == "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch")
        end
    end
    puts "ok"

    return cl
end
