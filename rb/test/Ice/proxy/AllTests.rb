#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def allTests(communicator)
    print "testing stringToProxy... "
    STDOUT.flush
    ref = "test:default -p 12010 -t 10000"
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
    prop.setProperty(propertyPrefix, "test:default -p 12010 -t 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.empty? && \
         b1.ice_getAdapterId().empty? && b1.ice_getFacet().empty?)

    # These two properties don't do anything to direct proxies so
    # first we test that.
    property = propertyPrefix + ".Locator"
    test(!b1.ice_getLocator())
    prop.setProperty(property, "locator:default -p 10000")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(!b1.ice_getLocator())
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

    prop.setProperty(propertyPrefix, "test:default -p 12010 -t 10000")

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
    #property = propertyPrefix + ".CollocationOptimization"
    #test(b1.ice_isCollocationOptimized())
    #prop.setProperty(property, "0")
    #b1 = communicator.propertyToProxy(propertyPrefix)
    #test(!b1.ice_isCollocationOptimized())
    #prop.setProperty(property, "")

    property = propertyPrefix + ".ThreadPerConnection"
    test(!b1.ice_isThreadPerConnection())
    prop.setProperty(property, "1")
    b1 = communicator.propertyToProxy(propertyPrefix)
    test(b1.ice_isThreadPerConnection())
    prop.setProperty(property, "")

    puts "ok"

    print "testing ice_getCommunicator... "
    STDOUT.flush
    test(base.ice_getCommunicator() == communicator)
    puts "ok"

    print "testing proxy methods... "
    STDOUT.flush
    test(communicator.identityToString(base.ice_identity(communicator.stringToIdentity("other")).ice_getIdentity()) == "other");
    test(base.ice_facet("facet").ice_getFacet() == "facet");
    test(base.ice_adapterId("id").ice_getAdapterId() == "id");
    test(base.ice_twoway().ice_isTwoway());
    test(base.ice_oneway().ice_isOneway());
    test(base.ice_batchOneway().ice_isBatchOneway());
    test(base.ice_datagram().ice_isDatagram());
    test(base.ice_batchDatagram().ice_isBatchDatagram());
    test(base.ice_secure(true).ice_isSecure());
    test(!base.ice_secure(false).ice_isSecure());
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

    print "testing opaque endpoints... "
    STDOUT.flush

    begin
        # Invalid -x option
        p = communicator.stringToProxy("id:opaque -t 99 -v abc -x abc");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Missing -t and -v
        p = communicator.stringToProxy("id:opaque");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Repeated -t
        p = communicator.stringToProxy("id:opaque -t 1 -t 1 -v abc");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Repeated -v
        p = communicator.stringToProxy("id:opaque -t 1 -v abc -v abc");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Missing -t
        p = communicator.stringToProxy("id:opaque -v abc");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Missing -v
        p = communicator.stringToProxy("id:opaque -t 1");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Missing arg for -t
        p = communicator.stringToProxy("id:opaque -t -v abc");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Missing arg for -v
        p = communicator.stringToProxy("id:opaque -t 1 -v");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Not a number for -t
        p = communicator.stringToProxy("id:opaque -t x -v abc");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # < 0 for -t
        p = communicator.stringToProxy("id:opaque -t -1 -v abc");
        test(false);
    rescue Ice::EndpointParseException
    end

    begin
        # Invalid char for -v
        p = communicator.stringToProxy("id:opaque -t 99 -v x?c");
        test(false);
    rescue Ice::EndpointParseException
    end

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
    begin
        p1.ice_ping();
        test(false);
    rescue Ice::NoEndpointException
    end

    #
    # Test that the proxy with an SSL endpoint and a nonsense
    # endpoint (which the server doesn't understand either) can be
    # sent over the wire and returned by the server without losing
    # the opaque endpoints.
    #
    p2 = derived.echo(p1);
    pstr = communicator.proxyToString(p2);
    test(pstr == "test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");

    puts "ok"

    return cl
end
