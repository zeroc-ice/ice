// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <Ice/Router.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "test:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);

    Ice::ObjectPrx b1 = communicator->stringToProxy("test");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getAdapterId().empty() && b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("test ");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy(" test ");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy(" test");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("'test -f facet'");
    test(b1->ice_getIdentity().name == "test -f facet" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    try
    {
        b1 = communicator->stringToProxy("\"test -f facet'");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("\"test -f facet\"");
    test(b1->ice_getIdentity().name == "test -f facet" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("\"test -f facet@test\"");
    test(b1->ice_getIdentity().name == "test -f facet@test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("\"test -f facet@test @test\"");
    test(b1->ice_getIdentity().name == "test -f facet@test @test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    try
    {
        b1 = communicator->stringToProxy("test test");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("test\\040test");
    test(b1->ice_getIdentity().name == "test test" && b1->ice_getIdentity().category.empty());
    try
    {
        b1 = communicator->stringToProxy("test\\777");
        test(false);
    }
    catch(const Ice::IdentityParseException&)
    {
    }
    b1 = communicator->stringToProxy("test\\40test");
    test(b1->ice_getIdentity().name == "test test");

    // Test some octal and hex corner cases.
    b1 = communicator->stringToProxy("test\\4test");
    test(b1->ice_getIdentity().name == "test\4test");
    b1 = communicator->stringToProxy("test\\04test");
    test(b1->ice_getIdentity().name == "test\4test");
    b1 = communicator->stringToProxy("test\\004test");
    test(b1->ice_getIdentity().name == "test\4test");
    b1 = communicator->stringToProxy("test\\1114test");
    test(b1->ice_getIdentity().name == "test\1114test");

    b1 = communicator->stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test");
    test(b1->ice_getIdentity().name == "test\b\f\n\r\t\'\"\\test" && b1->ice_getIdentity().category.empty());

    b1 = communicator->stringToProxy("category/test");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category" &&
         b1->ice_getAdapterId().empty());

    b1 = communicator->stringToProxy("test@adapter");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getAdapterId() == "adapter");
    try
    {
        b1 = communicator->stringToProxy("id@adapter test");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("category/test@adapter");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category" &&
         b1->ice_getAdapterId() == "adapter");
    b1 = communicator->stringToProxy("category/test@adapter:tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category" &&
         b1->ice_getAdapterId() == "adapter:tcp");
    b1 = communicator->stringToProxy("'category 1/test'@adapter");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category 1" &&
         b1->ice_getAdapterId() == "adapter");
    b1 = communicator->stringToProxy("'category/test 1'@adapter");
    test(b1->ice_getIdentity().name == "test 1" && b1->ice_getIdentity().category == "category" &&
         b1->ice_getAdapterId() == "adapter");
    b1 = communicator->stringToProxy("'category/test'@'adapter 1'");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category" &&
         b1->ice_getAdapterId() == "adapter 1");
    b1 = communicator->stringToProxy("\"category \\/test@foo/test\"@adapter");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category /test@foo" &&
         b1->ice_getAdapterId() == "adapter");
    b1 = communicator->stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category /test@foo" &&
         b1->ice_getAdapterId() == "adapter:tcp");

    b1 = communicator->stringToProxy("id -f facet");
    test(b1->ice_getIdentity().name == "id" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet");
    b1 = communicator->stringToProxy("id -f 'facet x'");
    test(b1->ice_getIdentity().name == "id" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet x");
    b1 = communicator->stringToProxy("id -f \"facet x\"");
    test(b1->ice_getIdentity().name == "id" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet x");
    try
    {
        b1 = communicator->stringToProxy("id -f \"facet x");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    try
    {
        b1 = communicator->stringToProxy("id -f \'facet x");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("test -f facet:tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet" && b1->ice_getAdapterId().empty());
    b1 = communicator->stringToProxy("test -f \"facet:tcp\"");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet:tcp" && b1->ice_getAdapterId().empty());
    b1 = communicator->stringToProxy("test -f facet@test");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet" && b1->ice_getAdapterId() == "test");
    b1 = communicator->stringToProxy("test -f 'facet@test'");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet@test" && b1->ice_getAdapterId().empty());
    b1 = communicator->stringToProxy("test -f 'facet@test'@test");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet@test" && b1->ice_getAdapterId() == "test");
    try
    {
        b1 = communicator->stringToProxy("test -f facet@test @test");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("test");
    test(b1->ice_isTwoway());
    b1 = communicator->stringToProxy("test -t");
    test(b1->ice_isTwoway());
    b1 = communicator->stringToProxy("test -o");
    test(b1->ice_isOneway());
    b1 = communicator->stringToProxy("test -O");
    test(b1->ice_isBatchOneway());
    b1 = communicator->stringToProxy("test -d");
    test(b1->ice_isDatagram());
    b1 = communicator->stringToProxy("test -D");
    test(b1->ice_isBatchDatagram());
    b1 = communicator->stringToProxy("test");
    test(!b1->ice_isSecure());
    b1 = communicator->stringToProxy("test -s");
    test(b1->ice_isSecure());

    try
    {
        b1 = communicator->stringToProxy("test:tcp@adapterId");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }
    // This is an unknown endpoint warning, not a parse exception.
    //
    //try
    //{
    //   b1 = communicator->stringToProxy("test -f the:facet:tcp");
    //   test(false);
    //}
    //catch(const Ice::EndpointParseException&)
    //{
    //}
    try
    {
        b1 = communicator->stringToProxy("test::tcp");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing propertyToProxy... " << flush;
    Ice::PropertiesPtr prop = communicator->getProperties();
    string propertyPrefix = "Foo.Proxy";
    prop->setProperty(propertyPrefix, "test:default -p 12010 -t 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getAdapterId().empty() && b1->ice_getFacet().empty());

    string property;

    // These two properties don't do anything to direct proxies so
    // first we test that.
    /*
     * Commented out because setting a locator or locator cache
     * timeout on a direct proxy causes warning.
     *
    property = propertyPrefix + ".Locator";
    test(!b1->ice_getLocator());
    prop->setProperty(property, "locator:default -p 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(!b1->ice_getLocator());
    prop->setProperty(property, "");

    property = propertyPrefix + ".LocatorCacheTimeout";
    test(b1->ice_getLocatorCacheTimeout() == 0);
    prop->setProperty(property, "1");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocatorCacheTimeout() == 0);
    prop->setProperty(property, "");
    */

    // Now retest with an indirect proxy.
    prop->setProperty(propertyPrefix, "test");
    property = propertyPrefix + ".Locator";
    prop->setProperty(property, "locator:default -p 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocator() && b1->ice_getLocator()->ice_getIdentity().name == "locator");
    prop->setProperty(property, "");

    property = propertyPrefix + ".LocatorCacheTimeout";
    test(b1->ice_getLocatorCacheTimeout() == -1);
    prop->setProperty(property, "1");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocatorCacheTimeout() == 1);
    prop->setProperty(property, "");

    // This cannot be tested so easily because the property is cached
    // on communicator initialization.
    //
    //prop->setProperty("Ice.Default.LocatorCacheTimeout", "60");
    //b1 = communicator->propertyToProxy(propertyPrefix);
    //test(b1->ice_getLocatorCacheTimeout() == 60);
    //prop->setProperty("Ice.Default.LocatorCacheTimeout", "");

    prop->setProperty(propertyPrefix, "test:default -p 12010 -t 10000");

    property = propertyPrefix + ".Router";
    test(!b1->ice_getRouter());
    prop->setProperty(property, "router:default -p 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getRouter() && b1->ice_getRouter()->ice_getIdentity().name == "router");
    prop->setProperty(property, "");

    property = propertyPrefix + ".PreferSecure";
    test(!b1->ice_isPreferSecure());
    prop->setProperty(property, "1");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_isPreferSecure());
    prop->setProperty(property, "");

    property = propertyPrefix + ".ConnectionCached";
    test(b1->ice_isConnectionCached());
    prop->setProperty(property, "0");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(!b1->ice_isConnectionCached());
    prop->setProperty(property, "");

    property = propertyPrefix + ".EndpointSelection";
    test(b1->ice_getEndpointSelection() == Ice::Random);
    prop->setProperty(property, "Random");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getEndpointSelection() == Ice::Random);
    prop->setProperty(property, "Ordered");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getEndpointSelection() == Ice::Ordered);
    prop->setProperty(property, "");

    property = propertyPrefix + ".CollocationOptimization";
    test(b1->ice_isCollocationOptimized());
    prop->setProperty(property, "0");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(!b1->ice_isCollocationOptimized());
    prop->setProperty(property, "");

    property = propertyPrefix + ".ThreadPerConnection";
    test(!b1->ice_isThreadPerConnection());
    prop->setProperty(property, "1");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_isThreadPerConnection());
    prop->setProperty(property, "");

    cout << "ok" << endl;

    cout << "testing ice_getCommunicator... " << flush;
    test(base->ice_getCommunicator() == communicator);
    cout << "ok" << endl;

    cout << "testing proxy methods... " << flush;
    test(communicator->identityToString(base->ice_identity(communicator->stringToIdentity("other"))->ice_getIdentity())
         == "other");
    test(base->ice_facet("facet")->ice_getFacet() == "facet");
    test(base->ice_adapterId("id")->ice_getAdapterId() == "id");
    test(base->ice_twoway()->ice_isTwoway());
    test(base->ice_oneway()->ice_isOneway());
    test(base->ice_batchOneway()->ice_isBatchOneway());
    test(base->ice_datagram()->ice_isDatagram());
    test(base->ice_batchDatagram()->ice_isBatchDatagram());
    test(base->ice_secure(true)->ice_isSecure());
    test(!base->ice_secure(false)->ice_isSecure());
    test(base->ice_collocationOptimized(true)->ice_isCollocationOptimized());
    test(!base->ice_collocationOptimized(false)->ice_isCollocationOptimized());
    cout << "ok" << endl;

    cout << "testing proxy comparison... " << flush;

    test(communicator->stringToProxy("foo") == communicator->stringToProxy("foo"));
    test(communicator->stringToProxy("foo") != communicator->stringToProxy("foo2"));
    test(communicator->stringToProxy("foo") < communicator->stringToProxy("foo2"));
    test(!(communicator->stringToProxy("foo2") < communicator->stringToProxy("foo")));

    Ice::ObjectPrx compObj = communicator->stringToProxy("foo");

    test(compObj->ice_facet("facet") == compObj->ice_facet("facet"));
    test(compObj->ice_facet("facet") != compObj->ice_facet("facet1"));
    test(compObj->ice_facet("facet") < compObj->ice_facet("facet1"));
    test(!(compObj->ice_facet("facet") < compObj->ice_facet("facet")));

    test(compObj->ice_oneway() == compObj->ice_oneway());
    test(compObj->ice_oneway() != compObj->ice_twoway());
    test(compObj->ice_twoway() < compObj->ice_oneway());
    test(!(compObj->ice_oneway() < compObj->ice_twoway()));

    test(compObj->ice_secure(true) == compObj->ice_secure(true));
    test(compObj->ice_secure(false) != compObj->ice_secure(true));
    test(compObj->ice_secure(false) < compObj->ice_secure(true));
    test(!(compObj->ice_secure(true) < compObj->ice_secure(false)));

    test(compObj->ice_collocationOptimized(true) == compObj->ice_collocationOptimized(true));
    test(compObj->ice_collocationOptimized(false) != compObj->ice_collocationOptimized(true));
    test(compObj->ice_collocationOptimized(false) < compObj->ice_collocationOptimized(true));
    test(!(compObj->ice_collocationOptimized(true) < compObj->ice_collocationOptimized(false)));

    test(compObj->ice_connectionCached(true) == compObj->ice_connectionCached(true));
    test(compObj->ice_connectionCached(false) != compObj->ice_connectionCached(true));
    test(compObj->ice_connectionCached(false) < compObj->ice_connectionCached(true));
    test(!(compObj->ice_connectionCached(true) < compObj->ice_connectionCached(false)));

    test(compObj->ice_endpointSelection(Ice::Random) == compObj->ice_endpointSelection(Ice::Random));
    test(compObj->ice_endpointSelection(Ice::Random) != compObj->ice_endpointSelection(Ice::Ordered));
    test(compObj->ice_endpointSelection(Ice::Random) < compObj->ice_endpointSelection(Ice::Ordered));
    test(!(compObj->ice_endpointSelection(Ice::Ordered) < compObj->ice_endpointSelection(Ice::Random)));

    test(compObj->ice_connectionId("id2") == compObj->ice_connectionId("id2"));
    test(compObj->ice_connectionId("id1") != compObj->ice_connectionId("id2"));
    test(compObj->ice_connectionId("id1") < compObj->ice_connectionId("id2"));
    test(!(compObj->ice_connectionId("id2") < compObj->ice_connectionId("id1")));

    test(compObj->ice_compress(true) == compObj->ice_compress(true));
    test(compObj->ice_compress(false) != compObj->ice_compress(true));
    test(compObj->ice_compress(false) < compObj->ice_compress(true));
    test(!(compObj->ice_compress(true) < compObj->ice_compress(false)));

    test(compObj->ice_timeout(20) == compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) != compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) < compObj->ice_timeout(20));
    test(!(compObj->ice_timeout(20) < compObj->ice_timeout(10)));

    Ice::ObjectPrx compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
    Ice::ObjectPrx compObj2 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));

    compObj1 = communicator->stringToProxy("foo@MyAdapter1");
    compObj2 = communicator->stringToProxy("foo@MyAdapter2");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));

    test(compObj1->ice_locatorCacheTimeout(20) == compObj1->ice_locatorCacheTimeout(20));
    test(compObj1->ice_locatorCacheTimeout(10) != compObj1->ice_locatorCacheTimeout(20));
    test(compObj1->ice_locatorCacheTimeout(10) < compObj1->ice_locatorCacheTimeout(20));
    test(!(compObj1->ice_locatorCacheTimeout(20) < compObj1->ice_locatorCacheTimeout(10)));

    compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
    compObj2 = communicator->stringToProxy("foo@MyAdapter1");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));

    //
    // TODO: Ideally we should also test comparison of fixed proxies.
    //

    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
    Ice::LocatorPrx loc = Ice::LocatorPrx::checkedCast(base);
    test(loc == 0);

    //
    // Upcasting
    //
    Test::MyClassPrx cl2 = Test::MyClassPrx::checkedCast(derived);
    Ice::ObjectPrx obj = Ice::ObjectPrx::checkedCast(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    //
    // Now with alternate API
    //
    cl = checkedCast<Test::MyClassPrx>(base);
    test(cl);
    derived = checkedCast<Test::MyDerivedClassPrx>(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
    loc = checkedCast<Ice::LocatorPrx>(base);
    test(loc == 0);

    cl2 = checkedCast<Test::MyClassPrx>(derived);
    obj = checkedCast<Ice::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    cout << "ok" << endl;

    cout << "testing checked cast with context... " << flush;
    Ice::Context c = cl->getContext();
    test(c.size() == 0);

    c["one"] = "hello";
    c["two"] = "world";
    cl = Test::MyClassPrx::checkedCast(base, c);
    Ice::Context c2 = cl->getContext();
    test(c == c2);

    //
    // Now with alternate API
    //
    cl = checkedCast<Test::MyClassPrx>(base);
    c = cl->getContext();
    test(c.size() == 0);

    cl = checkedCast<Test::MyClassPrx>(base, c);
    c2 = cl->getContext();
    test(c == c2);

    cout << "ok" << endl;

    cout << "testing opaque endpoints... " << flush;

    try
    {
        // Invalid -x option
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t 99 -v abc -x abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing -t and -v
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Repeated -t
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t 1 -t 1 -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Repeated -v
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t 1 -v abc -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing -t
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing -v
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t 1");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing arg for -t
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing arg for -v
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t 1 -v");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Not a number for -t
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t x -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // < 0 for -t
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t -1 -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Invalid char for -v
        Ice::ObjectPrx p = communicator->stringToProxy("id:opaque -t 99 -v x?c");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    // Legal TCP endpoint expressed as opaque endpoint
    Ice::ObjectPrx p1 = communicator->stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    string pstr = communicator->proxyToString(p1);
    test(pstr == "test -t:tcp -h 127.0.0.1 -p 12010 -t 10000");
    
    // Working?
    p1->ice_ping();

    // Two legal TCP endpoints expressed as opaque endpoints
    p1 = communicator->stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
    pstr = communicator->proxyToString(p1);
    test(pstr == "test -t:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

    //
    // Test that an SSL endpoint and a nonsense endpoint get written
    // back out as an opaque endpoint.
    //
    p1 = communicator->stringToProxy("test:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");
    pstr = communicator->proxyToString(p1);
    test(pstr == "test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");

    //
    // Try to invoke on the SSL endpoint to verify that we get a
    // NoEndpointException.
    //
    try
    {
        p1->ice_ping();
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }

    //
    // Test that the proxy with an SSL endpoint and a nonsense
    // endpoint (which the server doesn't understand either) can be
    // sent over the wire and returned by the server without losing
    // the opaque endpoints.
    //
    Ice::ObjectPrx p2 = derived->echo(p1);
    pstr = communicator->proxyToString(p2);
    test(pstr == "test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");

    cout << "ok" << endl;

    return cl;
}
