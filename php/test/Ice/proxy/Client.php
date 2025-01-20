<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function currentEncodingToString()
{
    return Ice\encodingVersionToString(Ice\currentEncoding());
}

function allTests($helper)
{
    global $Ice_Encoding_1_0;
    global $Ice_Encoding_1_1;

    echo "testing stringToProxy... ";
    flush();
    $ref = sprintf("test:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $base = $communicator->stringToProxy($ref);
    test($base != null);

    $b1 = $communicator->stringToProxy("test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getAdapterId() == "" && $b1->ice_getFacet() == "");
    $b1 = $communicator->stringToProxy("test ");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "");
    $b1 = $communicator->stringToProxy(" test ");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "");
    $b1 = $communicator->stringToProxy(" test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "");
    $b1 = $communicator->stringToProxy("'test -f facet'");
    test($b1->ice_getIdentity()->name == "test -f facet" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "");
    try {
        $b1 = $communicator->stringToProxy("\"test -f facet'");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    $b1 = $communicator->stringToProxy("\"test -f facet\"");
    test($b1->ice_getIdentity()->name == "test -f facet" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "");
    $b1 = $communicator->stringToProxy("\"test -f facet@test\"");
    test($b1->ice_getIdentity()->name == "test -f facet@test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "");
    $b1 = $communicator->stringToProxy("\"test -f facet@test @test\"");
    test($b1->ice_getIdentity()->name == "test -f facet@test @test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "");
    try {
        $b1 = $communicator->stringToProxy("test test");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    $b1 = $communicator->stringToProxy("test\\040test");
    test($b1->ice_getIdentity()->name == "test test" && $b1->ice_getIdentity()->category == "");
    try {
        $b1 = $communicator->stringToProxy("test\\777");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    $b1 = $communicator->stringToProxy("test\\40test");
    test($b1->ice_getIdentity()->name == "test test");

    // Test some octal and hex corner cases.
    $b1 = $communicator->stringToProxy("test\\4test");
    test($b1->ice_getIdentity()->name == "test\4test");
    $b1 = $communicator->stringToProxy("test\\04test");
    test($b1->ice_getIdentity()->name == "test\4test");
    $b1 = $communicator->stringToProxy("test\\004test");
    test($b1->ice_getIdentity()->name == "test\4test");
    $b1 = $communicator->stringToProxy("test\\1114test");
    test($b1->ice_getIdentity()->name == "test\1114test");

    $b1 = $communicator->stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test");
    test($b1->ice_getIdentity()->name == "test\x08\x0c\n\r\t'\"\\test" && $b1->ice_getIdentity()->category == "");

    $b1 = $communicator->stringToProxy("category/test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
        $b1->ice_getAdapterId() == "");

    $b1 = $communicator->stringToProxy("test:tcp --sourceAddress \"::1\"");
    test($b1 == $communicator->stringToProxy($b1->ice_toString()));

    $b1 = $communicator->stringToProxy("test:udp --sourceAddress \"::1\" --interface \"0:0:0:0:0:0:0:1%lo\"");
    test($b1 == $communicator->stringToProxy($b1->ice_toString()));

    $b1 = $communicator->stringToProxy("test@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getAdapterId() == "adapter");
    try {
        $b1 = $communicator->stringToProxy("id@adapter test");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    $b1 = $communicator->stringToProxy("category/test@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
        $b1->ice_getAdapterId() == "adapter");
    $b1 = $communicator->stringToProxy("category/test@adapter:tcp");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
        $b1->ice_getAdapterId() == "adapter:tcp");
    $b1 = $communicator->stringToProxy("'category 1/test'@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category 1" &&
        $b1->ice_getAdapterId() == "adapter");
    $b1 = $communicator->stringToProxy("'category/test 1'@adapter");
    test($b1->ice_getIdentity()->name == "test 1" && $b1->ice_getIdentity()->category == "category" &&
        $b1->ice_getAdapterId() == "adapter");
    $b1 = $communicator->stringToProxy("'category/test'@'adapter 1'");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category" &&
        $b1->ice_getAdapterId() == "adapter 1");
    $b1 = $communicator->stringToProxy("\"category \\/test@foo/test\"@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category /test@foo" &&
        $b1->ice_getAdapterId() == "adapter");
    $b1 = $communicator->stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "category /test@foo" &&
        $b1->ice_getAdapterId() == "adapter:tcp");

    $b1 = $communicator->stringToProxy("id -f facet");
    test($b1->ice_getIdentity()->name == "id" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet");
    $b1 = $communicator->stringToProxy("id -f 'facet x'");
    test($b1->ice_getIdentity()->name == "id" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet x");
    $b1 = $communicator->stringToProxy("id -f \"facet x\"");
    test($b1->ice_getIdentity()->name == "id" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet x");
    try {
        $b1 = $communicator->stringToProxy("id -f \"facet x");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    try {
        $b1 = $communicator->stringToProxy("id -f \'facet x");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    $b1 = $communicator->stringToProxy("test -f facet:tcp");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet" && $b1->ice_getAdapterId() == "");
    $b1 = $communicator->stringToProxy("test -f \"facet:tcp\"");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet:tcp" && $b1->ice_getAdapterId() == "");
    $b1 = $communicator->stringToProxy("test -f facet@test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet" && $b1->ice_getAdapterId() == "test");
    $b1 = $communicator->stringToProxy("test -f 'facet@test'");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet@test" && $b1->ice_getAdapterId() == "");
    $b1 = $communicator->stringToProxy("test -f 'facet@test'@test");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getFacet() == "facet@test" && $b1->ice_getAdapterId() == "test");
    try {
        $b1 = $communicator->stringToProxy("test -f facet@test @test");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    $b1 = $communicator->stringToProxy("test");
    test($b1->ice_isTwoway());
    $b1 = $communicator->stringToProxy("test -t");
    test($b1->ice_isTwoway());
    $b1 = $communicator->stringToProxy("test -o");
    test($b1->ice_isOneway());
    $b1 = $communicator->stringToProxy("test -O");
    test($b1->ice_isBatchOneway());
    $b1 = $communicator->stringToProxy("test -d");
    test($b1->ice_isDatagram());
    $b1 = $communicator->stringToProxy("test -D");
    test($b1->ice_isBatchDatagram());
    $b1 = $communicator->stringToProxy("test");
    test(!$b1->ice_isSecure());
    $b1 = $communicator->stringToProxy("test -s");
    test($b1->ice_isSecure());

    try {
        $b1 = $communicator->stringToProxy("test:tcp@adapterId");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    // This is an unknown endpoint warning, not a parse exception.
    //
    //try
    //{
    //   $b1 = $communicator->stringToProxy("test -f the:facet:tcp");
    //   test(false);
    //}
    //catch(Ice_ParseException $ex)
    //{
    //}
    try {
        $b1 = $communicator->stringToProxy("test::tcp");
        test(false);
    } catch (Ice\ParseException $ex) {
    }
    echo "ok\n";

    echo "testing propertyToProxy... ";
    flush();
    $propertyPrefix = "Foo.Proxy";
    $communicator->getProperties()->setProperty($propertyPrefix, "test:default -p 12010");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
        $b1->ice_getAdapterId() == "" && $b1->ice_getFacet() == "");

    $property = $propertyPrefix . ".Locator";
    test(!$b1->ice_getLocator());
    $communicator->getProperties()->setProperty($property, "locator:default -p 10000");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getLocator() && $b1->ice_getLocator()->ice_getIdentity()->name == "locator");
    $communicator->getProperties()->setProperty($property, "");

    $property = $propertyPrefix . ".LocatorCacheTimeout";
    test($b1->ice_getLocatorCacheTimeout() == -1);
    $communicator->getProperties()->setProperty($property, "1");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getLocatorCacheTimeout() == 1);
    $communicator->getProperties()->setProperty($property, "");

    // Now retest with an indirect proxy.
    $communicator->getProperties()->setProperty($propertyPrefix, "test");
    $property = $propertyPrefix . ".Locator";
    $communicator->getProperties()->setProperty($property, "locator:default -p 10000");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getLocator() && $b1->ice_getLocator()->ice_getIdentity()->name == "locator");
    $communicator->getProperties()->setProperty($property, "");

    $property = $propertyPrefix . ".LocatorCacheTimeout";
    test($b1->ice_getLocatorCacheTimeout() == -1);
    $communicator->getProperties()->setProperty($property, "1");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getLocatorCacheTimeout() == 1);
    $communicator->getProperties()->setProperty($property, "");

    // This cannot be tested so easily because the $property is cached
    // on communicator initialization.
    //
    //$communicator->getProperties()->setProperty("Ice.Default.LocatorCacheTimeout", "60");
    //$b1 = $communicator->propertyToProxy($propertyPrefix);
    //test($b1->ice_getLocatorCacheTimeout() == 60);
    //$communicator->getProperties()->setProperty("Ice.Default.LocatorCacheTimeout", "");

    $communicator->getProperties()->setProperty($propertyPrefix, sprintf("test:%s", $helper->getTestEndpoint()));

    $property = $propertyPrefix . ".Router";
    test(!$b1->ice_getRouter());
    $communicator->getProperties()->setProperty($property, "router:default -p 10000");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getRouter() && $b1->ice_getRouter()->ice_getIdentity()->name == "router");
    $communicator->getProperties()->setProperty($property, "");

    $property = $propertyPrefix . ".PreferSecure";
    test(!$b1->ice_isPreferSecure());
    $communicator->getProperties()->setProperty($property, "1");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_isPreferSecure());
    $communicator->getProperties()->setProperty($property, "");

    $property = $propertyPrefix . ".ConnectionCached";
    test($b1->ice_isConnectionCached());
    $communicator->getProperties()->setProperty($property, "0");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test(!$b1->ice_isConnectionCached());
    $communicator->getProperties()->setProperty($property, "");

    $property = $propertyPrefix . ".EndpointSelection";
    test($b1->ice_getEndpointSelection() == Ice\EndpointSelectionType::Random);
    $communicator->getProperties()->setProperty($property, "Random");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getEndpointSelection() == Ice\EndpointSelectionType::Random);
    $communicator->getProperties()->setProperty($property, "Ordered");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getEndpointSelection() == Ice\EndpointSelectionType::Ordered);
    $communicator->getProperties()->setProperty($property, "");

    //$property = $propertyPrefix . ".CollocationOptimized";
    //test($b1->ice_isCollocationOptimized());
    //$communicator->getProperties()->setProperty($property, "0");
    //$b1 = $communicator->propertyToProxy($propertyPrefix);
    //test(!$b1->ice_isCollocationOptimized());
    //$communicator->getProperties()->setProperty($property, "");

    echo "ok\n";

    echo "testing proxyToProperty... ";
    flush();

    $b1 = $communicator->stringToProxy("test");
    //$b1 = $b1->ice_collocationOptimized(true);
    $b1 = $b1->ice_connectionCached(true);
    $b1 = $b1->ice_preferSecure(false);
    $b1 = $b1->ice_endpointSelection(Ice\EndpointSelectionType::Ordered);
    $b1 = $b1->ice_locatorCacheTimeout(100);
    $b1 = $b1->ice_encodingVersion(new Ice\EncodingVersion(1, 0));

    $router = Ice\RouterPrxHelper::createProxy($communicator, "router");
    //$router = $router->ice_collocationOptimized(false);
    $router = $router->ice_connectionCached(true);
    $router = $router->ice_preferSecure(true);
    $router = $router->ice_endpointSelection(Ice\EndpointSelectionType::Random);
    $router = $router->ice_locatorCacheTimeout(200);

    $locator = Ice\LocatorPrxHelper::createProxy($communicator, "locator");
    //$locator = $locator->ice_collocationOptimized(true);
    $locator = $locator->ice_connectionCached(false);
    $locator = $locator->ice_preferSecure(true);
    $locator = $locator->ice_endpointSelection(Ice\EndpointSelectionType::Random);
    $locator = $locator->ice_locatorCacheTimeout(300);

    $locator = $locator->ice_router($router);
    $b1 = $b1->ice_locator($locator);

    $proxyProps = $communicator->proxyToProperty($b1, "Test");
    test(count($proxyProps) == 21);

    test($proxyProps["Test"] == "test -t -e 1.0");
    //test($proxyProps["Test.CollocationOptimized"] == "1");
    test($proxyProps["Test.ConnectionCached"] == "1");
    test($proxyProps["Test.PreferSecure"] == "0");
    test($proxyProps["Test.EndpointSelection"] == "Ordered");
    test($proxyProps["Test.LocatorCacheTimeout"] == "100");

    test($proxyProps["Test.Locator"] == "locator -t -e " . currentEncodingToString());
    //test($proxyProps["Test.Locator.CollocationOptimized"] == "1");
    test($proxyProps["Test.Locator.ConnectionCached"] == "0");
    test($proxyProps["Test.Locator.PreferSecure"] == "1");
    test($proxyProps["Test.Locator.EndpointSelection"] == "Random");
    test($proxyProps["Test.Locator.LocatorCacheTimeout"] == "300");

    test($proxyProps["Test.Locator.Router"] == "router -t -e " . currentEncodingToString());
    //test($proxyProps["Test.Locator.Router.CollocationOptimized"] == "0");
    test($proxyProps["Test.Locator.Router.ConnectionCached"] == "1");
    test($proxyProps["Test.Locator.Router.PreferSecure"] == "1");
    test($proxyProps["Test.Locator.Router.EndpointSelection"] == "Random");
    test($proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200");

    echo "ok\n";

    echo "testing ice_getCommunicator... ";
    flush();
    test($base->ice_getCommunicator() === $communicator);
    echo "ok\n";

    echo "testing proxy methods... ";
    flush();
    test($communicator->identityToString($base->ice_identity(Ice\stringToIdentity("other"))->ice_getIdentity()) == "other");

    //
    // Verify that ToStringMode is passed correctly
    //
    $ident = new Ice\Identity("test", "\x7F\xE2\x82\xAC");
    $idStr = Ice\identityToString($ident, Ice\ToStringMode::Unicode);
    test($idStr == "\\u007f\xE2\x82\xAC/test");
    $ident2 = Ice\stringToIdentity($idStr);
    test($ident == $ident2);
    test(Ice\identityToString($ident) == $idStr);

    $idStr = Ice\identityToString($ident, Ice\ToStringMode::ASCII);
    test($idStr == "\\u007f\\u20ac/test");
    $ident2 = Ice\stringToIdentity($idStr);
    test($ident == $ident2);

    $idStr = Ice\identityToString($ident, Ice\ToStringMode::Compat);
    test($idStr == "\\177\\342\\202\\254/test");
    $ident2 = Ice\stringToIdentity($idStr);
    test($ident == $ident2);

    $ident2 = Ice\stringToIdentity($communicator->identityToString($ident));
    test($ident == $ident2);

    test($base->ice_facet("facet")->ice_getFacet() == "facet");
    test($base->ice_adapterId("id")->ice_getAdapterId() == "id");
    test($base->ice_twoway()->ice_isTwoway());
    test($base->ice_oneway()->ice_isOneway());
    test($base->ice_batchOneway()->ice_isBatchOneway());
    test($base->ice_datagram()->ice_isDatagram());
    test($base->ice_batchDatagram()->ice_isBatchDatagram());
    test($base->ice_secure(true)->ice_isSecure());
    test(!$base->ice_secure(false)->ice_isSecure());
    test($base->ice_preferSecure(true)->ice_isPreferSecure());
    test(!$base->ice_preferSecure(false)->ice_isPreferSecure());
    test($base->ice_connectionId("id1")->ice_getConnectionId() == "id1");
    test($base->ice_connectionId("id2")->ice_getConnectionId() == "id2");
    test($base->ice_encodingVersion($Ice_Encoding_1_0)->ice_getEncodingVersion() == $Ice_Encoding_1_0);
    test($base->ice_encodingVersion($Ice_Encoding_1_1)->ice_getEncodingVersion() == $Ice_Encoding_1_1);
    test($base->ice_encodingVersion($Ice_Encoding_1_0)->ice_getEncodingVersion() != $Ice_Encoding_1_1);

    test($base->ice_getCompress() == Ice\None);
    test($base->ice_compress(true)->ice_getCompress() == true);
    test($base->ice_compress(false)->ice_getCompress() == false);

    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $cl = Test\MyClassPrxHelper::checkedCast($base);
    test($cl != null);
    $derived = Test\MyDerivedClassPrxHelper::checkedCast($cl);
    test($derived != null);
    test($cl == $base);
    test($derived == $base);
    test($cl == $derived);

    try {
        Test\MyClassPrxHelper::checkedCast($base, "facet");
        test(false);
    } catch (Ice\FacetNotExistException $ex) {
        // Expected
    }
    echo "ok\n";

    echo "testing checked cast with context... ";
    flush();
    $c = $cl->getContext();
    test(count($c) == 0);

    $c["one"] = "hello";
    $c["two"] = "world";
    $clc = Test\MyClassPrxHelper::checkedCast($base, $c);
    $c2 = $clc->getContext();
    test($c == $c2);

    echo "ok\n";

    echo "testing ice_fixed... ";
    flush();
    $connection = $cl->ice_getConnection();
    if ($connection != null) {
        test(!$cl->ice_isFixed());
        test($cl->ice_fixed($connection)->ice_isFixed());
        $cl->ice_fixed($connection)->getContext();
        test($cl->ice_secure(true)->ice_fixed($connection)->ice_isSecure());
        test($cl->ice_facet("facet")->ice_fixed($connection)->ice_getFacet() == "facet");
        test($cl->ice_oneway()->ice_fixed($connection)->ice_isOneway());
        $ctx = array();
        $ctx["one"] = "hello";
        $ctx["two"] = "world";
        test($cl->ice_fixed($connection)->ice_getContext() == null);
        test(count($cl->ice_context($ctx)->ice_fixed($connection)->ice_getContext()) == 2);
        test($cl->ice_fixed($connection)->ice_getInvocationTimeout() == -1);
        test($cl->ice_invocationTimeout(10)->ice_fixed($connection)->ice_getInvocationTimeout() == 10);
        test($cl->ice_fixed($connection)->ice_getConnection() == $connection);
        test($cl->ice_fixed($connection)->ice_fixed($connection)->ice_getConnection() == $connection);
        $fixedConnection = $cl->ice_connectionId("ice_fixed")->ice_getConnection();
        test($cl->ice_fixed($connection)->ice_fixed($fixedConnection)->ice_getConnection() == $fixedConnection);
        try {
            $cl->ice_secure(!$connection->getEndpoint()->getInfo()->secure())->ice_fixed($connection)->ice_ping();
        } catch (Exception $ex) {
        }
        try {
            $cl->ice_datagram()->ice_fixed($connection)->ice_ping();
        } catch (Exception $ex) {
        }
    } else {
        try {
            $cl->ice_fixed($connection);
            test(false);
        } catch (Exception $ex) {
            # Expected with null connection.
        }
    }
    echo "ok\n";

    echo "testing encoding versioning... ";
    flush();
    $ref20 = sprintf("test -e 2.0:%s", $helper->getTestEndpoint());
    $cl20 = Test\MyClassPrxHelper::createProxy($communicator, $ref20);
    try {
        $cl20->ice_ping();
        test(false);
    } catch (Ice\MarshalException $ex) {
        // Server 2.0 endpoint doesn't support 1.1 version.
    }
    $ref10 = sprintf("test -e 1.0:%s", $helper->getTestEndpoint());
    $cl10 = Test\MyClassPrxHelper::createProxy($communicator, $ref10);
    $cl10->ice_ping();
    $cl10->ice_encodingVersion($Ice_Encoding_1_0)->ice_ping();
    $cl->ice_encodingVersion($Ice_Encoding_1_0)->ice_ping();

    echo "testing opaque endpoints... ";
    flush();

    try {
        // Invalid -x option
        $p = $communicator->stringToProxy("id:opaque -t 99 -v abc -x abc");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Missing -t and -v
        $p = $communicator->stringToProxy("id:opaque");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Repeated -t
        $p = $communicator->stringToProxy("id:opaque -t 1 -t 1 -v abc");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Repeated -v
        $p = $communicator->stringToProxy("id:opaque -t 1 -v abc -v abc");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Missing -t
        $p = $communicator->stringToProxy("id:opaque -v abc");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Missing -v
        $p = $communicator->stringToProxy("id:opaque -t 1");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Missing arg for -t
        $p = $communicator->stringToProxy("id:opaque -t -v abc");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Missing arg for -v
        $p = $communicator->stringToProxy("id:opaque -t 1 -v");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Not a number for -t
        $p = $communicator->stringToProxy("id:opaque -t x -v abc");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // < 0 for -t
        $p = $communicator->stringToProxy("id:opaque -t -1 -v abc");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    try {
        // Invalid char for -v
        $p = $communicator->stringToProxy("id:opaque -t 99 -v x?c");
        test(false);
    } catch (Ice\ParseException $ex) {
    }

    // Legal TCP endpoint expressed as opaque endpoint.
    $p1 = $communicator->stringToProxy("test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    $pstr = $communicator->proxyToString($p1);
    test($pstr == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

    // Opaque endpoint encoded with 1.1 encoding.
    $p2 = $communicator->stringToProxy("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    test($communicator->proxyToString($p2) == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

    // Working?
    if ($communicator->getProperties()->getIcePropertyAsInt("Ice.IPv6") == 0) {
        $tcp = $communicator->getProperties()->getIceProperty("Ice.Default.Protocol") == "tcp";

        // Two legal TCP endpoints expressed as opaque endpoints
        $p1 = $communicator->stringToProxy("test -e 1.0:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
        $pstr = $communicator->proxyToString($p1);
        test($pstr == "test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

        //
        // Test that an SSL endpoint and a nonsense endpoint get written
        // back out as an opaque endpoint.
        //
        $p1 = $communicator->stringToProxy("test -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch");
        $pstr = $communicator->proxyToString($p1);
        test($pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");

        //
        // Try to invoke on the SSL endpoint to verify that we get a
        // NoEndpointException (or ConnectionRefusedException when
        // running with SSL)->
        //
        try {
            $p1->ice_encodingVersion($Ice_Encoding_1_0)->ice_ping();
            test(false);
        } catch (Ice\ConnectionRefusedException $ex) {
        }

        //
        // Test that the proxy with an SSL endpoint and a nonsense
        // endpoint (which the server doesn't understand either) can be
        // sent over the wire and returned by the server without losing
        // the opaque endpoints.
        //
        $p2 = $derived->_echo($p1);
        $pstr = $communicator->proxyToString($p2);
        test($pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");
    }
    echo "ok\n";

    echo "testing communicator shutdown/destroy... ";
    $c = Ice\initialize();
    $c->shutdown();
    test($c->isShutdown());
    $c->waitForShutdown();
    $c->destroy();
    $c->shutdown();
    test($c->isShutdown());
    $c->waitForShutdown();
    $c->destroy();
    echo "ok\n";

    return $cl;
}

class Client extends TestHelper
{
    function run($args)
    {
        try {
            $communicator = $this->initialize($args);
            $proxy = allTests($this);
            $proxy->shutdown();
            $communicator->destroy();
        } catch (Exception $ex) {
            $communicator->destroy();
            throw $ex;
        }
    }
}
