<?
// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}
Ice_loadProfileWithArgs($argv);

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        die("\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n");
    }
}

function allTests()
{
    global $ICE;

    echo "testing stringToProxy... ";
    flush();
    $ref = "test:default -p 12010 -t 2000";
    $base = $ICE->stringToProxy($ref);
    test($base != null);

    $b1 = $ICE->stringToProxy("test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getAdapterId() == "" && $b1->ice_getFacet() == "");
    $b1 = $ICE->stringToProxy("test ");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "");
    $b1 = $ICE->stringToProxy(" test ");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "");
    $b1 = $ICE->stringToProxy(" test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "");
    $b1 = $ICE->stringToProxy("'test -f facet'");
    test($b1->ice_getIdentity()->name == "test -f facet" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "");
    try
    {
        $b1 = $ICE->stringToProxy("\"test -f facet'");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_ProxyParseException $ex)
    {
    }
    $b1 = $ICE->stringToProxy("\"test -f facet\"");
    test($b1->ice_getIdentity()->name == "test -f facet" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "");
    $b1 = $ICE->stringToProxy("\"test -f facet@test\"");
    test($b1->ice_getIdentity()->name == "test -f facet@test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "");
    $b1 = $ICE->stringToProxy("\"test -f facet@test @test\"");
    test($b1->ice_getIdentity()->name == "test -f facet@test @test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "");
    try
    {
        $b1 = $ICE->stringToProxy("test test");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_ProxyParseException $ex)
    {
    }
    $b1 = $ICE->stringToProxy("test\\040test");
    test($b1->ice_getIdentity()->name == "test test" && $b1->ice_getIdentity()->category == "");
    try
    {
        $b1 = $ICE->stringToProxy("test\\777");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_IdentityParseException $ex)
    {
    }
    $b1 = $ICE->stringToProxy("test\\40test");
    test($b1->ice_getIdentity()->name == "test test");

    // Test some octal and hex corner cases.
    $b1 = $ICE->stringToProxy("test\\4test");
    test($b1->ice_getIdentity()->name == "test\4test");
    $b1 = $ICE->stringToProxy("test\\04test");
    test($b1->ice_getIdentity()->name == "test\4test");
    $b1 = $ICE->stringToProxy("test\\004test");
    test($b1->ice_getIdentity()->name == "test\4test");
    $b1 = $ICE->stringToProxy("test\\1114test");
    test($b1->ice_getIdentity()->name == "test\1114test");

    $b1 = $ICE->stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test");
    test($b1->ice_getIdentity()->name == "test\x08\x0c\n\r\t'\"\\test" && $b1->ice_getIdentity()->category == "");

    $b1 = $ICE->stringToProxy("category/test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
         $b1->ice_getAdapterId() == "");

    $b1 = $ICE->stringToProxy("test@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getAdapterId() == "adapter");
    try
    {
        $b1 = $ICE->stringToProxy("id@adapter test");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_ProxyParseException $ex)
    {
    }
    $b1 = $ICE->stringToProxy("category/test@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
         $b1->ice_getAdapterId() == "adapter");
    $b1 = $ICE->stringToProxy("category/test@adapter:tcp");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
         $b1->ice_getAdapterId() == "adapter:tcp");
    $b1 = $ICE->stringToProxy("'category 1/test'@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category 1" &&
         $b1->ice_getAdapterId() == "adapter");
    $b1 = $ICE->stringToProxy("'category/test 1'@adapter");
    test($b1->ice_getIdentity()->name == "test 1" && $b1->ice_getIdentity()->category == "category" &&
         $b1->ice_getAdapterId() == "adapter");
    $b1 = $ICE->stringToProxy("'category/test'@'adapter 1'");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
         $b1->ice_getAdapterId() == "adapter 1");
    $b1 = $ICE->stringToProxy("\"category \\/test@foo/test\"@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category /test@foo" &&
         $b1->ice_getAdapterId() == "adapter");
    $b1 = $ICE->stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category /test@foo" &&
         $b1->ice_getAdapterId() == "adapter:tcp");

    $b1 = $ICE->stringToProxy("id -f facet");
    test($b1->ice_getIdentity()->name == "id" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet");
    $b1 = $ICE->stringToProxy("id -f 'facet x'");
    test($b1->ice_getIdentity()->name == "id" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet x");
    $b1 = $ICE->stringToProxy("id -f \"facet x\"");
    test($b1->ice_getIdentity()->name == "id" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet x");
    try
    {
        $b1 = $ICE->stringToProxy("id -f \"facet x");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_ProxyParseException $ex)
    {
    }
    try
    {
        $b1 = $ICE->stringToProxy("id -f \'facet x");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_ProxyParseException $ex)
    {
    }
    $b1 = $ICE->stringToProxy("test -f facet:tcp");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet" && $b1->ice_getAdapterId() == "");
    $b1 = $ICE->stringToProxy("test -f \"facet:tcp\"");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet:tcp" && $b1->ice_getAdapterId() == "");
    $b1 = $ICE->stringToProxy("test -f facet@test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet" && $b1->ice_getAdapterId() == "test");
    $b1 = $ICE->stringToProxy("test -f 'facet@test'");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet@test" && $b1->ice_getAdapterId() == "");
    $b1 = $ICE->stringToProxy("test -f 'facet@test'@test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getFacet() == "facet@test" && $b1->ice_getAdapterId() == "test");
    try
    {
        $b1 = $ICE->stringToProxy("test -f facet@test @test");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_ProxyParseException $ex)
    {
    }
    $b1 = $ICE->stringToProxy("test");
    test($b1->ice_isTwoway());
    $b1 = $ICE->stringToProxy("test -t");
    test($b1->ice_isTwoway());
    $b1 = $ICE->stringToProxy("test -o");
    test($b1->ice_isOneway());
    $b1 = $ICE->stringToProxy("test -O");
    test($b1->ice_isBatchOneway());
    $b1 = $ICE->stringToProxy("test -d");
    test($b1->ice_isDatagram());
    $b1 = $ICE->stringToProxy("test -D");
    test($b1->ice_isBatchDatagram());
    $b1 = $ICE->stringToProxy("test");
    test(!$b1->ice_isSecure());
    $b1 = $ICE->stringToProxy("test -s");
    test($b1->ice_isSecure());

    try
    {
        $b1 = $ICE->stringToProxy("test:tcp@adapterId");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }
    // This is an unknown endpoint warning, not a parse exception.
    //
    //try
    //{
    //   $b1 = $ICE->stringToProxy("test -f the:facet:tcp");
    //   test(false);
    //}
    //catch(Ice_EndpointParseException $ex) //catch(Ice_UnknownLocalException $ex)
    //{
    //}
    try
    {
        $b1 = $ICE->stringToProxy("test::tcp");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }
    echo "ok\n";

    echo "testing propertyToProxy... ";
    $propertyPrefix = "Foo.Proxy";
    $ICE->setProperty($propertyPrefix, "test:default -p 12010 -t 10000");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getAdapterId() == "" && $b1->ice_getFacet() == "");

    // These two properties don't do anything to direct proxies so
    // first we test that.
    $property = $propertyPrefix . ".Locator";
    test(!$b1->ice_getLocator());
    $ICE->setProperty($property, "locator:default -p 10000");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test(!$b1->ice_getLocator());
    $ICE->setProperty($property, "");

    $property = $propertyPrefix . ".LocatorCacheTimeout";
    test($b1->ice_getLocatorCacheTimeout() == 0);
    $ICE->setProperty($property, "1");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_getLocatorCacheTimeout() == 0);
    $ICE->setProperty($property, "");

    // Now retest with an indirect proxy.
    $ICE->setProperty($propertyPrefix, "test");
    $property = $propertyPrefix . ".Locator";
    $ICE->setProperty($property, "locator:default -p 10000");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_getLocator() && $b1->ice_getLocator()->ice_getIdentity()->name == "locator");
    $ICE->setProperty($property, "");

    $property = $propertyPrefix . ".LocatorCacheTimeout";
    test($b1->ice_getLocatorCacheTimeout() == -1);
    $ICE->setProperty($property, "1");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_getLocatorCacheTimeout() == 1);
    $ICE->setProperty($property, "");

    // This cannot be tested so easily because the $property is cached
    // on $ICE initialization.
    //
    //$ICE->setProperty("Ice.Default.LocatorCacheTimeout", "60");
    //$b1 = $ICE->propertyToProxy($propertyPrefix);
    //test($b1->ice_getLocatorCacheTimeout() == 60);
    //$ICE->setProperty("Ice.Default.LocatorCacheTimeout", "");

    $ICE->setProperty($propertyPrefix, "test:default -p 12010 -t 10000");

    $property = $propertyPrefix . ".Router";
    test(!$b1->ice_getRouter());
    $ICE->setProperty($property, "router:default -p 10000");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_getRouter() && $b1->ice_getRouter()->ice_getIdentity()->name == "router");
    $ICE->setProperty($property, "");

    $property = $propertyPrefix . ".PreferSecure";
    test(!$b1->ice_isPreferSecure());
    $ICE->setProperty($property, "1");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_isPreferSecure());
    $ICE->setProperty($property, "");

    $property = $propertyPrefix . ".ConnectionCached";
    test($b1->ice_isConnectionCached());
    $ICE->setProperty($property, "0");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test(!$b1->ice_isConnectionCached());
    $ICE->setProperty($property, "");

    $property = $propertyPrefix . ".EndpointSelection";
    test($b1->ice_getEndpointSelection() == Ice_EndpointSelectionType::Random);
    $ICE->setProperty($property, "Random");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_getEndpointSelection() == Ice_EndpointSelectionType::Random);
    $ICE->setProperty($property, "Ordered");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_getEndpointSelection() == Ice_EndpointSelectionType::Ordered);
    $ICE->setProperty($property, "");

    //$property = $propertyPrefix . ".CollocationOptimization";
    //test($b1->ice_isCollocationOptimized());
    //$ICE->setProperty($property, "0");
    //$b1 = $ICE->propertyToProxy($propertyPrefix);
    //test(!$b1->ice_isCollocationOptimized());
    //$ICE->setProperty($property, "");

    $property = $propertyPrefix . ".ThreadPerConnection";
    test(!$b1->ice_isThreadPerConnection());
    $ICE->setProperty($property, "1");
    $b1 = $ICE->propertyToProxy($propertyPrefix);
    test($b1->ice_isThreadPerConnection());
    $ICE->setProperty($property, "");

    echo "ok\n";

    echo "testing proxy methods... ";
    flush();
    test($ICE->identityToString($base->ice_identity($ICE->stringToIdentity("other"))->ice_getIdentity()) == "other");
    test($base->ice_facet("facet")->ice_getFacet() == "facet");
    test($base->ice_adapterId("id")->ice_getAdapterId() == "id");
    test($base->ice_twoway()->ice_isTwoway());
    test($base->ice_oneway()->ice_isOneway());
    test($base->ice_batchOneway()->ice_isBatchOneway());
    test($base->ice_datagram()->ice_isDatagram());
    test($base->ice_batchDatagram()->ice_isBatchDatagram());
    test($base->ice_secure(true)->ice_isSecure());
    test(!$base->ice_secure(false)->ice_isSecure());
    echo "ok\n";

    echo "testing ice_getCommunicator... ";
    flush();
    test($base->ice_getCommunicator() === $ICE);
    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $cl = $base->ice_checkedCast("::Test::MyClass");
    test($cl != null);
    $derived = $cl->ice_checkedCast("::Test::MyDerivedClass");
    test($derived != null);
    test($cl == $base);
    test($derived == $base);
    test($cl == $derived);
    echo "ok\n";

    echo "testing checked cast with context... ";
    flush();
    $c = $cl->getContext();
    test(count($c) == 0);

    $c["one"] = "hello";
    $c["two"] = "world";
    $cl = $base->ice_checkedCast("::Test::MyClass", $c);
    $c2 = $cl->getContext();
    test($c == $c2);

    echo "ok\n";

    echo "testing opaque endpoints... ";

    try
    {
        // Invalid -x option
        $p = $ICE->stringToProxy("id:opaque -t 99 -v abc -x abc");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Missing -t and -v
        $p = $ICE->stringToProxy("id:opaque");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Repeated -t
        $p = $ICE->stringToProxy("id:opaque -t 1 -t 1 -v abc");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Repeated -v
        $p = $ICE->stringToProxy("id:opaque -t 1 -v abc -v abc");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Missing -t
        $p = $ICE->stringToProxy("id:opaque -v abc");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Missing -v
        $p = $ICE->stringToProxy("id:opaque -t 1");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Missing arg for -t
        $p = $ICE->stringToProxy("id:opaque -t -v abc");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Missing arg for -v
        $p = $ICE->stringToProxy("id:opaque -t 1 -v");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Not a number for -t
        $p = $ICE->stringToProxy("id:opaque -t x -v abc");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // < 0 for -t
        $p = $ICE->stringToProxy("id:opaque -t -1 -v abc");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    try
    {
        // Invalid char for -v
        $p = $ICE->stringToProxy("id:opaque -t 99 -v x?c");
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_EndpointParseException $ex)
    {
    }

    // Legal TCP endpoint expressed as opaque endpoint
    $p1 = $ICE->stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    $pstr = $ICE->proxyToString($p1);
    test($pstr == "test -t:tcp -h 127.0.0.1 -p 12010 -t 10000");
    
    // Working?
    $p1->ice_ping();

    // Two legal TCP endpoints expressed as opaque endpoints
    $p1 = $ICE->stringToProxy("test:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
    $pstr = $ICE->proxyToString($p1);
    test($pstr == "test -t:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

    //
    // Test that an SSL endpoint and a nonsense endpoint get written
    // back out as an opaque endpoint.
    //
    $p1 = $ICE->stringToProxy("test:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");
    $pstr = $ICE->proxyToString($p1);
    test($pstr == "test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");

    //
    // Try to invoke on the SSL endpoint to verify that we get a
    // NoEndpointException.
    //
    try
    {
        $p1->ice_ping();
        test(false);
    }
    catch(Ice_UnknownLocalException $ex) //catch(Ice_NoEndpointException $ex)
    {
    }

    //
    // Test that the proxy with an SSL endpoint and a nonsense
    // endpoint (which the server doesn't understand either) can be
    // sent over the wire and returned by the server without losing
    // the opaque endpoints.
    //
    $p2 = $derived->_echo($p1);
    $pstr = $ICE->proxyToString($p2);
    test($pstr == "test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch");

    echo "ok\n";

    return $cl;
}

$myClass = allTests();
$myClass->shutdown();

exit();
?>
