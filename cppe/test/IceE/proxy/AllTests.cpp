// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#ifdef ICEE_HAS_ROUTER
#include <IceE/Router.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#include <IceE/Locator.h>
#endif
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = "test:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);

    Ice::ObjectPrx b1 = communicator->stringToProxy("test:tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("test :tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy(" test :tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy(" test:tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("'test -f facet':tcp");
    test(b1->ice_getIdentity().name == "test -f facet" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    try
    {
        b1 = communicator->stringToProxy("\"test -f facet':tcp");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("\"test -f facet\":tcp");
    test(b1->ice_getIdentity().name == "test -f facet" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("\"test -f facet@test\":tcp");
    test(b1->ice_getIdentity().name == "test -f facet@test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    b1 = communicator->stringToProxy("\"test -f facet@test @test\":tcp");
    test(b1->ice_getIdentity().name == "test -f facet@test @test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());
    try
    {
        b1 = communicator->stringToProxy("test test:tcp");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("test\\040test:tcp");
    test(b1->ice_getIdentity().name == "test test" && b1->ice_getIdentity().category.empty());
    try
    {
        b1 = communicator->stringToProxy("test\\777:tcp");
        test(false);
    }
    catch(const Ice::IdentityParseException&)
    {
    }
    b1 = communicator->stringToProxy("test\\40test:tcp");
    test(b1->ice_getIdentity().name == "test test");

    // Test some octal and hex corner cases.
    b1 = communicator->stringToProxy("test\\4test:tcp");
    test(b1->ice_getIdentity().name == "test\4test");
    b1 = communicator->stringToProxy("test\\04test:tcp");
    test(b1->ice_getIdentity().name == "test\4test");
    b1 = communicator->stringToProxy("test\\004test:tcp");
    test(b1->ice_getIdentity().name == "test\4test");
    b1 = communicator->stringToProxy("test\\1114test:tcp");
    test(b1->ice_getIdentity().name == "test\1114test");

    b1 = communicator->stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test:tcp");
    test(b1->ice_getIdentity().name == "test\b\f\n\r\t\'\"\\test" && b1->ice_getIdentity().category.empty());

    b1 = communicator->stringToProxy("category/test:tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category");

#ifdef ICEE_HAS_LOCATOR
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
#endif

    b1 = communicator->stringToProxy("id -f facet:tcp");
    test(b1->ice_getIdentity().name == "id" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet");
    b1 = communicator->stringToProxy("id -f 'facet x':tcp");
    test(b1->ice_getIdentity().name == "id" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet x");
    b1 = communicator->stringToProxy("id -f \"facet x\":tcp");
    test(b1->ice_getIdentity().name == "id" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet x");
    try
    {
        b1 = communicator->stringToProxy("id -f \"facet x:tcp");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    try
    {
        b1 = communicator->stringToProxy("id -f \'facet x:tcp");
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }
    b1 = communicator->stringToProxy("test -f facet:tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet");
    b1 = communicator->stringToProxy("test -f \"facet:tcp\":tcp");
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet() == "facet:tcp");
#ifdef ICEE_HAS_LOCATOR
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
#endif
    b1 = communicator->stringToProxy("test:tcp");
    test(b1->ice_isTwoway());
    b1 = communicator->stringToProxy("test -t:tcp");
    test(b1->ice_isTwoway());
    b1 = communicator->stringToProxy("test -o:tcp:tcp");
    test(b1->ice_isOneway());
    b1 = communicator->stringToProxy("test -O:tcp");
    test(b1->ice_isBatchOneway());
    b1 = communicator->stringToProxy("test -d:tcp");
    test(b1->ice_isDatagram());
    b1 = communicator->stringToProxy("test -D:tcp");
    test(b1->ice_isBatchDatagram());
    b1 = communicator->stringToProxy("test:tcp");
    test(!b1->ice_isSecure());
    b1 = communicator->stringToProxy("test -s:tcp");
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
    tprintf("ok\n");

    tprintf("testing propertyToProxy... ");
    Ice::PropertiesPtr prop = communicator->getProperties();
    string propertyPrefix = "Foo.Proxy";
    prop->setProperty(propertyPrefix, "test:default -p 12010 -t 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getFacet().empty());

    string property;

    // These two properties don't do anything to direct proxies so
    // first we test that.
    /*
     * Commented out because setting a locator or locator cache
     * timeout on a direct proxy causes warning.
     *
#ifdef ICEE_HAS_LOCATOR
    string property = propertyPrefix + ".Locator";
    test(!b1->ice_getLocator());
    prop->setProperty(property, "locator:default -p 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(!b1->ice_getLocator());
    prop->setProperty(property, "");
#endif

    property = propertyPrefix + ".LocatorCacheTimeout";
    test(b1->ice_getLocatorCacheTimeout() == 0);
    prop->setProperty(property, "1");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocatorCacheTimeout() == 0);
    prop->setProperty(property, "");
    */

    // Now retest with an indirect proxy.
#ifdef ICEE_HAS_LOCATOR
    prop->setProperty(propertyPrefix, "test");
    property = propertyPrefix + ".Locator";
    prop->setProperty(property, "locator:default -p 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocator() && b1->ice_getLocator()->ice_getIdentity().name == "locator");
    prop->setProperty(property, "");
#endif

/*
    property = propertyPrefix + ".LocatorCacheTimeout";
    test(b1->ice_getLocatorCacheTimeout() == -1);
    prop->setProperty(property, "1");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocatorCacheTimeout() == 1);
    prop->setProperty(property, "");
    */

    // This cannot be tested so easily because the property is cached
    // on communicator initialization.
    //
    //prop->setProperty("Ice.Default.LocatorCacheTimeout", "60");
    //b1 = communicator->propertyToProxy(propertyPrefix);
    //test(b1->ice_getLocatorCacheTimeout() == 60);
    //prop->setProperty("Ice.Default.LocatorCacheTimeout", "");

    prop->setProperty(propertyPrefix, "test:default -p 12010 -t 10000");

#ifdef ICEE_HAS_ROUTER
    property = propertyPrefix + ".Router";
    test(!b1->ice_getRouter());
    prop->setProperty(property, "router:default -p 10000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getRouter() && b1->ice_getRouter()->ice_getIdentity().name == "router");
    prop->setProperty(property, "");
#endif

    /*
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
*/

    tprintf("ok\n");

    tprintf("testing ice_getCommunicator... ");
    test(base->ice_getCommunicator() == communicator);
    tprintf("ok\n");

    tprintf("testing proxy methods... ");
    test(communicator->identityToString(base->ice_identity(communicator->stringToIdentity("other"))->ice_getIdentity())
         == "other");
    test(base->ice_facet("facet")->ice_getFacet() == "facet");
#ifdef ICEE_HAS_LOCATOR
    test(base->ice_adapterId("id")->ice_getAdapterId() == "id");
#endif
    test(base->ice_twoway()->ice_isTwoway());
    test(base->ice_oneway()->ice_isOneway());
    test(base->ice_batchOneway()->ice_isBatchOneway());
    test(base->ice_datagram()->ice_isDatagram());
    test(base->ice_batchDatagram()->ice_isBatchDatagram());
    test(base->ice_secure(true)->ice_isSecure());
    test(!base->ice_secure(false)->ice_isSecure());
    //test(base->ice_collocationOptimized(true)->ice_isCollocationOptimized());
    //test(!base->ice_collocationOptimized(false)->ice_isCollocationOptimized());
    tprintf("ok\n");

    tprintf("testing proxy comparison... ");

    test(communicator->stringToProxy("foo:tcp") == communicator->stringToProxy("foo:tcp"));
    test(communicator->stringToProxy("foo:tcp") != communicator->stringToProxy("foo2:tcp"));
    test(communicator->stringToProxy("foo:tcp") < communicator->stringToProxy("foo2:tcp"));
    test(!(communicator->stringToProxy("foo2:tcp") < communicator->stringToProxy("foo:tcp")));

    Ice::ObjectPrx compObj = communicator->stringToProxy("foo:tcp");

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

/*
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
*/

    test(compObj->ice_timeout(20) == compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) != compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) < compObj->ice_timeout(20));
    test(!(compObj->ice_timeout(20) < compObj->ice_timeout(10)));

    Ice::ObjectPrx compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
    Ice::ObjectPrx compObj2 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));

#ifdef ICEE_HAS_LOCATOR
    compObj1 = communicator->stringToProxy("foo@MyAdapter1");
    compObj2 = communicator->stringToProxy("foo@MyAdapter2");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));
#endif

/*
    test(compObj1->ice_locatorCacheTimeout(20) == compObj1->ice_locatorCacheTimeout(20));
    test(compObj1->ice_locatorCacheTimeout(10) != compObj1->ice_locatorCacheTimeout(20));
    test(compObj1->ice_locatorCacheTimeout(10) < compObj1->ice_locatorCacheTimeout(20));
    test(!(compObj1->ice_locatorCacheTimeout(20) < compObj1->ice_locatorCacheTimeout(10)));
*/

#ifdef ICEE_HAS_LOCATOR
    compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
    compObj2 = communicator->stringToProxy("foo@MyAdapter1");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));
#endif

    //
    // TODO: Ideally we should also test comparison of fixed proxies.
    //

    tprintf("ok\n");

    tprintf("testing checked cast... ");
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
#ifdef ICEE_HAS_LOCATOR
    Ice::LocatorPrx loc = Ice::LocatorPrx::checkedCast(base);
    test(loc == 0);
#endif

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
    
#ifdef ICEE_HAS_LOCATOR
    loc = checkedCast<Ice::LocatorPrx>(base);
    test(loc == 0);
#endif

    cl2 = checkedCast<Test::MyClassPrx>(derived);
    obj = checkedCast<Ice::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    tprintf("ok\n");

    tprintf("testing checked cast with context... ");
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

    tprintf("ok\n");

    return cl;
}
