<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

require_once('Test.php');

function currentEncodingToString()
{
    global $NS;
    return $NS ? eval("return Ice\\encodingVersionToString(Ice\\currentEncoding());") :
                 eval("return Ice_encodingVersionToString(Ice_currentEncoding());");
}

function allTests($helper)
{
    global $NS;
    global $Ice_Encoding_1_0;
    global $Ice_Encoding_1_1;

    $identity = $NS ? "Ice\\Identity" : "Ice_Identity";
    $random = $NS ? constant("Ice\\EndpointSelectionType::Random") : constant("Ice_EndpointSelectionType::Random");
    $ordered = $NS ? constant("Ice\\EndpointSelectionType::Ordered") : constant("Ice_EndpointSelectionType::Ordered");
    $encodingVersion = $NS ? "Ice\\EncodingVersion" : "Ice_EncodingVersion";

    $identityToString = $NS ? "Ice\\identityToString" : "Ice_identityToString";
    $stringToIdentity = $NS ? "Ice\\stringToIdentity" : "Ice_stringToIdentity";
    $modeUnicode = $NS ? constant("Ice\\ToStringMode::Unicode") : constant("Ice_ToStringMode::Unicode");
    $modeASCII = $NS ? constant("Ice\\ToStringMode::ASCII") : constant("Ice_ToStringMode::ASCII");
    $modeCompat = $NS ? constant("Ice\\ToStringMode::Compat") : constant("Ice_ToStringMode::Compat");

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
    try
    {
        $b1 = $communicator->stringToProxy("\"test -f facet'");
        test(false);
    }
    catch(Exception $ex)
    {
        $ppe = $NS ? "Ice\\ProxyParseException" : "Ice_ProxyParseException";
        if(!($ex instanceof $ppe))
        {
            throw $ex;
        }
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
    try
    {
        $b1 = $communicator->stringToProxy("test test");
        test(false);
    }
    catch(Exception $ex)
    {
        $ppe = $NS ? "Ice\\ProxyParseException" : "Ice_ProxyParseException";
        if(!($ex instanceof $ppe))
        {
            throw $ex;
        }
    }
    $b1 = $communicator->stringToProxy("test\\040test");
    test($b1->ice_getIdentity()->name == "test test" && $b1->ice_getIdentity()->category == "");
    try
    {
        $b1 = $communicator->stringToProxy("test\\777");
        test(false);
    }
    catch(Exception $ex)
    {
        $ipe = $NS ? "Ice\\IdentityParseException" : "Ice_IdentityParseException";
        if(!($ex instanceof $ipe))
        {
            throw $ex;
        }
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

    $b1 = $communicator->stringToProxy("test@adapter");
    test($b1->ice_getIdentity()->name == "test" && $b1->ice_getIdentity()->category == "" &&
         $b1->ice_getAdapterId() == "adapter");
    try
    {
        $b1 = $communicator->stringToProxy("id@adapter test");
        test(false);
    }
    catch(Exception $ex)
    {
        $ppe = $NS ? "Ice\\ProxyParseException" : "Ice_ProxyParseException";
        if(!($ex instanceof $ppe))
        {
            throw $ex;
        }
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
    try
    {
        $b1 = $communicator->stringToProxy("id -f \"facet x");
        test(false);
    }
    catch(Exception $ex)
    {
        $ppe = $NS ? "Ice\\ProxyParseException" : "Ice_ProxyParseException";
        if(!($ex instanceof $ppe))
        {
            throw $ex;
        }
    }
    try
    {
        $b1 = $communicator->stringToProxy("id -f \'facet x");
        test(false);
    }
    catch(Exception $ex)
    {
        $ppe = $NS ? "Ice\\ProxyParseException" : "Ice_ProxyParseException";
        if(!($ex instanceof $ppe))
        {
            throw $ex;
        }
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
    try
    {
        $b1 = $communicator->stringToProxy("test -f facet@test @test");
        test(false);
    }
    catch(Exception $ex)
    {
        $ppe = $NS ? "Ice\\ProxyParseException" : "Ice_ProxyParseException";
        if(!($ex instanceof $ppe))
        {
            throw $ex;
        }
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

    try
    {
        $b1 = $communicator->stringToProxy("test:tcp@adapterId");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }
    // This is an unknown endpoint warning, not a parse exception.
    //
    //try
    //{
    //   $b1 = $communicator->stringToProxy("test -f the:facet:tcp");
    //   test(false);
    //}
    //catch(Ice_EndpointParseException $ex)
    //{
    //}
    try
    {
        $b1 = $communicator->stringToProxy("test::tcp");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
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
    test($b1->ice_getEndpointSelection() == $random);
    $communicator->getProperties()->setProperty($property, "Random");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getEndpointSelection() == $random);
    $communicator->getProperties()->setProperty($property, "Ordered");
    $b1 = $communicator->propertyToProxy($propertyPrefix);
    test($b1->ice_getEndpointSelection() == $ordered);
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
    $b1 = $b1->ice_endpointSelection($ordered);
    $b1 = $b1->ice_locatorCacheTimeout(100);
    $b1 = $b1->ice_encodingVersion(eval("return new " . $encodingVersion . "(1, 0);"));

    $router = $communicator->stringToProxy("router");
    //$router = $router->ice_collocationOptimized(false);
    $router = $router->ice_connectionCached(true);
    $router = $router->ice_preferSecure(true);
    $router = $router->ice_endpointSelection($random);
    $router = $router->ice_locatorCacheTimeout(200);

    $locator = $communicator->stringToProxy("locator");
    //$locator = $locator->ice_collocationOptimized(true);
    $locator = $locator->ice_connectionCached(false);
    $locator = $locator->ice_preferSecure(true);
    $locator = $locator->ice_endpointSelection($random);
    $locator = $locator->ice_locatorCacheTimeout(300);

    $locator = $locator->ice_router($router->ice_uncheckedCast("::Ice::Router"));
    $b1 = $b1->ice_locator($locator->ice_uncheckedCast("::Ice::Locator"));

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
    test($communicator->identityToString($base->ice_identity($communicator->stringToIdentity("other"))->ice_getIdentity()) == "other");

    //
    // Verify that ToStringMode is passed correctly
    //
    $ident = eval("return new " . $identity . "('test', '\x7F\xE2\x82\xAC');");

    $idStr = $identityToString($ident, $modeUnicode);
    test($idStr == "\\u007f\xE2\x82\xAC/test");
    $ident2 = $stringToIdentity($idStr);
    test($ident == $ident2);
    test($identityToString($ident) == $idStr);

    $idStr = $identityToString($ident, $modeASCII);
    test($idStr == "\\u007f\\u20ac/test");
    $ident2 = $stringToIdentity($idStr);
    test($ident == $ident2);

    $idStr = $identityToString($ident, $modeCompat);
    test($idStr == "\\177\\342\\202\\254/test");
    $ident2 = $stringToIdentity($idStr);
    test($ident == $ident2);

    $ident2 = $stringToIdentity($communicator->identityToString($ident));
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

    $none = $NS ? constant("Ice\\None") : constant("Ice_Unset");

    test($base->ice_getCompress() == $none);
    test($base->ice_compress(true)->ice_getCompress() == true);
    test($base->ice_compress(false)->ice_getCompress() == false);

    test($base->ice_getTimeout() == $none);
    test($base->ice_timeout(10)->ice_getTimeout() == 10);
    test($base->ice_timeout(20)->ice_getTimeout() == 20);

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
    $clc = $base->ice_checkedCast("::Test::MyClass", $c);
    $c2 = $clc->getContext();
    test($c == $c2);

    echo "ok\n";

    echo "testing ice_fixed... ";
    flush();
    $connection = $cl->ice_getConnection();
    if($connection != null)
    {
        $cl->ice_fixed($connection)->getContext();
        test($cl->ice_secure(true)->ice_fixed($connection)->ice_isSecure());
        test($cl->ice_facet("facet")->ice_fixed($connection)->ice_getFacet() == "facet");
        test($cl->ice_oneway()->ice_fixed($connection)->ice_isOneway());
        $ctx = array();
        $ctx["one"] = "hello";
        $ctx["two"] = "world";
        echo count($cl->ice_fixed($connection)->ice_getContext());
        test($cl->ice_fixed($connection)->ice_getContext() == null);
        test(count($cl->ice_context($ctx)->ice_fixed($connection)->ice_getContext()) == 2);
        test($cl->ice_fixed($connection)->ice_getInvocationTimeout() == -1);
        test($cl->ice_invocationTimeout(10)->ice_fixed($connection)->ice_getInvocationTimeout() == 10);
        test($cl->ice_fixed($connection)->ice_getConnection() == $connection);
        test($cl->ice_fixed($connection)->ice_fixed($connection)->ice_getConnection() == $connection);
        test($cl->ice_fixed($connection)->ice_getTimeout() == $none);
        $fixedConnection = $cl->ice_connectionId("ice_fixed")->ice_getConnection();
        test($cl->ice_fixed($connection)->ice_fixed($fixedConnection)->ice_getConnection() == $fixedConnection);
        try
        {
            $cl->ice_secure(!$connection->getEndpoint()->getInfo()->secure())->ice_fixed($connection)->ice_ping();
        }
        catch(Exception $ex)
        {
        }
        try
        {
            $cl->ice_datagram()->ice_fixed($connection)->ice_ping();
        }
        catch(Exception $ex)
        {
        }
    }
    else
    {
        try
        {
            $cl->ice_fixed($connection);
            test(false);
        }
        catch(Exception $ex)
        {
            # Expected with null connection.
        }
    }
    echo "ok\n";

    echo "testing encoding versioning... ";
    flush();
    $ref20 = sprintf("test -e 2.0:%s", $helper->getTestEndpoint());
    $cl20 = $communicator->stringToProxy($ref20)->ice_uncheckedCast("::Test::MyClass");
    try
    {
        $cl20->ice_ping();
        test(false);
    }
    catch(Exception $ex)
    {
        // Server 2.0 endpoint doesn't support 1.1 version.
        $epe = $NS ? "Ice\\UnsupportedEncodingException" : "Ice_UnsupportedEncodingException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }
    $ref10 = sprintf("test -e 1.0:%s", $helper->getTestEndpoint());
    $cl10 = $communicator->stringToProxy($ref10)->ice_uncheckedCast("::Test::MyClass");
    $cl10->ice_ping();
    $cl10->ice_encodingVersion($Ice_Encoding_1_0)->ice_ping();
    $cl->ice_encodingVersion($Ice_Encoding_1_0)->ice_ping();

    // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
    // call will use the 1.1 encoding
    $ref13 = sprintf("test -e 1.3:%s", $helper->getTestEndpoint());
    $cl13 = $communicator->stringToProxy($ref13)->ice_uncheckedCast("::Test::MyClass");
    $cl13->ice_ping();
    echo "ok\n";

    echo "testing opaque endpoints... ";
    flush();

    try
    {
        // Invalid -x option
        $p = $communicator->stringToProxy("id:opaque -t 99 -v abc -x abc");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Missing -t and -v
        $p = $communicator->stringToProxy("id:opaque");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Repeated -t
        $p = $communicator->stringToProxy("id:opaque -t 1 -t 1 -v abc");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Repeated -v
        $p = $communicator->stringToProxy("id:opaque -t 1 -v abc -v abc");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Missing -t
        $p = $communicator->stringToProxy("id:opaque -v abc");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Missing -v
        $p = $communicator->stringToProxy("id:opaque -t 1");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Missing arg for -t
        $p = $communicator->stringToProxy("id:opaque -t -v abc");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Missing arg for -v
        $p = $communicator->stringToProxy("id:opaque -t 1 -v");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Not a number for -t
        $p = $communicator->stringToProxy("id:opaque -t x -v abc");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // < 0 for -t
        $p = $communicator->stringToProxy("id:opaque -t -1 -v abc");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    try
    {
        // Invalid char for -v
        $p = $communicator->stringToProxy("id:opaque -t 99 -v x?c");
        test(false);
    }
    catch(Exception $ex)
    {
        $epe = $NS ? "Ice\\EndpointParseException" : "Ice_EndpointParseException";
        if(!($ex instanceof $epe))
        {
            throw $ex;
        }
    }

    // Legal TCP endpoint expressed as opaque endpoint.
    $p1 = $communicator->stringToProxy("test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    $pstr = $communicator->proxyToString($p1);
    test($pstr == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

    // Opaque endpoint encoded with 1.1 encoding.
    $p2 = $communicator->stringToProxy("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
    test($communicator->proxyToString($p2) == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

    // Working?
    if($communicator->getProperties()->getPropertyAsInt("Ice.IPv6") == 0)
    {
        $ssl = $communicator->getProperties()->getProperty("Ice.Default.Protocol") == "ssl";
        $tcp = $communicator->getProperties()->getProperty("Ice.Default.Protocol") == "tcp";

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
        if($ssl)
        {
            test($pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");
        }
        elseif($tcp)
        {
            test($pstr == "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch");
        }

        //
        // Try to invoke on the SSL endpoint to verify that we get a
        // NoEndpointException (or ConnectionRefusedException when
        // running with SSL)->
        //
        try
        {
            $p1->ice_encodingVersion($Ice_Encoding_1_0)->ice_ping();
            test(false);
        }
        catch(Exception $ex)
        {
            $nee = $NS ? "Ice\\NoEndpointException" : "Ice_NoEndpointException";
            $cre = $NS ? "Ice\\ConnectionRefusedException" : "Ice_ConnectionRefusedException";
            if($ex instanceof $nee)
            {
                test(!$ssl);
            }
            elseif($ex instanceof $cre)
            {
                test(!$tcp);
            }
            else
            {
                throw $ex;
            }
        }

        //
        // Test that the proxy with an SSL endpoint and a nonsense
        // endpoint (which the server doesn't understand either) can be
        // sent over the wire and returned by the server without losing
        // the opaque endpoints.
        //
        $p2 = $derived->_echo($p1);
        $pstr = $communicator->proxyToString($p2);
        if($ssl)
        {
            test($pstr == "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");
        }
        elseif($tcp)
        {
            test($pstr == "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch");
        }
    }
    echo "ok\n";

    echo "testing communicator shutdown/destroy... ";
    $c = $NS ? eval("return Ice\\initialize();") : eval("return Ice_initialize();");
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
        try
        {
            $communicator = $this->initialize($args);
            $proxy= allTests($this);
            $proxy->shutdown();
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
        }
    }
}
?>
