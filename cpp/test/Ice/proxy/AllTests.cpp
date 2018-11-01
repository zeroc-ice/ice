// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <Ice/Router.h>
#include <TestHelper.h>
#include <Test.h>

#ifdef _MSC_VER
#   pragma warning(disable:4125) // decimal digit terminates octal escape sequence
#endif

using namespace std;

Test::MyClassPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    const string protocol = communicator->getProperties()->getProperty("Ice.Default.Protocol");

    const string endp = helper->getTestEndpoint();
    cout << "testing stringToProxy... " << flush;

    string ref = "test:" + endp;
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);

    Ice::ObjectPrxPtr b1 = communicator->stringToProxy("test");
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

    // Test some octal corner cases.
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

    b1 = communicator->stringToProxy("");
    test(!b1);
    b1 = communicator->stringToProxy("\"\"");
    test(!b1);

    try
    {
        b1 = communicator->stringToProxy("\"\" test"); // Invalid trailing characters.
        test(false);
    }
    catch(const Ice::ProxyParseException&)
    {
    }

    try
    {
        b1 = communicator->stringToProxy("test:"); // Missing endpoint.
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

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

    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category == "category /test@foo"
         && b1->ice_getAdapterId() == "adapter");

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

    test(b1->ice_getEncodingVersion() == Ice::currentEncoding);

    b1 = communicator->stringToProxy("test -e 1.0");
    test(b1->ice_getEncodingVersion().major == 1 && b1->ice_getEncodingVersion().minor == 0);

    b1 = communicator->stringToProxy("test -e 6.5");
    test(b1->ice_getEncodingVersion().major == 6 && b1->ice_getEncodingVersion().minor == 5);

    b1 = communicator->stringToProxy("test -p 1.0 -e 1.0");
    test(b1->ice_toString() == "test -t -e 1.0");

    b1 = communicator->stringToProxy("test -p 6.5 -e 1.0");
    test(b1->ice_toString() == "test -t -p 6.5 -e 1.0");

    try
    {
        communicator->stringToProxy("test:tcp@adapterId");
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
        communicator->stringToProxy("test: :tcp");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    //
    // Test invalid endpoint syntax
    //
    try
    {
        communicator->createObjectAdapterWithEndpoints("BadAdapter", " : ");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        communicator->createObjectAdapterWithEndpoints("BadAdapter", "tcp: ");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        communicator->createObjectAdapterWithEndpoints("BadAdapter", ":tcp");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    //
    // Test for bug ICE-5543: escaped escapes in stringToIdentity
    //
    Ice::Identity id = { "test", ",X2QNUAzSBcJ_e$AV;E\\" };
    Ice::Identity id2 = Ice::stringToIdentity(communicator->identityToString(id));
    test(id == id2);

    id.name = "test";
    id.category = ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\";
    id2 = Ice::stringToIdentity(communicator->identityToString(id));
    test(id == id2);

    id.name = "/test";
    id.category = "cat/";
    string idStr = communicator->identityToString(id);
    test(idStr == "cat\\//\\/test");
    id2 = Ice::stringToIdentity(idStr);
    test(id == id2);

    // Input string with various pitfalls
    id = Ice::stringToIdentity("\\342\\x82\\254\\60\\x9\\60\\");
    test(id.name == "\xE2\x82\xAC\60\t0\\" && id.category.empty());

    try
    {
        // Illegal character < 32
        id = Ice::stringToIdentity("xx\01FooBar");
        test(false);
    }
    catch(const Ice::IdentityParseException&)
    {
    }

    try
    {
        // Illegal surrogate
        id = Ice::stringToIdentity("xx\\ud911");
        test(false);
    }
    catch(const Ice::IdentityParseException&)
    {
    }

    // Testing bytes 127 (\x7F) and €
    id.name = "test";
    id.category = "\x7F\xE2\x82\xAC";

    idStr = identityToString(id, Ice::ICE_ENUM(ToStringMode, Unicode));
    test(idStr == "\\u007f\xE2\x82\xAC/test");
    id2 = Ice::stringToIdentity(idStr);
    test(id == id2);
    test(Ice::identityToString(id) == idStr);

    idStr = identityToString(id, Ice::ICE_ENUM(ToStringMode, ASCII));
    test(idStr == "\\u007f\\u20ac/test");
    id2 = Ice::stringToIdentity(idStr);
    test(id == id2);

    idStr = identityToString(id, Ice::ICE_ENUM(ToStringMode, Compat));
    test(idStr == "\\177\\342\\202\\254/test");
    id2 = Ice::stringToIdentity(idStr);
    test(id == id2);

    id2 = Ice::stringToIdentity(communicator->identityToString(id));
    test(id == id2);

    // More unicode characters
#ifdef ICE_CPP11_MAPPING
    id.name = u8"banana \016-\U0001F34C\U000020AC\u00a2\u0024";
    id.category = u8"greek \U0001016A";

    idStr = identityToString(id, Ice::ICE_ENUM(ToStringMode, Unicode));
    test(idStr == u8"greek \U0001016A/banana \\u000e-\U0001F34C\U000020AC\u00a2$");
    id2 = Ice::stringToIdentity(idStr);
    test(id == id2);

    idStr = identityToString(id, Ice::ICE_ENUM(ToStringMode, ASCII));
    test(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$");
    id2 = Ice::stringToIdentity(idStr);
    test(id == id2);

    idStr = identityToString(id, Ice::ICE_ENUM(ToStringMode, Compat));
    test(idStr == "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$");
    id2 = Ice::stringToIdentity(idStr);
    test(id == id2);
#endif

    cout << "ok" << endl;

    cout << "testing proxyToString... " << flush;
    b1 = communicator->stringToProxy(ref);
    Ice::ObjectPrxPtr b2 = communicator->stringToProxy(communicator->proxyToString(b1));
    test(Ice::targetEqualTo(b1, b2));

    if(b1->ice_getConnection()) // not colloc-optimized target
    {
        b2 = b1->ice_getConnection()->createProxy(Ice::stringToIdentity("fixed"));
        string str = communicator->proxyToString(b2);
        test(b2->ice_toString() == str);
        string str2 = b1->ice_identity(b2->ice_getIdentity())->ice_secure(b2->ice_isSecure())->ice_toString();

        // Verify that the stringified fixed proxy is the same as a regular stringified proxy
        // but without endpoints
        test(str2.substr(0, str.size()) == str);
        test(str2[str.size()] == ':');
    }
    cout << "ok" << endl;

    cout << "testing propertyToProxy... " << flush;
    Ice::PropertiesPtr prop = communicator->getProperties();
    string propertyPrefix = "Foo.Proxy";
    prop->setProperty(propertyPrefix, "test:" + endp);
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getIdentity().name == "test" && b1->ice_getIdentity().category.empty() &&
         b1->ice_getAdapterId().empty() && b1->ice_getFacet().empty());

    string property;

    property = propertyPrefix + ".Locator";
    test(!b1->ice_getLocator());
    prop->setProperty(property, "locator:" + endp);
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocator() && b1->ice_getLocator()->ice_getIdentity().name == "locator");
    prop->setProperty(property, "");

    property = propertyPrefix + ".LocatorCacheTimeout";
    test(b1->ice_getLocatorCacheTimeout() == -1);
    prop->setProperty(property, "1");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getLocatorCacheTimeout() == 1);
    prop->setProperty(property, "");

    // Now retest with an indirect proxy.
    prop->setProperty(propertyPrefix, "test");
    property = propertyPrefix + ".Locator";
    prop->setProperty(property, "locator:" + endp);
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

    prop->setProperty(propertyPrefix, "test:" + endp);

    property = propertyPrefix + ".Router";
    test(!b1->ice_getRouter());
    prop->setProperty(property, "router:" + endp);
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

    property = propertyPrefix + ".InvocationTimeout";
    test(b1->ice_getInvocationTimeout() == -1);
    prop->setProperty(property, "1000");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getInvocationTimeout() == 1000);
    prop->setProperty(property, "");

    property = propertyPrefix + ".EndpointSelection";
    test(b1->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Random));
    prop->setProperty(property, "Random");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Random));
    prop->setProperty(property, "Ordered");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getEndpointSelection() == Ice::ICE_ENUM(EndpointSelectionType, Ordered));
    prop->setProperty(property, "");

    property = propertyPrefix + ".CollocationOptimized";
    test(b1->ice_isCollocationOptimized());
    prop->setProperty(property, "0");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(!b1->ice_isCollocationOptimized());
    prop->setProperty(property, "");

    property = propertyPrefix + ".Context.c1";
    test(b1->ice_getContext()["c1"].empty());
    prop->setProperty(property, "TEST");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getContext()["c1"] == "TEST");

    property = propertyPrefix + ".Context.c2";
    test(b1->ice_getContext()["c2"].empty());
    prop->setProperty(property, "TEST");
    b1 = communicator->propertyToProxy(propertyPrefix);
    test(b1->ice_getContext()["c2"] == "TEST");

    prop->setProperty(propertyPrefix + ".Context.c1", "");
    prop->setProperty(propertyPrefix + ".Context.c2", "");

    cout << "ok" << endl;

    cout << "testing proxyToProperty... " << flush;

    b1 = communicator->stringToProxy("test");
    b1 = b1->ice_collocationOptimized(true);
    b1 = b1->ice_connectionCached(true);
    b1 = b1->ice_preferSecure(false);
    b1 = b1->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered));
    b1 = b1->ice_locatorCacheTimeout(100);
    b1 = b1->ice_invocationTimeout(1234);
    Ice::EncodingVersion v = { 1, 0 };
    b1 = b1->ice_encodingVersion(v);
    Ice::ObjectPrxPtr router = communicator->stringToProxy("router");
    router = router->ice_collocationOptimized(false);
    router = router->ice_connectionCached(true);
    router = router->ice_preferSecure(true);
    router = router->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random));
    router = router->ice_locatorCacheTimeout(200);
    router = router->ice_invocationTimeout(1500);

    Ice::ObjectPrxPtr locator = communicator->stringToProxy("locator");
    locator = locator->ice_collocationOptimized(true);
    locator = locator->ice_connectionCached(false);
    locator = locator->ice_preferSecure(true);
    locator = locator->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random));
    locator = locator->ice_locatorCacheTimeout(300);
    locator = locator->ice_invocationTimeout(1500);

    locator = locator->ice_router(ICE_UNCHECKED_CAST(Ice::RouterPrx, router));
    b1 = b1->ice_locator(ICE_UNCHECKED_CAST(Ice::LocatorPrx, locator));

    Ice::PropertyDict proxyProps = communicator->proxyToProperty(b1, "Test");
    test(proxyProps.size() == 21);

    test(proxyProps["Test"] == "test -t -e 1.0");
    test(proxyProps["Test.CollocationOptimized"] == "1");
    test(proxyProps["Test.ConnectionCached"] == "1");
    test(proxyProps["Test.PreferSecure"] == "0");
    test(proxyProps["Test.EndpointSelection"] == "Ordered");
    test(proxyProps["Test.LocatorCacheTimeout"] == "100");
    test(proxyProps["Test.InvocationTimeout"] == "1234");

    test(proxyProps["Test.Locator"] == "locator -t -e " + Ice::encodingVersionToString(Ice::currentEncoding));
    // Locator collocation optimization is always disabled.
    //test(proxyProps["Test.Locator.CollocationOptimized"] == "1");
    test(proxyProps["Test.Locator.ConnectionCached"] == "0");
    test(proxyProps["Test.Locator.PreferSecure"] == "1");
    test(proxyProps["Test.Locator.EndpointSelection"] == "Random");
    test(proxyProps["Test.Locator.LocatorCacheTimeout"] == "300");
    test(proxyProps["Test.Locator.InvocationTimeout"] == "1500");

    test(proxyProps["Test.Locator.Router"] == "router -t -e " + Ice::encodingVersionToString(Ice::currentEncoding));
    test(proxyProps["Test.Locator.Router.CollocationOptimized"] == "0");
    test(proxyProps["Test.Locator.Router.ConnectionCached"] == "1");
    test(proxyProps["Test.Locator.Router.PreferSecure"] == "1");
    test(proxyProps["Test.Locator.Router.EndpointSelection"] == "Random");
    test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200");
    test(proxyProps["Test.Locator.Router.InvocationTimeout"] == "1500");

    cout << "ok" << endl;

    cout << "testing ice_getCommunicator... " << flush;
    test(base->ice_getCommunicator() == communicator);
    cout << "ok" << endl;

    cout << "testing proxy methods... " << flush;

    test(communicator->identityToString(base->ice_identity(Ice::stringToIdentity("other"))->ice_getIdentity())
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
    test(base->ice_preferSecure(true)->ice_isPreferSecure());
    test(!base->ice_preferSecure(false)->ice_isPreferSecure());
    test(base->ice_encodingVersion(Ice::Encoding_1_0)->ice_getEncodingVersion() == Ice::Encoding_1_0);
    test(base->ice_encodingVersion(Ice::Encoding_1_1)->ice_getEncodingVersion() == Ice::Encoding_1_1);
    test(base->ice_encodingVersion(Ice::Encoding_1_0)->ice_getEncodingVersion() != Ice::Encoding_1_1);

    try
    {
        base->ice_timeout(0);
        test(false);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
    }

    try
    {
        base->ice_timeout(-1);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
        test(false);
    }

    try
    {
        base->ice_timeout(-2);
        test(false);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
    }

    try
    {
        base->ice_invocationTimeout(0);
        test(false);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
    }

    try
    {
        base->ice_invocationTimeout(-1);
        base->ice_invocationTimeout(-2);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
        test(false);
    }

    try
    {
        base->ice_invocationTimeout(-3);
        test(false);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
    }

    try
    {
        base->ice_locatorCacheTimeout(0);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
        test(false);
    }

    try
    {
        base->ice_locatorCacheTimeout(-1);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
        test(false);
    }

    try
    {
        base->ice_locatorCacheTimeout(-2);
        test(false);
    }
#ifdef ICE_CPP11_MAPPING
    catch(const invalid_argument&)
#else
    catch(const IceUtil::IllegalArgumentException&)
#endif
    {
    }

    cout << "ok" << endl;

    cout << "testing proxy comparison... " << flush;

#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(communicator->stringToProxy("foo"), communicator->stringToProxy("foo")));
    test(Ice::targetNotEqualTo(communicator->stringToProxy("foo"), communicator->stringToProxy("foo2")));
    test(Ice::targetLess(communicator->stringToProxy("foo"), communicator->stringToProxy("foo2")));
    test(Ice::targetGreaterEqual(communicator->stringToProxy("foo2"), communicator->stringToProxy("foo")));

    Ice::ObjectPrxPtr compObj = communicator->stringToProxy("foo");

    test(Ice::targetEqualTo(compObj->ice_facet("facet"), compObj->ice_facet("facet")));
    test(Ice::targetNotEqualTo(compObj->ice_facet("facet"), compObj->ice_facet("facet1")));
    test(Ice::targetLess(compObj->ice_facet("facet"), compObj->ice_facet("facet1")));
    test(Ice::targetGreaterEqual(compObj->ice_facet("facet"), compObj->ice_facet("facet")));

    test(Ice::targetEqualTo(compObj->ice_oneway(), compObj->ice_oneway()));
    test(Ice::targetNotEqualTo(compObj->ice_oneway(), compObj->ice_twoway()));
    test(Ice::targetLess(compObj->ice_twoway(), compObj->ice_oneway()));
    test(Ice::targetGreaterEqual(compObj->ice_oneway(), compObj->ice_twoway()));

    test(Ice::targetEqualTo(compObj->ice_secure(true), compObj->ice_secure(true)));
    test(Ice::targetNotEqualTo(compObj->ice_secure(false), compObj->ice_secure(true)));
    test(Ice::targetLess(compObj->ice_secure(false), compObj->ice_secure(true)));
    test(Ice::targetGreaterEqual(compObj->ice_secure(true), compObj->ice_secure(false)));

    test(Ice::targetEqualTo(compObj->ice_collocationOptimized(true), compObj->ice_collocationOptimized(true)));
    test(Ice::targetNotEqualTo(compObj->ice_collocationOptimized(false), compObj->ice_collocationOptimized(true)));
    test(Ice::targetLess(compObj->ice_collocationOptimized(false), compObj->ice_collocationOptimized(true)));
    test(Ice::targetGreaterEqual(compObj->ice_collocationOptimized(true), compObj->ice_collocationOptimized(false)));

    test(Ice::targetEqualTo(compObj->ice_connectionCached(true), compObj->ice_connectionCached(true)));
    test(Ice::targetNotEqualTo(compObj->ice_connectionCached(false), compObj->ice_connectionCached(true)));
    test(Ice::targetLess(compObj->ice_connectionCached(false), compObj->ice_connectionCached(true)));
    test(Ice::targetGreaterEqual(compObj->ice_connectionCached(true), compObj->ice_connectionCached(false)));

    test(Ice::targetEqualTo(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)), compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random))));
    test(Ice::targetNotEqualTo(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)), compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered))));
    test(Ice::targetLess(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)), compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered))));
    test(Ice::targetGreaterEqual(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered)), compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random))));

    test(Ice::targetEqualTo(compObj->ice_connectionId("id2"), compObj->ice_connectionId("id2")));
    test(Ice::targetNotEqualTo(compObj->ice_connectionId("id1"), compObj->ice_connectionId("id2")));
    test(Ice::targetLess(compObj->ice_connectionId("id1"), compObj->ice_connectionId("id2")));
    test(Ice::targetGreaterEqual(compObj->ice_connectionId("id2"), compObj->ice_connectionId("id1")));

    test(compObj->ice_connectionId("id1")->ice_getConnectionId() == "id1");
    test(compObj->ice_connectionId("id2")->ice_getConnectionId() == "id2");

    test(Ice::targetEqualTo(compObj->ice_compress(true), compObj->ice_compress(true)));
    test(Ice::targetNotEqualTo(compObj->ice_compress(false), compObj->ice_compress(true)));
    test(Ice::targetLess(compObj->ice_compress(false), compObj->ice_compress(true)));
    test(Ice::targetGreaterEqual(compObj->ice_compress(true), compObj->ice_compress(false)));

    test(compObj->ice_getCompress() == Ice::nullopt);
    test(compObj->ice_compress(true)->ice_getCompress() == Ice::optional<bool>(true));
    test(compObj->ice_compress(false)->ice_getCompress() == Ice::optional<bool>(false));

    test(Ice::targetEqualTo(compObj->ice_timeout(20), compObj->ice_timeout(20)));
    test(Ice::targetNotEqualTo(compObj->ice_timeout(10), compObj->ice_timeout(20)));
    test(Ice::targetLess(compObj->ice_timeout(10), compObj->ice_timeout(20)));
    test(Ice::targetGreaterEqual(compObj->ice_timeout(20), compObj->ice_timeout(10)));

    test(compObj->ice_getTimeout() == Ice::nullopt);
    test(compObj->ice_timeout(10)->ice_getTimeout() == Ice::optional<int>(10));
    test(compObj->ice_timeout(20)->ice_getTimeout() == Ice::optional<int>(20));

    auto loc1 = Ice::uncheckedCast<Ice::LocatorPrx>(communicator->stringToProxy("loc1:default -p 10000"));
    auto loc2 = Ice::uncheckedCast<Ice::LocatorPrx>(communicator->stringToProxy("loc2:default -p 10000"));

    test(Ice::targetEqualTo(compObj->ice_locator(0), compObj->ice_locator(0)));
    test(Ice::targetEqualTo(compObj->ice_locator(loc1), compObj->ice_locator(loc1)));
    test(Ice::targetNotEqualTo(compObj->ice_locator(loc1), compObj->ice_locator(0)));
    test(Ice::targetNotEqualTo(compObj->ice_locator(0), compObj->ice_locator(loc2)));
    test(Ice::targetNotEqualTo(compObj->ice_locator(loc1), compObj->ice_locator(loc2)));
    test(Ice::targetLess(compObj->ice_locator(0), compObj->ice_locator(loc1)));
    test(Ice::targetGreaterEqual(compObj->ice_locator(loc1), compObj->ice_locator(0)));
    test(Ice::targetLess(compObj->ice_locator(loc1), compObj->ice_locator(loc2)));
    test(Ice::targetGreaterEqual(compObj->ice_locator(loc2), compObj->ice_locator(loc1)));

    auto rtr1 = Ice::uncheckedCast<Ice::RouterPrx>(communicator->stringToProxy("rtr1:default -p 10000"));
    auto rtr2 = Ice::uncheckedCast<Ice::RouterPrx>(communicator->stringToProxy("rtr2:default -p 10000"));

    test(Ice::targetEqualTo(compObj->ice_router(0), compObj->ice_router(0)));
    test(Ice::targetEqualTo(compObj->ice_router(rtr1), compObj->ice_router(rtr1)));
    test(Ice::targetNotEqualTo(compObj->ice_router(rtr1), compObj->ice_router(0)));
    test(Ice::targetNotEqualTo(compObj->ice_router(0), compObj->ice_router(rtr2)));
    test(Ice::targetNotEqualTo(compObj->ice_router(rtr1), compObj->ice_router(rtr2)));
    test(Ice::targetLess(compObj->ice_router(0), compObj->ice_router(rtr1)));
    test(Ice::targetGreaterEqual(compObj->ice_router(rtr1), compObj->ice_router(0)));
    test(Ice::targetLess(compObj->ice_router(rtr1), compObj->ice_router(rtr2)));
    test(Ice::targetGreaterEqual(compObj->ice_router(rtr2), compObj->ice_router(rtr1)));

    Ice::Context ctx1;
    ctx1["ctx1"] = "v1";
    Ice::Context ctx2;
    ctx2["ctx2"] = "v2";
    test(Ice::targetEqualTo(compObj->ice_context(Ice::Context()), compObj->ice_context(Ice::Context())));
    test(Ice::targetEqualTo(compObj->ice_context(ctx1), compObj->ice_context(ctx1)));
    test(Ice::targetNotEqualTo(compObj->ice_context(ctx1), compObj->ice_context(Ice::Context())));
    test(Ice::targetNotEqualTo(compObj->ice_context(Ice::Context()), compObj->ice_context(ctx2)));
    test(Ice::targetNotEqualTo(compObj->ice_context(ctx1), compObj->ice_context(ctx2)));
    test(Ice::targetLess(compObj->ice_context(ctx1), compObj->ice_context(ctx2)));
    test(Ice::targetGreaterEqual(compObj->ice_context(ctx2), compObj->ice_context(ctx1)));

    test(Ice::targetEqualTo(compObj->ice_preferSecure(true), compObj->ice_preferSecure(true)));
    test(Ice::targetNotEqualTo(compObj->ice_preferSecure(true), compObj->ice_preferSecure(false)));
    test(Ice::targetLess(compObj->ice_preferSecure(false), compObj->ice_preferSecure(true)));
    test(Ice::targetGreaterEqual(compObj->ice_preferSecure(true), compObj->ice_preferSecure(false)));

    auto compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
    auto compObj2 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
    test(Ice::targetNotEqualTo(compObj1, compObj2));
    test(Ice::targetLess(compObj1, compObj2));
    test(Ice::targetGreaterEqual(compObj2, compObj1));

    compObj1 = communicator->stringToProxy("foo@MyAdapter1");
    compObj2 = communicator->stringToProxy("foo@MyAdapter2");
    test(Ice::targetNotEqualTo(compObj1, compObj2));
    test(Ice::targetLess(compObj1, compObj2));
    test(Ice::targetGreaterEqual(compObj2, compObj1));

    test(Ice::targetEqualTo(compObj1->ice_locatorCacheTimeout(20), compObj1->ice_locatorCacheTimeout(20)));
    test(Ice::targetNotEqualTo(compObj1->ice_locatorCacheTimeout(10), compObj1->ice_locatorCacheTimeout(20)));
    test(Ice::targetLess(compObj1->ice_locatorCacheTimeout(10), compObj1->ice_locatorCacheTimeout(20)));
    test(Ice::targetGreaterEqual(compObj1->ice_locatorCacheTimeout(20), compObj1->ice_locatorCacheTimeout(10)));

    test(Ice::targetEqualTo(compObj1->ice_invocationTimeout(20), compObj1->ice_invocationTimeout(20)));
    test(Ice::targetNotEqualTo(compObj1->ice_invocationTimeout(10), compObj1->ice_invocationTimeout(20)));
    test(Ice::targetLess(compObj1->ice_invocationTimeout(10), compObj1->ice_invocationTimeout(20)));
    test(Ice::targetGreaterEqual(compObj1->ice_invocationTimeout(20), compObj1->ice_invocationTimeout(10)));

    compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
    compObj2 = communicator->stringToProxy("foo@MyAdapter1");
    test(Ice::targetNotEqualTo(compObj1, compObj2));
    test(Ice::targetLess(compObj1, compObj2));
    test(Ice::targetGreaterEqual(compObj2, compObj1));

    Ice::EndpointSeq endpts1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")->ice_getEndpoints();
    Ice::EndpointSeq endpts2 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10001")->ice_getEndpoints();

    test(endpts1.size() != endpts2.size() || !equal(endpts1.begin(), endpts1.end(), endpts2.begin(), Ice::TargetCompare<shared_ptr<Ice::Endpoint>, std::equal_to>()));
    test(lexicographical_compare(endpts1.begin(), endpts1.end(), endpts2.begin(), endpts2.end(), Ice::TargetCompare<shared_ptr<Ice::Endpoint>, std::less>()));
    test(!lexicographical_compare(endpts2.begin(), endpts2.end(), endpts1.begin(), endpts1.end(), Ice::TargetCompare<shared_ptr<Ice::Endpoint>, std::less>()));

    Ice::EndpointSeq endpts3 =  communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")->ice_getEndpoints();
    test(endpts1.size() == endpts3.size() && equal(endpts1.begin(), endpts1.end(), endpts3.begin(), Ice::TargetCompare<shared_ptr<Ice::Endpoint>, std::equal_to>()));

    test(Ice::targetEqualTo(compObj1->ice_encodingVersion(Ice::Encoding_1_0), compObj1->ice_encodingVersion(Ice::Encoding_1_0)));
    test(Ice::targetNotEqualTo(compObj1->ice_encodingVersion(Ice::Encoding_1_0), compObj1->ice_encodingVersion(Ice::Encoding_1_1)));
    test(Ice::targetLess(compObj->ice_encodingVersion(Ice::Encoding_1_0), compObj->ice_encodingVersion(Ice::Encoding_1_1)));
    test(Ice::targetGreaterEqual(compObj->ice_encodingVersion(Ice::Encoding_1_1), compObj->ice_encodingVersion(Ice::Encoding_1_0)));

    Ice::ConnectionPtr baseConnection = base->ice_getConnection();
    if(baseConnection && protocol != "bt")
    {
        Ice::ConnectionPtr baseConnection2 = base->ice_connectionId("base2")->ice_getConnection();
        compObj1 = compObj1->ice_fixed(baseConnection);
        compObj2 = compObj2->ice_fixed(baseConnection2);
        test(Ice::targetNotEqualTo(compObj1, compObj2));
        if(Ice::targetLess(compObj1, compObj2))
        {
            test(Ice::targetGreaterEqual(compObj2, compObj1));
        }
        else
        {
            test(Ice::targetGreaterEqual(compObj1, compObj2));
        }
    }

    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    auto cl = Ice::checkedCast<Test::MyClassPrx>(base);
    test(cl);

    auto derived = Ice::checkedCast<Test::MyDerivedClassPrx>(cl);
    test(derived);
    test(Ice::targetEqualTo(cl, base));
    test(Ice::targetEqualTo(derived, base));
    test(Ice::targetEqualTo(cl, derived));

    auto loc = Ice::checkedCast<Ice::LocatorPrx>(base);
    test(loc == nullptr);

    //
    // Upcasting
    //
    auto cl2 = Ice::checkedCast<Test::MyClassPrx>(derived);
    auto obj = Ice::checkedCast<Ice::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(Ice::targetEqualTo(cl2, obj));
    test(Ice::targetEqualTo(cl2, derived));
