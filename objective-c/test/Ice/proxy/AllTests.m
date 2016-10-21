// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <ProxyTest.h>


TestProxyMyClassPrx*
proxyAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing stringToProxy... ");
    NSString* ref = @"test:default -p 12010";
    id<ICEObjectPrx> base = [communicator stringToProxy:ref];
    test(base);

    id<ICEObjectPrx> b1 = [communicator stringToProxy:@"test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getAdapterId] length] == 0 && [[b1 ice_getFacet] length] == 0);
    b1 = [communicator stringToProxy:@"test "];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] length] == 0);
    b1 = [communicator stringToProxy:@" test "];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] length] == 0);
    b1 = [communicator stringToProxy:@" test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] length] == 0);
    b1 = [communicator stringToProxy:@"'test -f facet'"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test -f facet"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] length] == 0);
    @try
    {
        b1 = [communicator stringToProxy:@"\"test -f facet'"];
        test(NO);
    }
    @catch(ICEProxyParseException*)
    {
    }
    b1 = [communicator stringToProxy:@"\"test -f facet\""];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test -f facet"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] length] == 0);
    b1 = [communicator stringToProxy:@"\"test -f facet@test\""];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test -f facet@test"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] length] == 0);
    b1 = [communicator stringToProxy:@"\"test -f facet@test @test\""];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test -f facet@test @test"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] length] == 0);
    @try
    {
        b1 = [communicator stringToProxy:@"test test"];
        test(NO);
    }
    @catch(ICEProxyParseException*)
    {
    }
    b1 = [communicator stringToProxy:@"test\\040test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test test"] &&
         [[[b1 ice_getIdentity] category] length] == 0);
    @try
    {
        b1 = [communicator stringToProxy:@"test\\777"];
        test(NO);
    }
    @catch(ICEIdentityParseException*)
    {
    }
    b1 = [communicator stringToProxy:@"test\\40test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test test"]);

    // TestProxy some octal and hex corner cases.
    b1 = [communicator stringToProxy:@"test\\4test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test\4test"]);
    b1 = [communicator stringToProxy:@"test\\04test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test\4test"]);
    b1 = [communicator stringToProxy:@"test\\004test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test\4test"]);
    b1 = [communicator stringToProxy:@"test\\1114test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test\1114test"]);

    b1 = [communicator stringToProxy:@"test\\b\\f\\n\\r\\t\\'\\\"\\\\test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test\b\f\n\r\t\'\"\\test"] &&
         [[[b1 ice_getIdentity] category] length] == 0);

    b1 = [communicator stringToProxy:@"category/test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category"] &&
         [[b1 ice_getAdapterId] length] == 0);

    b1 = [communicator stringToProxy:@"test@adapter"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter"]);
    @try
    {
        b1 = [communicator stringToProxy:@"id@adapter test"];
        test(NO);
    }
    @catch(ICEProxyParseException*)
    {
    }
    b1 = [communicator stringToProxy:@"category/test@adapter"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category"] &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter"]);
    b1 = [communicator stringToProxy:@"category/test@adapter:tcp"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category"] &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter:tcp"]);
    b1 = [communicator stringToProxy:@"'category 1/test'@adapter"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category 1"] &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter"]);
    b1 = [communicator stringToProxy:@"'category/test 1'@adapter"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test 1"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category"] &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter"]);
    b1 = [communicator stringToProxy:@"'category/test'@'adapter 1'"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category"] &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter 1"]);
    b1 = [communicator stringToProxy:@"\"category \\/test@foo/test\"@adapter"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category /test@foo"] &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter"]);
    b1 = [communicator stringToProxy:@"\"category \\/test@foo/test\"@\"adapter:tcp\""];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] &&
         [[[b1 ice_getIdentity] category] isEqualToString:@"category /test@foo"] &&
         [[b1 ice_getAdapterId] isEqualToString:@"adapter:tcp"]);

    b1 = [communicator stringToProxy:@"id -f facet"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"id"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet"]);
    b1 = [communicator stringToProxy:@"id -f 'facet x'"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"id"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet x"]);
    b1 = [communicator stringToProxy:@"id -f \"facet x\""];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"id"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet x"]);
    @try
    {
        b1 = [communicator stringToProxy:@"id -f \"facet x"];
        test(NO);
    }
    @catch(ICEProxyParseException*)
    {
    }
    @try
    {
        b1 = [communicator stringToProxy:@"id -f \'facet x"];
        test(NO);
    }
    @catch(ICEProxyParseException*)
    {
    }
    b1 = [communicator stringToProxy:@"test -f facet:tcp"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet"] && [[b1 ice_getAdapterId] length] == 0);
    b1 = [communicator stringToProxy:@"test -f \"facet:tcp\""];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet:tcp"] && [[b1 ice_getAdapterId] length] == 0);
    b1 = [communicator stringToProxy:@"test -f facet@test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet"] && [[b1 ice_getAdapterId] isEqualToString:@"test"]);
    b1 = [communicator stringToProxy:@"test -f 'facet@test'"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet@test"] && [[b1 ice_getAdapterId] length] == 0);
    b1 = [communicator stringToProxy:@"test -f 'facet@test'@test"];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getFacet] isEqualToString:@"facet@test"] && [[b1 ice_getAdapterId] isEqualToString:@"test"]);
    @try
    {
        b1 = [communicator stringToProxy:@"test -f facet@test @test"];
        test(NO);
    }
    @catch(ICEProxyParseException*)
    {
    }
    b1 = [communicator stringToProxy:@"test"];
    test([b1 ice_isTwoway]);
    b1 = [communicator stringToProxy:@"test -t"];
    test([b1 ice_isTwoway]);
    b1 = [communicator stringToProxy:@"test -o"];
    test([b1 ice_isOneway]);
    b1 = [communicator stringToProxy:@"test -O"];
    test([b1 ice_isBatchOneway]);
    b1 = [communicator stringToProxy:@"test -d"];
    test([b1 ice_isDatagram]);
    b1 = [communicator stringToProxy:@"test -D"];
    test([b1 ice_isBatchDatagram]);
    b1 = [communicator stringToProxy:@"test"];
    test(![b1 ice_isSecure]);
    b1 = [communicator stringToProxy:@"test -s"];
    test([b1 ice_isSecure]);

    test([[b1 ice_getEncodingVersion] isEqual:ICECurrentEncoding]);

    b1 = [communicator stringToProxy:@"test -e 1.0"];
    test([b1 ice_getEncodingVersion].major == 1 && [b1 ice_getEncodingVersion].minor == 0);

    b1 = [communicator stringToProxy:@"test -e 6.5"];
    test([b1 ice_getEncodingVersion].major == 6 && [b1 ice_getEncodingVersion].minor == 5);

    b1 = [communicator stringToProxy:@"test -p 1.0 -e 1.0"];
    test([[b1 ice_toString] isEqualToString:@"test -t -e 1.0"]);

    b1 = [communicator stringToProxy:@"test -p 6.5 -e 1.0"];
    test([[b1 ice_toString] isEqualToString:@"test -t -p 6.5 -e 1.0"]);

    @try
    {
        b1 = [communicator stringToProxy:@"test:tcp@adapterId"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }
    // This is an unknown endpoint warning, not a parse exception.
    //
    //try
    //{
    //   b1 = [communicator stringToProxy:@"test -f the:facet:tcp"];
    //   test(NO);
    //}
    //catch(ICEEndpointParseException*)
    //{
    //}
    @try
    {
        b1 = [communicator stringToProxy:@"test::tcp"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }
    tprintf("ok\n");

    tprintf("testing propertyToProxy... ");
    id<ICEProperties> prop = [communicator getProperties];
    NSString* propertyPrefix = @"Foo.Proxy";
    [prop setProperty:propertyPrefix value:@"test:default -p 12010"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([[[b1 ice_getIdentity] name] isEqualToString:@"test"] && [[[b1 ice_getIdentity] category] length] == 0 &&
         [[b1 ice_getAdapterId] length] == 0 && [[b1 ice_getFacet] length] == 0);

    NSString* property;

    property = [propertyPrefix stringByAppendingString:@".Locator"];
    test(![b1 ice_getLocator]);
    [prop  setProperty:property value:@"locator:default -p 10000"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getLocator] && [[[[b1 ice_getLocator] ice_getIdentity] name] isEqualToString:@"locator"]);
    [prop  setProperty:property value:@""];

    property = [propertyPrefix stringByAppendingString:@".LocatorCacheTimeout"];
    test([b1 ice_getLocatorCacheTimeout] == -1);
    [prop  setProperty:property value:@"1"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getLocatorCacheTimeout] == 1);
    [prop  setProperty:property value:@""];

    // Now retest with an indirect proxy.
    [prop setProperty:propertyPrefix value:@"test"];
    property = [propertyPrefix stringByAppendingString:@".Locator"];
    [prop  setProperty:property value:@"locator:default -p 10000"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getLocator] && [[[[b1 ice_getLocator] ice_getIdentity] name] isEqualToString:@"locator"]);
    [prop  setProperty:property value:@""];

    property = [propertyPrefix stringByAppendingString:@".LocatorCacheTimeout"];
    test([b1 ice_getLocatorCacheTimeout] == -1);
    [prop  setProperty:property value:@"1"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getLocatorCacheTimeout] == 1);
    [prop  setProperty:property value:@""];

    // This cannot be tested so easily because the property is cached
    // on communicator initialization.
    //
    //prop->setProperty:"Ice.Default.LocatorCacheTimeout" :@"60");
    //b1 = [communicator propertyToProxy:propertyPrefix];
    //test([b1 ice_getLocatorCacheTimeout] == 60);
    //prop->setProperty("Ice.Default.LocatorCacheTimeout" :@"");

    [prop setProperty:propertyPrefix value:@"test:default -p 12010"];

    property = [propertyPrefix stringByAppendingString:@".Router"];
    test(![b1 ice_getRouter]);
    [prop  setProperty:property value:@"router:default -p 10000"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getRouter] && [[[[b1 ice_getRouter] ice_getIdentity] name] isEqualToString:@"router"]);
    [prop  setProperty:property value:@""];

    property = [propertyPrefix stringByAppendingString:@".PreferSecure"];
    test(![b1 ice_isPreferSecure]);
    [prop  setProperty:property value:@"1"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_isPreferSecure]);
    [prop  setProperty:property value:@""];

    property = [propertyPrefix stringByAppendingString:@".ConnectionCached"];
    test([b1 ice_isConnectionCached]);
    [prop  setProperty:property value:@"0"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test(![b1 ice_isConnectionCached]);
    [prop  setProperty:property value:@""];

    property = [propertyPrefix stringByAppendingString:@".InvocationTimeout"];
    test([b1 ice_getInvocationTimeout] == -1);
    [prop  setProperty:property value:@"1000"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getInvocationTimeout] == 1000);
    [prop  setProperty:property value:@""];

    property = [propertyPrefix stringByAppendingString:@".EndpointSelection"];
    test([b1 ice_getEndpointSelection] == ICERandom);
    [prop  setProperty:property value:@"Random"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getEndpointSelection] == ICERandom);
    [prop  setProperty:property value:@"Ordered"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test([b1 ice_getEndpointSelection] == ICEOrdered);
    [prop  setProperty:property value:@""];

    property = [propertyPrefix stringByAppendingString:@".CollocationOptimized"];
    test([b1 ice_isCollocationOptimized]);
    [prop  setProperty:property value:@"0"];
    b1 = [communicator propertyToProxy:propertyPrefix];
    test(![b1 ice_isCollocationOptimized]);
    [prop  setProperty:property value:@""];

    tprintf("ok\n");

    tprintf("testing proxyToProperty... ");

    b1 = [communicator stringToProxy:@"test"];
    b1 = [b1 ice_collocationOptimized:YES];
    b1 = [b1 ice_connectionCached:YES];
    b1 = [b1 ice_preferSecure:false];
    b1 = [b1 ice_endpointSelection:ICEOrdered];
    b1 = [b1 ice_locatorCacheTimeout:100];
    b1 = [b1 ice_invocationTimeout:1234];
    ICEEncodingVersion* v = [ICEEncodingVersion encodingVersion:1 minor:0];
    b1 = [b1 ice_encodingVersion:v];
    id<ICEObjectPrx> router = [communicator stringToProxy:@"router"];
    router = [router ice_collocationOptimized:false];
    router = [router ice_connectionCached:YES];
    router = [router ice_preferSecure:YES];
    router = [router ice_endpointSelection:ICERandom];
    router = [router ice_locatorCacheTimeout:200];
    router = [router ice_invocationTimeout:1500];

    id<ICEObjectPrx> locator = [communicator stringToProxy:@"locator"];
    locator = [locator ice_collocationOptimized:YES];
    locator = [locator ice_connectionCached:false];
    locator = [locator ice_preferSecure:YES];
    locator = [locator ice_endpointSelection:ICERandom];
    locator = [locator ice_locatorCacheTimeout:300];
    locator = [locator ice_invocationTimeout:1500];

    locator = [locator ice_router:[ICERouterPrx uncheckedCast:router]];
    b1 = [b1 ice_locator:[ICELocatorPrx uncheckedCast:locator]];

    ICEMutablePropertyDict* proxyProps = [communicator proxyToProperty:b1 property:@"Test"];
    test([proxyProps count] == 21);

    test([[proxyProps objectForKey:@"Test"] isEqualToString:@"test -t -e 1.0"]);
    test([[proxyProps objectForKey:@"Test.CollocationOptimized"] isEqualToString:@"1"]);
    test([[proxyProps objectForKey:@"Test.ConnectionCached"] isEqualToString:@"1"]);
    test([[proxyProps objectForKey:@"Test.PreferSecure"] isEqualToString:@"0"]);
    test([[proxyProps objectForKey:@"Test.EndpointSelection"] isEqualToString:@"Ordered"]);
    test([[proxyProps objectForKey:@"Test.LocatorCacheTimeout"] isEqualToString:@"100"]);
    test([[proxyProps objectForKey:@"Test.InvocationTimeout"] isEqualToString:@"1234"]);

    NSString* sl = [NSString stringWithFormat:@"locator -t -e %@", [ICECurrentEncoding description]];
    test([[proxyProps objectForKey:@"Test.Locator"] isEqualToString:sl]);
    test([[proxyProps objectForKey:@"Test.Locator.CollocationOptimized"] isEqualToString:@"1"]);
    test([[proxyProps objectForKey:@"Test.Locator.ConnectionCached"] isEqualToString:@"0"]);
    test([[proxyProps objectForKey:@"Test.Locator.PreferSecure"] isEqualToString:@"1"]);
    test([[proxyProps objectForKey:@"Test.Locator.EndpointSelection"] isEqualToString:@"Random"]);
    test([[proxyProps objectForKey:@"Test.Locator.LocatorCacheTimeout"] isEqualToString:@"300"]);
    test([[proxyProps objectForKey:@"Test.Locator.InvocationTimeout"] isEqualToString:@"1500"]);

    NSString* sr = [NSString stringWithFormat:@"router -t -e %@", [ICECurrentEncoding description]];
    test([[proxyProps objectForKey:@"Test.Locator.Router"] isEqualToString:sr]);
    test([[proxyProps objectForKey:@"Test.Locator.Router.CollocationOptimized"] isEqualToString:@"0"]);
    test([[proxyProps objectForKey:@"Test.Locator.Router.ConnectionCached"] isEqualToString:@"1"]);
    test([[proxyProps objectForKey:@"Test.Locator.Router.PreferSecure"] isEqualToString:@"1"]);
    test([[proxyProps objectForKey:@"Test.Locator.Router.EndpointSelection"] isEqualToString:@"Random"]);
    test([[proxyProps objectForKey:@"Test.Locator.Router.LocatorCacheTimeout"] isEqualToString:@"200"]);
    test([[proxyProps objectForKey:@"Test.Locator.Router.InvocationTimeout"] isEqualToString:@"1500"]);

    tprintf("ok\n");

    tprintf("testing ice_getCommunicator... ");
    test([base ice_getCommunicator] == communicator);
    tprintf("ok\n");

    tprintf("testing proxy methods... ");
    test([[communicator identityToString:[[base ice_identity:[communicator stringToIdentity:@"other"]] ice_getIdentity]]
             isEqualToString:@"other"]);

    //
    // Verify that ToStringMode is passed correctly
    //
    ICEIdentity *ident = [ICEIdentity identity:@"test" category:@"\x7F\xE2\x82\xAC"];

    NSString *idStr = [ICEUtil identityToString:ident toStringMode:ICEUnicode];
    test([idStr isEqualToString:@"\\u007f\xE2\x82\xAC/test"]);
    ICEIdentity *id2 = [ICEUtil stringToIdentity:idStr];
    test([ident isEqual:id2]);
    test([[ICEUtil identityToString:ident] isEqualToString:idStr]);

    idStr = [ICEUtil identityToString:ident toStringMode:ICEASCII];
    test([idStr isEqualToString:@"\\u007f\\u20ac/test"]);
    id2 = [ICEUtil stringToIdentity:idStr];
    test([ident isEqual:id2]);

    idStr = [ICEUtil identityToString:ident toStringMode:ICECompat];
    test([idStr isEqualToString:@"\\177\\342\\202\\254/test"]);
    id2 = [ICEUtil stringToIdentity:idStr];
    test([ident isEqual:id2]);

    id2 = [ICEUtil stringToIdentity:[communicator identityToString:ident]];
    test([ident isEqual:id2]);

    test([[[base ice_facet:@"facet"] ice_getFacet] isEqualToString:@"facet"]);
    test([[[base ice_adapterId:@"id"] ice_getAdapterId] isEqualToString:@"id"]);
    test([[base ice_twoway] ice_isTwoway]);
    test([[base ice_oneway] ice_isOneway]);
    test([[base ice_batchOneway] ice_isBatchOneway]);
    test([[base ice_datagram] ice_isDatagram]);
    test([[base ice_batchDatagram] ice_isBatchDatagram]);
    test([[base ice_secure:YES] ice_isSecure]);
    test(![[base ice_secure:NO] ice_isSecure]);
    test([[base ice_collocationOptimized:YES] ice_isCollocationOptimized]);
    test(![[base ice_collocationOptimized:NO] ice_isCollocationOptimized]);
    test([[base ice_preferSecure:YES] ice_isPreferSecure]);
    test(![[base ice_preferSecure:NO] ice_isPreferSecure]);

    test([[[base ice_encodingVersion:ICEEncoding_1_0] ice_getEncodingVersion] isEqual:ICEEncoding_1_0]);
    test([[[base ice_encodingVersion:ICEEncoding_1_1] ice_getEncodingVersion] isEqual:ICEEncoding_1_1]);
    test(![[[base ice_encodingVersion:ICEEncoding_1_0] ice_getEncodingVersion] isEqual:ICEEncoding_1_1]);

    @try
    {
        [base ice_timeout:0];
        test(NO);
    }
    @catch(NSException* ex)
    {
        test([ex name] == NSInvalidArgumentException);
    }

    @try
    {
        [base ice_timeout:-1];
    }
    @catch(NSException* ex)
    {
        test(NO);
    }

    @try
    {
        [base ice_timeout:-2];
        test(NO);
    }
    @catch(NSException* ex)
    {
        test([ex name] == NSInvalidArgumentException);
    }

    @try
    {
        [base ice_invocationTimeout:0];
        test(NO);
    }
    @catch(NSException* ex)
    {
        test([ex name] == NSInvalidArgumentException);
    }

    @try
    {
        [base ice_invocationTimeout:-1];
        [base ice_invocationTimeout:-2];
    }
    @catch(NSException* ex)
    {
        test(NO);
    }

    @try
    {
        [base ice_locatorCacheTimeout:0];
    }
    @catch(NSException* ex)
    {
        test(NO);
    }

    @try
    {
        [base ice_locatorCacheTimeout:-1];
    }
    @catch(NSException* ex)
    {
        test(NO);
    }

    @try
    {
        [base ice_locatorCacheTimeout:-2];
        test(NO);
    }
    @catch(NSException* ex)
    {
        test([ex name] == NSInvalidArgumentException);
    }

    tprintf("ok\n");

    tprintf("testing proxy comparison... ");

    test([[communicator stringToProxy:@"foo"] isEqual:[communicator stringToProxy:@"foo"]]);
    test(![[communicator stringToProxy:@"foo"] isEqual:[communicator stringToProxy:@"foo2"]]);
//    test([communicator stringToProxy:@"foo"]] < [communicator stringToProxy:@"foo2"]);
//    test(!([communicator stringToProxy:@"foo2"]] < [communicator stringToProxy:@"foo"]);

    id<ICEObjectPrx> compObj = [communicator stringToProxy:@"foo"];

    test([[compObj ice_facet:@"facet"] isEqual:[compObj ice_facet:@"facet"]]);
    test(![[compObj ice_facet:@"facet"] isEqual:[compObj ice_facet:@"facet1"]]);
//    test([compObj ice_facet:@"facet"] < [compObj ice_facet:"facet1"]);
//    test(!([compObj ice_facet:@"facet"] < [compObj ice_facet:"facet"]));

    test([[compObj ice_oneway] isEqual:[compObj ice_oneway]]);
    test(![[compObj ice_oneway] isEqual:[compObj ice_twoway]]);
//     test([compObj ice_twoway] < [compObj ice_oneway]);
//     test(!([compObj ice_oneway] < [compObj ice_twoway]));

    test([[compObj ice_secure:YES] isEqual:[compObj ice_secure:YES]]);
    test(![[compObj ice_secure:NO] isEqual:[compObj ice_secure:YES]]);
//     test([compObj ice_secure:NO] < [compObj ice_secure:YES]);
//    test(!([compObj ice_secure:YES] < [compObj ice_secure:NO]));

    test([[compObj ice_collocationOptimized:YES] isEqual:[compObj ice_collocationOptimized:YES]]);
    test(![[compObj ice_collocationOptimized:NO] isEqual:[compObj ice_collocationOptimized:YES]]);
//    test([compObj ice_collocationOptimized:NO] < [compObj ice_collocationOptimized:YES]);
//    test(!([compObj ice_collocationOptimized:YES] < [compObj ice_collocationOptimized:NO]));

    test([[compObj ice_connectionCached:YES] isEqual:[compObj ice_connectionCached:YES]]);
    test(![[compObj ice_connectionCached:NO] isEqual:[compObj ice_connectionCached:YES]]);
//     test([compObj ice_connectionCached:NO] < [compObj ice_connectionCached:YES]);
//     test(!([compObj ice_connectionCached:YES] < [compObj ice_connectionCached:NO]));

    test([[compObj ice_endpointSelection:ICERandom] isEqual:[compObj ice_endpointSelection:ICERandom]]);
    test(![[compObj ice_endpointSelection:ICERandom] isEqual:[compObj ice_endpointSelection:ICEOrdered]]);
//     test([compObj ice_endpointSelection:ICERandom] < [compObj ice_endpointSelection:ICEOrdered]);
//     test(!([compObj ice_endpointSelection:ICEOrdered] < [compObj ice_endpointSelection:ICERandom]));

//    test([[compObj ice_connectionId:@"id2"] isEqual:[compObj ice_connectionId:@"id2"]]);
//    test(![[compObj ice_connectionId:@"id1"] isEqual:[compObj ice_connectionId:@"id2"]]);
//     test([compObj ice_connectionId:@"id1"] < [compObj ice_connectionId:"id2"]);
//     test(!([compObj ice_connectionId:@"id2"] < [compObj ice_connectionId:"id1"]));

    test([[compObj ice_compress:YES] isEqual:[compObj ice_compress:YES]]);
    test(![[compObj ice_compress:NO] isEqual:[compObj ice_compress:YES]]);
//     test([compObj ice_compress:NO] < [compObj ice_compress:YES]);
//     test(!([compObj ice_compress:YES] < [compObj ice_compress:NO]));

    test([[compObj ice_timeout:20] isEqual:[compObj ice_timeout:20]]);
    test(![[compObj ice_timeout:10] isEqual:[compObj ice_timeout:20]]);
//     test([compObj ice_timeout:10] < [compObj ice_timeout:20]);
//     test(!([compObj ice_timeout:20] < [compObj ice_timeout:10]));

    id<ICELocatorPrx> loc1 = [ICELocatorPrx uncheckedCast:[communicator stringToProxy:@"loc1:default -p 10000"]];
    id<ICELocatorPrx> loc2 = [ICELocatorPrx uncheckedCast:[communicator stringToProxy:@"loc2:default -p 10000"]];
    test([[compObj ice_locator:0] isEqual:[compObj ice_locator:0]]);
    test([[compObj ice_locator:loc1] isEqual:[compObj ice_locator:loc1]]);
    test(![[compObj ice_locator:loc1] isEqual:[compObj ice_locator:0]]);
    test(![[compObj ice_locator:0] isEqual:[compObj ice_locator:loc2]]);
    test(![[compObj ice_locator:loc1] isEqual:[compObj ice_locator:loc2]]);
//     test([compObj ice_locator:0] < [compObj ice_locator:loc1]);
//     test(!([compObj ice_locator:loc1] < [compObj ice_locator:0]));
//     test([compObj ice_locator:loc1] < [compObj ice_locator:loc2]);
//     test(!([compObj ice_locator:loc2] < [compObj ice_locator:loc1]));

    id<ICERouterPrx> rtr1 = [ICERouterPrx uncheckedCast:[communicator stringToProxy:@"rtr1:default -p 10000"]];
    id<ICERouterPrx> rtr2 = [ICERouterPrx uncheckedCast:[communicator stringToProxy:@"rtr2:default -p 10000"]];
    test([[compObj ice_router:0] isEqual:[compObj ice_router:0]]);
    test([[compObj ice_router:rtr1] isEqual:[compObj ice_router:rtr1]]);
    test(![[compObj ice_router:rtr1] isEqual:[compObj ice_router:0]]);
    test(![[compObj ice_router:0] isEqual:[compObj ice_router:rtr2]]);
    test(![[compObj ice_router:rtr1] isEqual:[compObj ice_router:rtr2]]);
//     test([compObj ice_router:0] < [compObj ice_router:rtr1]);
//     test(!([compObj ice_router:rtr1] < [compObj ice_router:0]));
//     test([compObj ice_router:rtr1] < [compObj ice_router:rtr2]);
//     test(!([compObj ice_router:rtr2] < [compObj ice_router:rtr1]));

    ICEMutableContext* ctx1 = [ICEMutableContext dictionary];
    [ctx1 setObject:@"v1" forKey:@"ctx1"];
    ICEMutableContext* ctx2 = [ICEMutableContext dictionary];
    [ctx2 setObject:@"v2" forKey:@"ctx2"];
    test([[compObj ice_context:nil] isEqual:[compObj ice_context:nil]]);
    test([[compObj ice_context:ctx1] isEqual:[compObj ice_context:ctx1]]);
    test(![[compObj ice_context:ctx1] isEqual:[compObj ice_context:nil]]);
    test(![[compObj ice_context:nil] isEqual:[compObj ice_context:ctx2]]);
    test(![[compObj ice_context:ctx1] isEqual:[compObj ice_context:ctx2]]);
//     test([compObj ice_context:ctx1] < [compObj ice_context:ctx2]);
//     test(!([compObj ice_context:ctx2] < [compObj ice_context:ctx1]));

    test([[compObj ice_preferSecure:YES] isEqual:[compObj ice_preferSecure:YES]]);
    test(![[compObj ice_preferSecure:YES] isEqual:[compObj ice_preferSecure:NO]]);
//     test([compObj ice_preferSecure:NO] < [compObj ice_preferSecure:YES]);
//     test(!([compObj ice_preferSecure:YES] < [compObj ice_preferSecure:NO]));

    id<ICEObjectPrx> compObj1 = [communicator stringToProxy:@"foo:tcp -h 127.0.0.1 -p 10000"];
    id<ICEObjectPrx> compObj2 = [communicator stringToProxy:@"foo:tcp -h 127.0.0.1 -p 10001"];
    test(![compObj isEqual:compObj2]);
//     test(compObj1 < compObj2);
//     test(!(compObj2 < compObj1));

    compObj1 = [communicator stringToProxy:@"foo@MyAdapter1"];
    compObj2 = [communicator stringToProxy:@"foo@MyAdapter2"];
    test(![compObj isEqual:compObj2]);
//     test(compObj1 < compObj2);
//     test(!(compObj2 < compObj1));

    test([[compObj1 ice_locatorCacheTimeout:20] isEqual:[compObj1 ice_locatorCacheTimeout:20]]);
    test(![[compObj1 ice_locatorCacheTimeout:10] isEqual:[compObj1 ice_locatorCacheTimeout:20]]);
//     test([compObj1 ice_locatorCacheTimeout:10] < [compObj1 ice_locatorCacheTimeout:20]);
//     test(!([compObj1 ice_locatorCacheTimeout:20] < [compObj1 ice_locatorCacheTimeout:10]));

    compObj1 = [communicator stringToProxy:@"foo:tcp -h 127.0.0.1 -p 1000"];
    compObj2 = [communicator stringToProxy:@"foo@MyAdapter1"];
    test(![compObj isEqual:compObj2]);
//     test(compObj1 < compObj2);
//     test(!(compObj2 < compObj1));

    test([[compObj1 ice_encodingVersion:ICEEncoding_1_0] isEqual:[compObj1 ice_encodingVersion:ICEEncoding_1_0]]);
    test(![[compObj1 ice_encodingVersion:ICEEncoding_1_0] isEqual:[compObj1 ice_encodingVersion:ICEEncoding_1_1]]);

//    test(compObj ice_encodingVersion:Ice::Encoding_1_0] < compObj->ice_encodingVersion(Ice::Encoding_1_1));
//    test(!(compObj->ice_encodingVersion(Ice::Encoding_1_1) < compObj->ice_encodingVersion(Ice::Encoding_1_0)));
    //
    // TODO: Ideally we should also test comparison of fixed proxies.
    //

    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestProxyMyClassPrx> cl = [TestProxyMyClassPrx checkedCast:base];
    test(cl);

    id<TestProxyMyDerivedClassPrx> derived = [TestProxyMyDerivedClassPrx checkedCast:cl];
    test(derived);
    test([cl isEqual:base]);
    test([derived isEqual:base]);
    test([cl isEqual:derived]);

    id<ICELocatorPrx> loc = [ICELocatorPrx checkedCast:base];
    test(loc == nil);

    //
    // Upcasting
    //
    id<TestProxyMyClassPrx> cl2 = [TestProxyMyClassPrx checkedCast:derived];
    id<ICEObjectPrx> obj = [ICEObjectPrx checkedCast:derived];
    test(cl2);
    test(obj);
    test([cl2 isEqual:obj]);
    test([cl2 isEqual:derived]);

    tprintf("ok\n");

    tprintf("testing checked cast with context... ");
    ICEMutableContext* c = [cl getContext];
    test([c count] == 0);

    [c setObject:@"hello" forKey:@"one"];
    [c setObject:@"world" forKey:@"two"];
    cl = [TestProxyMyClassPrx checkedCast:base context:c];
    ICEContext* c2 = [cl getContext];
    test([c isEqual:c2]);

    tprintf("ok\n");

    tprintf("testing encoding versioning... ");
    TestProxyMyClassPrx* cl20 = [TestProxyMyClassPrx uncheckedCast:
                                       [communicator stringToProxy:@"test -e 2.0:default -p 12010"]];
    @try
    {
        [cl20 ice_ping];
        test(NO);
    }
    @catch(ICEUnsupportedEncodingException*)
    {
        // Server 2.0 endpoint doesn't support 1.1 version.
    }

    TestProxyMyClassPrx* cl10 = [TestProxyMyClassPrx uncheckedCast:
                                       [communicator stringToProxy:@"test -e 1.0:default -p 12010"]];
    [cl10 ice_ping];
    [[cl10 ice_encodingVersion:ICEEncoding_1_0] ice_ping];
    //cl->ice_collocationOptimized(false)->ice_encodingVersion(Ice::Encoding_1_0)->ice_ping();

    // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
    // call will use the 1.1 encoding
    TestProxyMyClassPrx* cl13 = [TestProxyMyClassPrx uncheckedCast:
                                       [communicator stringToProxy:@"test -e 1.3:default -p 12010"]];
    [cl13 ice_ping];
    [cl13 end_ice_ping:[cl13 begin_ice_ping]];

    @try
    {
        // Send request with bogus 1.2 encoding.
        ICEEncodingVersion* version = [ICEEncodingVersion encodingVersion:1 minor:2];
        id<ICEOutputStream> out = [ICEUtil createOutputStream:communicator];
        [out startEncapsulation];
        [out endEncapsulation];
        NSMutableData* inEncaps = [out finished];
        ((ICEByte*)[inEncaps mutableBytes])[4] = version.major;
        ((ICEByte*)[inEncaps mutableBytes])[5] = version.minor;
        NSMutableData* outEncaps;
        [cl ice_invoke:@"ice_ping" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        test(NO);
    }
    @catch(ICEUnknownLocalException* ex)
    {
        // The server threw an UnsupportedEncodingException
        test([ex.unknown rangeOfString:@"UnsupportedEncodingException"].location != NSNotFound);
    }

    @try
    {
        // Send request with bogus 2.0 encoding.
        ICEEncodingVersion* version = [ICEEncodingVersion encodingVersion:2 minor:0];
        id<ICEOutputStream> out = [ICEUtil createOutputStream:communicator];
        [out startEncapsulation];
        [out endEncapsulation];
        NSMutableData* inEncaps = [out finished];
        ((ICEByte*)[inEncaps mutableBytes])[4] = version.major;
        ((ICEByte*)[inEncaps mutableBytes])[5] = version.minor;
        NSMutableData* outEncaps;
        [cl ice_invoke:@"ice_ping" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps];
        test(NO);
    }
    @catch(ICEUnknownLocalException* ex)
    {
        // The server thrown an UnsupportedEncodingException
        test([ex.unknown rangeOfString:@"UnsupportedEncodingException"].location != NSNotFound);
    }

    tprintf("ok\n");

    tprintf("testing protocol versioning... ");

    cl20 = [TestProxyMyClassPrx uncheckedCast:[communicator stringToProxy:@"test -p 2.0:default -p 12010"]];
    @try
    {
        [cl20 ice_ping];
        test(NO);
    }
    @catch(ICEUnsupportedProtocolException*)
    {
        // Server 2.0 proxy doesn't support 1.0 version.
    }

    cl10 = [TestProxyMyClassPrx uncheckedCast:[communicator stringToProxy:@"test -p 1.0:default -p 12010"]];
    [cl10 ice_ping];

    // 1.3 isn't supported but since a 1.3 proxy supports 1.0, the
    // call will use the 1.0 encoding
    cl13 = [TestProxyMyClassPrx uncheckedCast:[communicator stringToProxy:@"test -p 1.3:default -p 12010"]];
    [cl13 ice_ping];
    [cl13 end_ice_ping:[cl13 begin_ice_ping]];

    tprintf("ok\n");

    tprintf("testing opaque endpoints... ");

    @try
    {
        // Invalid -x option
        [communicator stringToProxy:@"id:opaque -t 99 -v abc -x abc"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Missing -t and -v
        [communicator stringToProxy:@"id:opaque"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Repeated -t
        [communicator stringToProxy:@"id:opaque -t 1 -t 1 -v abc"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Repeated -v
        [communicator stringToProxy:@"id:opaque -t 1 -v abc -v abc"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Missing -t
        [communicator stringToProxy:@"id:opaque -v abc"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Missing -v
        [communicator stringToProxy:@"id:opaque -t 1"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Missing arg for -t
        [communicator stringToProxy:@"id:opaque -t -v abc"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Missing arg for -v
        [communicator stringToProxy:@"id:opaque -t 1 -v"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Not a number for -t
        [communicator stringToProxy:@"id:opaque -t x -v abc"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // < 0 for -t
        [communicator stringToProxy:@"id:opaque -t -1 -v abc"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    @try
    {
        // Invalid char for -v
        [communicator stringToProxy:@"id:opaque -t 99 -v x?c"];
        test(NO);
    }
    @catch(ICEEndpointParseException*)
    {
    }

    // Legal TCP endpoint expressed as opaque endpoint
    id<ICEObjectPrx> p1 = [communicator stringToProxy:@"test -e 1.1:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA=="];
    NSString* pstr = [communicator proxyToString:p1];
    test([pstr isEqualToString:@"test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000"]);

    if([[communicator getProperties] getPropertyAsInt:@"Ice.IPv6"] == 0)
    {
        // Working?
        //BOOL ssl = [[[communicator getProperties] getProperty:@"Ice.Default.Protocol"] isEqualToString:@"ssl"];
        BOOL tcp = [[[communicator getProperties] getProperty:@"Ice.Default.Protocol"] isEqualToString:@"tcp"];
        if(tcp)
        {
            [[p1 ice_encodingVersion:ICEEncoding_1_0] ice_ping];
        }

        // Two legal TCP endpoints expressed as opaque endpoints
        p1 = [communicator stringToProxy:@"test -e 1.0:opaque -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA=="];
        pstr = [communicator proxyToString:p1];
        test([pstr isEqualToString:@"test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000"]);

#if 0 // SSL is always enabled
        //
        // TestProxy that an SSL endpoint and a nonsense endpoint get written
        // back out as an opaque endpoint.
        //
        p1 = [communicator stringToProxy:@"test:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch"];
        pstr = [communicator proxyToString:p1];
        if(!ssl)
        {
            test([pstr isEqualToString:@"test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch"]);
        }
        else
        {
            test([pstr isEqualToString:@"test -t:ssl -h 127.0.0.1 -p 10001:opaque -t 99 -v abch"]);
        }

        //
        // Try to invoke on the SSL endpoint to verify that we get a
        // NoEndpointException (or ConnectionRefusedException when
        // running with SSL).
        //
        @try
        {
            [p1 ice_ping];
            test(NO);
        }
        @catch(ICENoEndpointException*)
        {
            test(!ssl);
        }
        @catch(ICEConnectionRefusedException*)
        {
            test(ssl);
        }

        //
        // TestProxy that the proxy with an SSL endpoint and a nonsense
        // endpoint (which the server doesn't understand either) can be
        // sent over the wire and returned by the server without losing
        // the opaque endpoints.
        //
        id<ICEObjectPrx> p2 = [derived echo:p1];
        pstr = [communicator proxyToString:p2];
        if(!ssl)
        {
            test([pstr isEqualToString:@"test -t:opaque -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -v abch"]);
        }
        else
        {
            test([pstr isEqualToString:@"test -t:ssl -h 127.0.0.1 -p 10001:opaque -t 99 -v abch"]);
        }
#endif
    }

    tprintf("ok\n");

    return cl;
}