#else
    test(communicator->stringToProxy("foo") == communicator->stringToProxy("foo"));
    test(communicator->stringToProxy("foo") != communicator->stringToProxy("foo2"));
    test(communicator->stringToProxy("foo") < communicator->stringToProxy("foo2"));
    test(!(communicator->stringToProxy("foo2") < communicator->stringToProxy("foo")));

    Ice::ObjectPrxPtr compObj = communicator->stringToProxy("foo");

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

    test(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)) == compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)));
    test(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)) != compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered)));
    test(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random)) < compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered)));
    test(!(compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Ordered)) < compObj->ice_endpointSelection(Ice::ICE_ENUM(EndpointSelectionType, Random))));

    test(compObj->ice_connectionId("id2") == compObj->ice_connectionId("id2"));
    test(compObj->ice_connectionId("id1") != compObj->ice_connectionId("id2"));
    test(compObj->ice_connectionId("id1") < compObj->ice_connectionId("id2"));
    test(!(compObj->ice_connectionId("id2") < compObj->ice_connectionId("id1")));

    test(compObj->ice_connectionId("id1")->ice_getConnectionId() == "id1");
    test(compObj->ice_connectionId("id2")->ice_getConnectionId() == "id2");

    test(compObj->ice_compress(true) == compObj->ice_compress(true));
    test(compObj->ice_compress(false) != compObj->ice_compress(true));
    test(compObj->ice_compress(false) < compObj->ice_compress(true));
    test(!(compObj->ice_compress(true) < compObj->ice_compress(false)));

    test(!compObj->ice_getCompress());
    test(compObj->ice_compress(true)->ice_getCompress() == IceUtil::Optional<bool>(true));
    test(compObj->ice_compress(false)->ice_getCompress() == IceUtil::Optional<bool>(false));

    test(compObj->ice_timeout(20) == compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) != compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) < compObj->ice_timeout(20));
    test(!(compObj->ice_timeout(20) < compObj->ice_timeout(10)));

    test(!compObj->ice_getTimeout());
    test(compObj->ice_timeout(10)->ice_getTimeout() == IceUtil::Optional<int>(10));
    test(compObj->ice_timeout(20)->ice_getTimeout() == IceUtil::Optional<int>(20));

    Ice::LocatorPrxPtr loc1 = ICE_UNCHECKED_CAST(Ice::LocatorPrx, communicator->stringToProxy("loc1:" + endp));
    Ice::LocatorPrxPtr loc2 = ICE_UNCHECKED_CAST(Ice::LocatorPrx, communicator->stringToProxy("loc2:" + endp));
    test(compObj->ice_locator(0) == compObj->ice_locator(0));
    test(compObj->ice_locator(loc1) == compObj->ice_locator(loc1));
    test(compObj->ice_locator(loc1) != compObj->ice_locator(0));
    test(compObj->ice_locator(0) != compObj->ice_locator(loc2));
    test(compObj->ice_locator(loc1) != compObj->ice_locator(loc2));
    test(compObj->ice_locator(0) < compObj->ice_locator(loc1));
    test(!(compObj->ice_locator(loc1) < compObj->ice_locator(0)));
    test(compObj->ice_locator(loc1) < compObj->ice_locator(loc2));
    test(!(compObj->ice_locator(loc2) < compObj->ice_locator(loc1)));

    Ice::RouterPrxPtr rtr1 = ICE_UNCHECKED_CAST(Ice::RouterPrx, communicator->stringToProxy("rtr1:" + endp));
    Ice::RouterPrxPtr rtr2 = ICE_UNCHECKED_CAST(Ice::RouterPrx, communicator->stringToProxy("rtr2:" + endp));
    test(compObj->ice_router(0) == compObj->ice_router(0));
    test(compObj->ice_router(rtr1) == compObj->ice_router(rtr1));
    test(compObj->ice_router(rtr1) != compObj->ice_router(0));
    test(compObj->ice_router(0) != compObj->ice_router(rtr2));
    test(compObj->ice_router(rtr1) != compObj->ice_router(rtr2));
    test(compObj->ice_router(0) < compObj->ice_router(rtr1));
    test(!(compObj->ice_router(rtr1) < compObj->ice_router(0)));
    test(compObj->ice_router(rtr1) < compObj->ice_router(rtr2));
    test(!(compObj->ice_router(rtr2) < compObj->ice_router(rtr1)));

    Ice::Context ctx1;
    ctx1["ctx1"] = "v1";
    Ice::Context ctx2;
    ctx2["ctx2"] = "v2";
    test(compObj->ice_context(Ice::Context()) == compObj->ice_context(Ice::Context()));
    test(compObj->ice_context(ctx1) == compObj->ice_context(ctx1));
    test(compObj->ice_context(ctx1) != compObj->ice_context(Ice::Context()));
    test(compObj->ice_context(Ice::Context()) != compObj->ice_context(ctx2));
    test(compObj->ice_context(ctx1) != compObj->ice_context(ctx2));
    test(compObj->ice_context(ctx1) < compObj->ice_context(ctx2));
    test(!(compObj->ice_context(ctx2) < compObj->ice_context(ctx1)));

    test(compObj->ice_preferSecure(true) == compObj->ice_preferSecure(true));
    test(compObj->ice_preferSecure(true) != compObj->ice_preferSecure(false));
    test(compObj->ice_preferSecure(false) < compObj->ice_preferSecure(true));
    test(!(compObj->ice_preferSecure(true) < compObj->ice_preferSecure(false)));

    Ice::ObjectPrxPtr compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
    Ice::ObjectPrxPtr compObj2 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
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

    test(compObj1->ice_invocationTimeout(20) == compObj1->ice_invocationTimeout(20));
    test(compObj1->ice_invocationTimeout(10) != compObj1->ice_invocationTimeout(20));
    test(compObj1->ice_invocationTimeout(10) < compObj1->ice_invocationTimeout(20));
    test(!(compObj1->ice_invocationTimeout(20) < compObj1->ice_invocationTimeout(10)));

    compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
    compObj2 = communicator->stringToProxy("foo@MyAdapter1");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));

    Ice::EndpointSeq endpts1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")->ice_getEndpoints();
    Ice::EndpointSeq endpts2 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10001")->ice_getEndpoints();
    test(endpts1 != endpts2);
    test(endpts1 < endpts2);
    test(!(endpts2 < endpts1));
    test(endpts1 == communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")->ice_getEndpoints());

    test(compObj1->ice_encodingVersion(Ice::Encoding_1_0) == compObj1->ice_encodingVersion(Ice::Encoding_1_0));
    test(compObj1->ice_encodingVersion(Ice::Encoding_1_0) != compObj1->ice_encodingVersion(Ice::Encoding_1_1));
    test(compObj->ice_encodingVersion(Ice::Encoding_1_0) < compObj->ice_encodingVersion(Ice::Encoding_1_1));
    test(!(compObj->ice_encodingVersion(Ice::Encoding_1_1) < compObj->ice_encodingVersion(Ice::Encoding_1_0)));

    Ice::ConnectionPtr baseConnection = base->ice_getConnection();
    if(baseConnection && protocol != "bt")
    {
        Ice::ConnectionPtr baseConnection2 = base->ice_connectionId("base2")->ice_getConnection();
        compObj1 = compObj1->ice_fixed(baseConnection);
        compObj2 = compObj2->ice_fixed(baseConnection2);
        test(compObj1 != compObj2);
        if(compObj1 < compObj2)
        {
            test(compObj2 >= compObj1);
        }
        else
        {
            test(compObj1 >= compObj2);
        }
    }

    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    Test::MyClassPrxPtr cl = ICE_CHECKED_CAST(Test::MyClassPrx, base);
    test(cl);

    Test::MyDerivedClassPrxPtr derived = ICE_CHECKED_CAST(Test::MyDerivedClassPrx, cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);

    Ice::LocatorPrxPtr loc = ICE_CHECKED_CAST(Ice::LocatorPrx, base);
    test(loc == 0);

    //
    // Upcasting
    //
    Test::MyClassPrxPtr cl2 = ICE_CHECKED_CAST(Test::MyClassPrx, derived);
    Ice::ObjectPrxPtr obj = ICE_CHECKED_CAST(Ice::ObjectPrx, derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    //
    // Now with alternate API
    //
    cl = Ice::checkedCast<Test::MyClassPrx>(base);
    test(cl);
    derived = Ice::checkedCast<Test::MyDerivedClassPrx>(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);

    loc = Ice::checkedCast<Ice::LocatorPrx>(base);
    test(loc == 0);

    cl2 = Ice::checkedCast<Test::MyClassPrx>(derived);
    obj = Ice::checkedCast<Ice::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);
#endif
    cout << "ok" << endl;

    cout << "testing checked cast with context... " << flush;
    Ice::Context ctx = cl->getContext();
    test(ctx.size() == 0);

    ctx["one"] = "hello";
    ctx["two"] = "world";
#ifdef ICE_CPP11_MAPPING
    cl = Ice::checkedCast<Test::MyClassPrx>(base, ctx);
#else
    cl = Test::MyClassPrx::checkedCast(base, ctx);
#endif
    Ice::Context c2 = cl->getContext();
    test(ctx == c2);

    //
    // Now with alternate API
    //
#ifndef ICE_CPP11_MAPPING
    cl = Ice::checkedCast<Test::MyClassPrx>(base);
    ctx = cl->getContext();
    test(ctx.size() == 0);

    cl = Ice::checkedCast<Test::MyClassPrx>(base, ctx);
    c2 = cl->getContext();
    test(ctx == c2);
#endif
    cout << "ok" << endl;

    if(protocol != "bt")
    {
        cout << "testing ice_fixed... " << flush;
        {
            Ice::ConnectionPtr connection = cl->ice_getConnection();
            if(connection)
            {
                Test::MyClassPrxPtr prx = cl->ice_fixed(connection); // Test factory method return type
                prx->ice_ping();
                test(cl->ice_secure(true)->ice_fixed(connection)->ice_isSecure());
                test(cl->ice_facet("facet")->ice_fixed(connection)->ice_getFacet() == "facet");
                test(cl->ice_oneway()->ice_fixed(connection)->ice_isOneway());
                ctx.clear();
                ctx["one"] = "hello";
                ctx["two"] = "world";
                test(cl->ice_fixed(connection)->ice_getContext().empty());
                test(cl->ice_context(ctx)->ice_fixed(connection)->ice_getContext().size() == 2);
                test(cl->ice_fixed(connection)->ice_getInvocationTimeout() == -1);
                test(cl->ice_invocationTimeout(10)->ice_fixed(connection)->ice_getInvocationTimeout() == 10);
                test(cl->ice_fixed(connection)->ice_getConnection() == connection);
                test(cl->ice_fixed(connection)->ice_fixed(connection)->ice_getConnection() == connection);
                test(*cl->ice_compress(true)->ice_fixed(connection)->ice_getCompress());
                test(!cl->ice_fixed(connection)->ice_getTimeout());
                Ice::ConnectionPtr fixedConnection = cl->ice_connectionId("ice_fixed")->ice_getConnection();
                test(cl->ice_fixed(connection)->ice_fixed(fixedConnection)->ice_getConnection() == fixedConnection);
                try
                {
                    cl->ice_secure(!connection->getEndpoint()->getInfo()->secure())->ice_fixed(connection)->ice_ping();
                }
                catch(const Ice::NoEndpointException&)
                {
                }
                try
                {
                    cl->ice_datagram()->ice_fixed(connection)->ice_ping();
                }
                catch(const Ice::NoEndpointException&)
                {
                }
            }
            else
            {
                try
                {
                    cl->ice_fixed(connection);
                    test(false);
                }
#ifdef ICE_CPP11_MAPPING
                catch(const invalid_argument&)
#else
                catch(const IceUtil::IllegalArgumentException&)
#endif
                {
                    // Expected with null connection.
                }

            }
        }
        cout << "ok" << endl;
    }

    cout << "testing encoding versioning... " << flush;
    string ref20 = "test -e 2.0:" + endp;
    Test::MyClassPrxPtr cl20 = ICE_UNCHECKED_CAST(Test::MyClassPrx, communicator->stringToProxy(ref20));
    try
    {
        cl20->ice_ping();
        test(false);
    }
    catch(const Ice::UnsupportedEncodingException&)
    {
        // Server 2.0 endpoint doesn't support 1.1 version.
    }

    string ref10 = "test -e 1.0:" + endp;
    Test::MyClassPrxPtr cl10 = ICE_UNCHECKED_CAST(Test::MyClassPrx, communicator->stringToProxy(ref10));
    cl10->ice_ping();
    cl10->ice_encodingVersion(Ice::Encoding_1_0)->ice_ping();
    cl->ice_encodingVersion(Ice::Encoding_1_0)->ice_ping();

    // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
    // call will use the 1.1 encoding
    string ref13 = "test -e 1.3:" + endp;
    Test::MyClassPrxPtr cl13 = ICE_UNCHECKED_CAST(Test::MyClassPrx, communicator->stringToProxy(ref13));
    cl13->ice_ping();
#ifdef ICE_CPP11_MAPPING
    cl13->ice_pingAsync().get();
#else
    cl13->end_ice_ping(cl13->begin_ice_ping());
#endif

    try
    {
        // Send request with bogus 1.2 encoding.
        Ice::EncodingVersion version = { 1, 2 };
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.endEncapsulation();
        vector<Ice::Byte> inEncaps;
        out.finished(inEncaps);
        inEncaps[4] = version.major;
        inEncaps[5] = version.minor;
        vector<Ice::Byte> outEncaps;
        cl->ice_invoke("ice_ping", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        test(false);
    }
    catch(const Ice::UnknownLocalException& ex)
    {
        // The server thrown an UnsupportedEncodingException
        test(ex.unknown.find("UnsupportedEncodingException") != string::npos);
    }

    try
    {
        // Send request with bogus 2.0 encoding.
        Ice::EncodingVersion version = { 2, 0 };
        Ice::OutputStream out(communicator);
        out.startEncapsulation();
        out.endEncapsulation();
        vector<Ice::Byte> inEncaps;
        out.finished(inEncaps);
        inEncaps[4] = version.major;
        inEncaps[5] = version.minor;
        vector<Ice::Byte> outEncaps;
        cl->ice_invoke("ice_ping", Ice::ICE_ENUM(OperationMode, Normal), inEncaps, outEncaps);
        test(false);
    }
    catch(const Ice::UnknownLocalException& ex)
    {
        // The server thrown an UnsupportedEncodingException
        test(ex.unknown.find("UnsupportedEncodingException") != string::npos);
    }

    cout << "ok" << endl;

    cout << "testing protocol versioning... " << flush;

    ref20 = "test -p 2.0:" + endp;
    cl20 = ICE_UNCHECKED_CAST(Test::MyClassPrx, communicator->stringToProxy(ref20));
    try
    {
        cl20->ice_ping();
        test(false);
    }
    catch(const Ice::UnsupportedProtocolException&)
    {
        // Server 2.0 proxy doesn't support 1.0 version.
    }

    ref10 = "test -p 1.0:" + endp;
    cl10 = ICE_UNCHECKED_CAST(Test::MyClassPrx, communicator->stringToProxy(ref10));
    cl10->ice_ping();

    // 1.3 isn't supported but since a 1.3 proxy supports 1.0, the
    // call will use the 1.0 encoding
    ref13 = "test -p 1.3:" + endp;
    cl13 = ICE_UNCHECKED_CAST(Test::MyClassPrx, communicator->stringToProxy(ref13));
    cl13->ice_ping();
#ifdef ICE_CPP11_MAPPING
    cl13->ice_pingAsync().get();
#else
    cl13->end_ice_ping(cl13->begin_ice_ping());
#endif
    cout << "ok" <<endl;

    cout << "testing opaque endpoints... " << flush;

    try
    {
        // Invalid -x option
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t 99 -v abc -x abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing -t and -v
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Repeated -t
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t 1 -t 1 -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Repeated -v
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t 1 -v abc -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing -t
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing -v
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t 1");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing arg for -t
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Missing arg for -v
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t 1 -v");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Not a number for -t
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t x -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // < 0 for -t
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t -1 -v abc");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    try
    {
        // Invalid char for -v
        Ice::ObjectPrxPtr p = communicator->stringToProxy("id:opaque -t 99 -v x?c");
        test(false);
    }
    catch(const Ice::EndpointParseException&)
    {
    }

    // Legal TCP endpoint expressed as opaque endpoint
    Ice::ObjectPrxPtr p1 = communicator->stringToProxy("test -e 1.1:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    string pstr = communicator->proxyToString(p1);
    test(pstr == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

    // Opaque endpoint encoded with 1.1 encoding.
    {
        Ice::ObjectPrxPtr p2 = communicator->stringToProxy("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
        test(communicator->proxyToString(p2) == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");
    }

    if(communicator->getProperties()->getPropertyAsInt("Ice.IPv6") == 0 &&
       communicator->getProperties()->getProperty("Ice.Default.Host") == "127.0.0.1")
    {
        // SSL enabled?
        bool ssl;
        try
        {
            communicator->stringToProxy("dummy:ssl");
            ssl = true;
        }
        catch(const Ice::EndpointParseException&)
        {
            ssl = false;
        }

        const bool tcp = communicator->getProperties()->getProperty("Ice.Default.Protocol") == "tcp";

        // Two legal TCP endpoints expressed as opaque endpoints
        p1 = communicator->stringToProxy("test -e 1.0:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
        pstr = communicator->proxyToString(p1);
        test(pstr == "test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

        //
        // Test that an SSL endpoint and a nonsense endpoint get written
        // back out as an opaque endpoint.
        //
        p1 = communicator->stringToProxy(
                "test -e 1.0:opaque -e 1.0 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -e 1.0 -t 99 -v abch");
        pstr = communicator->proxyToString(p1);
        if(ssl)
        {
            test(pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");
        }
        else if(tcp)
        {
            test(pstr ==
                 "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch");
        }

        //
        // Test that the proxy with an SSL endpoint and a nonsense
        // endpoint (which the server doesn't understand either) can be
        // sent over the wire and returned by the server without losing
        // the opaque endpoints.
        //
        Ice::ObjectPrxPtr p2 = derived->echo(p1);
        pstr = communicator->proxyToString(p2);
        if(ssl)
        {
            test(pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");
        }
        else if(tcp)
        {
            if(pstr != "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch")
            {
                cerr << pstr << endl;
            }
            test(pstr ==
                 "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch");
        }
    }

    cout << "ok" << endl;

    cout << "testing communicator shutdown/destroy... " << flush;
    {
        Ice::CommunicatorPtr c = Ice::initialize();
        c->shutdown();
        test(c->isShutdown());
        c->waitForShutdown();
        c->destroy();
        c->shutdown();
        test(c->isShutdown());
        c->waitForShutdown();
        c->destroy();
    }
    cout << "ok" << endl;

    return cl;
}
