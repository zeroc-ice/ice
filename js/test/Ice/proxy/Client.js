//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests()
        {
            class TestError extends Error
            {
            }

            const communicator = this.communicator();
            const out = this.getWriter();

            const defaultProtocol = communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp");

            out.write("testing stringToProxy... ");
            const ref = "test:" + this.getTestEndpoint();
            const base = communicator.stringToProxy(ref);
            test(base !== null);

            let b1 = communicator.stringToProxy("test");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getAdapterId().length === 0 && b1.ice_getFacet().length === 0);
            b1 = communicator.stringToProxy("test ");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet().length === 0);
            b1 = communicator.stringToProxy(" test ");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet().length === 0);
            b1 = communicator.stringToProxy(" test");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet().length === 0);
            b1 = communicator.stringToProxy("'test -f facet'");
            test(b1.ice_getIdentity().name === "test -f facet" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet().length === 0);

            try
            {
                b1 = communicator.stringToProxy("\"test -f facet'");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ProxyParseException, ex);
            }

            b1 = communicator.stringToProxy("\"test -f facet\"");
            test(b1.ice_getIdentity().name === "test -f facet" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet().length === 0);
            b1 = communicator.stringToProxy("\"test -f facet@test\"");
            test(b1.ice_getIdentity().name === "test -f facet@test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet().length === 0);
            b1 = communicator.stringToProxy("\"test -f facet@test @test\"");
            test(b1.ice_getIdentity().name === "test -f facet@test @test" &&
                 b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet().length === 0);

            try
            {
                b1 = communicator.stringToProxy("test test");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ProxyParseException, ex);
            }

            b1 = communicator.stringToProxy("test\\040test");
            test(b1.ice_getIdentity().name === "test test" && b1.ice_getIdentity().category.length === 0);

            try
            {
                b1 = communicator.stringToProxy("test\\777");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.IdentityParseException, ex);
            }

            b1 = communicator.stringToProxy("test\\40test");
            test(b1.ice_getIdentity().name === "test test");

            // Test some octal and hex corner cases.
            b1 = communicator.stringToProxy("test\\4test");
            test(b1.ice_getIdentity().name === "test\x04test");
            b1 = communicator.stringToProxy("test\\04test");
            test(b1.ice_getIdentity().name === "test\x04test");
            b1 = communicator.stringToProxy("test\\004test");
            test(b1.ice_getIdentity().name === "test\x04test");
            b1 = communicator.stringToProxy("test\\1114test");
            test(b1.ice_getIdentity().name === "test\x494test");

            b1 = communicator.stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test");
            test(b1.ice_getIdentity().name === "test\b\f\n\r\t'\"\\test" &&
                 b1.ice_getIdentity().category.length === 0);

            b1 = communicator.stringToProxy("category/test");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category === "category" &&
                 b1.ice_getAdapterId().length === 0);

            b1 = communicator.stringToProxy("");
            test(b1 === null);
            b1 = communicator.stringToProxy("\"\"");
            test(b1 === null);
            try
            {
                b1 = communicator.stringToProxy("\"\" test"); // Invalid trailing characters.
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ProxyParseException, ex);
            }

            try
            {
                b1 = communicator.stringToProxy("test:"); // Missing endpoint.
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            b1 = communicator.stringToProxy("test@adapter");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getAdapterId() === "adapter");
            try
            {
                b1 = communicator.stringToProxy("id@adapter test");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ProxyParseException, ex);
            }

            b1 = communicator.stringToProxy("category/test@adapter");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category === "category" &&
                 b1.ice_getAdapterId() === "adapter");
            b1 = communicator.stringToProxy("category/test@adapter:" + defaultProtocol);
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category === "category" &&
                 b1.ice_getAdapterId() === "adapter:" + defaultProtocol);
            b1 = communicator.stringToProxy("'category 1/test'@adapter");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category === "category 1" &&
                 b1.ice_getAdapterId() === "adapter");
            b1 = communicator.stringToProxy("'category/test 1'@adapter");
            test(b1.ice_getIdentity().name === "test 1" && b1.ice_getIdentity().category === "category" &&
                 b1.ice_getAdapterId() === "adapter");
            b1 = communicator.stringToProxy("'category/test'@'adapter 1'");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category === "category" &&
                 b1.ice_getAdapterId() === "adapter 1");
            b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@adapter");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category === "category /test@foo" &&
                 b1.ice_getAdapterId() === "adapter");
            b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@\"adapter:" + defaultProtocol + "\"");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category === "category /test@foo" &&
                 b1.ice_getAdapterId() === "adapter:" + defaultProtocol);

            b1 = communicator.stringToProxy("id -f facet");
            test(b1.ice_getIdentity().name === "id" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet");
            b1 = communicator.stringToProxy("id -f 'facet x'");
            test(b1.ice_getIdentity().name === "id" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet x");
            b1 = communicator.stringToProxy("id -f \"facet x\"");
            test(b1.ice_getIdentity().name === "id" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet x");

            try
            {
                b1 = communicator.stringToProxy("id -f \"facet x");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ProxyParseException, ex);
            }

            try
            {
                b1 = communicator.stringToProxy("id -f 'facet x");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ProxyParseException, ex);
            }

            b1 = communicator.stringToProxy("test -f facet:" + defaultProtocol);
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet" && b1.ice_getAdapterId().length === 0);
            b1 = communicator.stringToProxy("test -f \"facet:" + defaultProtocol + "\"");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet:" + defaultProtocol && b1.ice_getAdapterId().length === 0);
            b1 = communicator.stringToProxy("test -f facet@test");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet" && b1.ice_getAdapterId() === "test");
            b1 = communicator.stringToProxy("test -f 'facet@test'");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet@test" && b1.ice_getAdapterId().length === 0);
            b1 = communicator.stringToProxy("test -f 'facet@test'@test");
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getFacet() === "facet@test" && b1.ice_getAdapterId() === "test");

            try
            {
                b1 = communicator.stringToProxy("test -f facet@test @test");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.ProxyParseException, ex);
            }
            b1 = communicator.stringToProxy("test");
            test(b1.ice_isTwoway());
            b1 = communicator.stringToProxy("test -t");
            test(b1.ice_isTwoway());
            b1 = communicator.stringToProxy("test -o");
            test(b1.ice_isOneway());
            b1 = communicator.stringToProxy("test -O");
            test(b1.ice_isBatchOneway());
            b1 = communicator.stringToProxy("test -d");
            test(b1.ice_isDatagram());
            b1 = communicator.stringToProxy("test -D");
            test(b1.ice_isBatchDatagram());
            b1 = communicator.stringToProxy("test");
            test(!b1.ice_isSecure());
            b1 = communicator.stringToProxy("test -s");
            test(b1.ice_isSecure());

            test(b1.ice_getEncodingVersion().equals(Ice.currentEncoding()));

            b1 = communicator.stringToProxy("test -e 1.0");
            test(b1.ice_getEncodingVersion().major === 1 && b1.ice_getEncodingVersion().minor === 0);

            b1 = communicator.stringToProxy("test -e 6z.5");
            test(b1.ice_getEncodingVersion().major === 6 && b1.ice_getEncodingVersion().minor === 5);

            b1 = communicator.stringToProxy("test -p 1.0 -e 1.0");
            test(b1.toString() === "test -t -e 1.0");

            b1 = communicator.stringToProxy("test -p 6.5 -e 1.0");
            test(b1.toString() === "test -t -p 6.5 -e 1.0");

            try
            {
                b1 = communicator.stringToProxy("test:" + defaultProtocol + "@adapterId");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                b1 = communicator.stringToProxy("test::" + defaultProtocol);
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            //
            // Test for bug ICE-5543: escaped escapes in stringToIdentity
            //

            let id = new Ice.Identity("test", ",X2QNUAzSBcJ_e$AV;E\\");
            let id2 = Ice.stringToIdentity(Ice.identityToString(id));
            test(id.equals(id2));

            id = new Ice.Identity("test", ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\");
            id2 = Ice.stringToIdentity(Ice.identityToString(id));
            test(id.equals(id2));

            id = new Ice.Identity("/test", "cat/");
            let idStr = Ice.identityToString(id);
            test(idStr === "cat\\//\\/test");
            id2 = Ice.stringToIdentity(idStr);
            test(id.equals(id2));

            // Input string with various pitfalls
            // id = Ice.stringToIdentity("\\342\\x82\\254\\60\\x9\\60\\");
            // test(id.name === "€0\t0\\" && id.category.isEmpty());

            try
            {
                // Illegal character < 32
                id = Ice.stringToIdentity("xx\x01FooBar");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.IdentityParseException);
            }

            try
            {
                // Illegal surrogate
                id = Ice.stringToIdentity("xx\\ud911");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.IdentityParseException, ex);
            }

            // Testing bytes 127 (\x7F) and €
            id = new Ice.Identity("test", "\x7F€");

            idStr = Ice.identityToString(id, Ice.ToStringMode.Unicode);
            test(idStr === "\\u007f€/test");
            id2 = Ice.stringToIdentity(idStr);
            test(id.equals(id2));
            test(Ice.identityToString(id) === idStr);

            idStr = Ice.identityToString(id, Ice.ToStringMode.ASCII);
            test(idStr === "\\u007f\\u20ac/test");
            id2 = Ice.stringToIdentity(idStr);
            test(id.equals(id2));

            idStr = Ice.identityToString(id, Ice.ToStringMode.Compat);
            test(idStr === "\\177\\342\\202\\254/test");
            id2 = Ice.stringToIdentity(idStr);
            test(id.equals(id2));

            id2 = Ice.stringToIdentity(communicator.identityToString(id));
            test(id.equals(id2));

            // More unicode characters

            id = new Ice.Identity("banana \x0e-\ud83c\udf4c\u20ac\u00a2\u0024", "greek \ud800\udd6a");

            idStr = Ice.identityToString(id, Ice.ToStringMode.Unicode);
            test(idStr === "greek \ud800\udd6a/banana \\u000e-\ud83c\udf4c\u20ac\u00a2$");
            id2 = Ice.stringToIdentity(idStr);
            test(id.equals(id2));

            idStr = Ice.identityToString(id, Ice.ToStringMode.ASCII);
            test(idStr === "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$");
            id2 = Ice.stringToIdentity(idStr);
            test(id.equals(id2));

            idStr = Ice.identityToString(id, Ice.ToStringMode.Compat);
            test(idStr === "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$");
            id2 = Ice.stringToIdentity(idStr);
            test(id.equals(id2));

            out.writeLine("ok");

            out.write("testing propertyToProxy... ");
            const prop = communicator.getProperties();
            const propertyPrefix = "Foo.Proxy";
            prop.setProperty(propertyPrefix, "test:" + this.getTestEndpoint());
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getIdentity().name === "test" && b1.ice_getIdentity().category.length === 0 &&
                 b1.ice_getAdapterId().length === 0 && b1.ice_getFacet().length === 0);

            let property = propertyPrefix + ".Locator";
            test(b1.ice_getLocator() === null);
            prop.setProperty(property, "locator:default -p 10000");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getLocator() !== null && b1.ice_getLocator().ice_getIdentity().name === "locator");
            prop.setProperty(property, "");

            property = propertyPrefix + ".LocatorCacheTimeout";
            test(b1.ice_getLocatorCacheTimeout() === -1);
            prop.setProperty(property, "1");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getLocatorCacheTimeout() === 1);
            prop.setProperty(property, "");

            // Now retest with an indirect proxy.
            prop.setProperty(propertyPrefix, "test");
            property = propertyPrefix + ".Locator";
            prop.setProperty(property, "locator:default -p 10000");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getLocator() !== null && b1.ice_getLocator().ice_getIdentity().name === "locator");
            prop.setProperty(property, "");

            property = propertyPrefix + ".LocatorCacheTimeout";
            test(b1.ice_getLocatorCacheTimeout() === -1);
            prop.setProperty(property, "1");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getLocatorCacheTimeout() === 1);
            prop.setProperty(property, "");

            prop.setProperty(propertyPrefix, "test:" + this.getTestEndpoint());

            property = propertyPrefix + ".Router";
            test(b1.ice_getRouter() === null);
            prop.setProperty(property, "router:default -p 10000");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getRouter() !== null && b1.ice_getRouter().ice_getIdentity().name === "router");
            prop.setProperty(property, "");

            property = propertyPrefix + ".PreferSecure";
            test(!b1.ice_isPreferSecure());
            prop.setProperty(property, "1");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_isPreferSecure());
            prop.setProperty(property, "");

            property = propertyPrefix + ".ConnectionCached";
            test(b1.ice_isConnectionCached());
            prop.setProperty(property, "0");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(!b1.ice_isConnectionCached());
            prop.setProperty(property, "");

            property = propertyPrefix + ".InvocationTimeout";
            test(b1.ice_getInvocationTimeout() == -1);
            prop.setProperty(property, "1000");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getInvocationTimeout() == 1000);
            prop.setProperty(property, "");

            property = propertyPrefix + ".EndpointSelection";
            test(b1.ice_getEndpointSelection() === Ice.EndpointSelectionType.Random);
            prop.setProperty(property, "Random");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getEndpointSelection() === Ice.EndpointSelectionType.Random);
            prop.setProperty(property, "Ordered");
            b1 = communicator.propertyToProxy(propertyPrefix);
            test(b1.ice_getEndpointSelection() === Ice.EndpointSelectionType.Ordered);
            prop.setProperty(property, "");

            out.writeLine("ok");

            out.write("testing proxyToProperty... ");

            b1 = communicator.stringToProxy("test");
            b1 = b1.ice_connectionCached(true);
            b1 = b1.ice_preferSecure(false);
            b1 = b1.ice_endpointSelection(Ice.EndpointSelectionType.Ordered);
            b1 = b1.ice_locatorCacheTimeout(100);
            b1 = b1.ice_invocationTimeout(1234);
            b1 = b1.ice_encodingVersion(new Ice.EncodingVersion(1, 0));

            let router = communicator.stringToProxy("router");
            router = router.ice_connectionCached(true);
            router = router.ice_preferSecure(true);
            router = router.ice_endpointSelection(Ice.EndpointSelectionType.Random);
            router = router.ice_locatorCacheTimeout(200);
            router = router.ice_invocationTimeout(1500);

            let locator = communicator.stringToProxy("locator");
            locator = locator.ice_connectionCached(false);
            locator = locator.ice_preferSecure(true);
            locator = locator.ice_endpointSelection(Ice.EndpointSelectionType.Random);
            locator = locator.ice_locatorCacheTimeout(300);
            locator = locator.ice_invocationTimeout(1500);

            locator = locator.ice_router(Ice.RouterPrx.uncheckedCast(router));
            b1 = b1.ice_locator(Ice.LocatorPrx.uncheckedCast(locator));

            const proxyProps = communicator.proxyToProperty(b1, "Test");
            test(proxyProps.size === 21);
            test(proxyProps.get("Test") === "test -t -e 1.0");
            test(proxyProps.get("Test.CollocationOptimized") === "0");
            test(proxyProps.get("Test.ConnectionCached") === "1");
            test(proxyProps.get("Test.PreferSecure") === "0");
            test(proxyProps.get("Test.EndpointSelection") === "Ordered");
            test(proxyProps.get("Test.LocatorCacheTimeout") === "100");
            test(proxyProps.get("Test.InvocationTimeout") === "1234");

            test(proxyProps.get("Test.Locator") === "locator -t -e " +
                 Ice.encodingVersionToString(Ice.currentEncoding()));
            test(proxyProps.get("Test.Locator.CollocationOptimized") === "0");
            test(proxyProps.get("Test.Locator.ConnectionCached") === "0");
            test(proxyProps.get("Test.Locator.PreferSecure") === "1");
            test(proxyProps.get("Test.Locator.EndpointSelection") === "Random");
            test(proxyProps.get("Test.Locator.LocatorCacheTimeout") === "300");
            test(proxyProps.get("Test.Locator.InvocationTimeout") === "1500");

            test(proxyProps.get("Test.Locator.Router") === "router -t -e " +
                 Ice.encodingVersionToString(Ice.currentEncoding()));
            test(proxyProps.get("Test.Locator.Router.CollocationOptimized") === "0");
            test(proxyProps.get("Test.Locator.Router.ConnectionCached") === "1");
            test(proxyProps.get("Test.Locator.Router.PreferSecure") === "1");
            test(proxyProps.get("Test.Locator.Router.EndpointSelection") === "Random");
            test(proxyProps.get("Test.Locator.Router.LocatorCacheTimeout") === "200");
            test(proxyProps.get("Test.Locator.Router.InvocationTimeout") === "1500");

            out.writeLine("ok");

            out.write("testing ice_getCommunicator... ");
            test(base.ice_getCommunicator() === communicator);
            out.writeLine("ok");

            out.write("testing proxy methods... ");
            test(communicator.identityToString(
                base.ice_identity(Ice.stringToIdentity("other")).ice_getIdentity()) === "other");
            test(Ice.identityToString(
                base.ice_identity(Ice.stringToIdentity("other")).ice_getIdentity()) === "other");
            test(base.ice_facet("facet").ice_getFacet() === "facet");
            test(base.ice_adapterId("id").ice_getAdapterId() === "id");
            test(base.ice_twoway().ice_isTwoway());
            test(base.ice_oneway().ice_isOneway());
            test(base.ice_batchOneway().ice_isBatchOneway());
            test(base.ice_datagram().ice_isDatagram());
            test(base.ice_batchDatagram().ice_isBatchDatagram());
            test(base.ice_secure(true).ice_isSecure());
            test(!base.ice_secure(false).ice_isSecure());
            test(base.ice_preferSecure(true).ice_isPreferSecure());
            test(!base.ice_preferSecure(false).ice_isPreferSecure());
            test(base.ice_encodingVersion(Ice.Encoding_1_0).ice_getEncodingVersion().equals(Ice.Encoding_1_0));
            test(base.ice_encodingVersion(Ice.Encoding_1_1).ice_getEncodingVersion().equals(Ice.Encoding_1_1));
            test(!base.ice_encodingVersion(Ice.Encoding_1_0).ice_getEncodingVersion().equals(Ice.Encoding_1_1));

            try
            {
                base.ice_timeout(0);
                test(false);
            }
            catch(ex)
            {
                test(!(ex instanceof TestError), ex);
            }

            try
            {
                base.ice_timeout(-1);
            }
            catch(ex)
            {
                test(false, ex);
            }

            try
            {
                base.ice_timeout(-2);
                test(false);
            }
            catch(ex)
            {
                test(!(ex instanceof TestError), ex);
            }

            try
            {
                base.ice_invocationTimeout(0);
                test(false);
            }
            catch(ex)
            {
                test(!(ex instanceof TestError), ex);
            }

            try
            {
                base.ice_invocationTimeout(-1);
            }
            catch(ex)
            {
                test(false);
            }

            try
            {
                base.ice_invocationTimeout(-2);
                test(false);
            }
            catch(ex)
            {
                test(!(ex instanceof TestError), ex);
            }

            try
            {
                base.ice_locatorCacheTimeout(0);
            }
            catch(ex)
            {
                test(false, ex);
            }

            try
            {
                base.ice_locatorCacheTimeout(-1);
            }
            catch(ex)
            {
                test(false, ex);
            }

            try
            {
                base.ice_locatorCacheTimeout(-2);
                test(false);
            }
            catch(ex)
            {
                test(!(ex instanceof TestError), ex);
            }

            out.writeLine("ok");

            out.write("testing proxy comparison... ");

            test(communicator.stringToProxy("foo").equals(communicator.stringToProxy("foo")));
            test(!communicator.stringToProxy("foo").equals(communicator.stringToProxy("foo2")));

            const compObj = communicator.stringToProxy("foo");

            test(compObj.ice_facet("facet").equals(compObj.ice_facet("facet")));
            test(!compObj.ice_facet("facet").equals(compObj.ice_facet("facet1")));

            test(compObj.ice_oneway().equals(compObj.ice_oneway()));
            test(!compObj.ice_oneway().equals(compObj.ice_twoway()));

            test(compObj.ice_secure(true).equals(compObj.ice_secure(true)));
            test(!compObj.ice_secure(false).equals(compObj.ice_secure(true)));

            test(compObj.ice_connectionCached(true).equals(compObj.ice_connectionCached(true)));
            test(!compObj.ice_connectionCached(false).equals(compObj.ice_connectionCached(true)));

            test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).equals(
                compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random)));
            test(!compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).equals(
                compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered)));

            test(compObj.ice_connectionId("id2").equals(compObj.ice_connectionId("id2")));
            test(!compObj.ice_connectionId("id1").equals(compObj.ice_connectionId("id2")));

            test(compObj.ice_connectionId("id1").ice_getConnectionId() === "id1");
            test(compObj.ice_connectionId("id2").ice_getConnectionId() === "id2");

            test(compObj.ice_timeout(20).equals(compObj.ice_timeout(20)));
            test(!compObj.ice_timeout(10).equals(compObj.ice_timeout(20)));

            test(compObj.ice_getTimeout() === undefined);
            test(compObj.ice_timeout(10).ice_getTimeout() == 10);
            test(compObj.ice_timeout(20).ice_getTimeout() == 20);

            const loc1 = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy("loc1:default -p 10000"));
            const loc2 = Ice.LocatorPrx.uncheckedCast(communicator.stringToProxy("loc2:default -p 10000"));
            test(compObj.ice_locator(null).equals(compObj.ice_locator(null)));
            test(compObj.ice_locator(loc1).equals(compObj.ice_locator(loc1)));
            test(!compObj.ice_locator(loc1).equals(compObj.ice_locator(null)));
            test(!compObj.ice_locator(null).equals(compObj.ice_locator(loc2)));
            test(!compObj.ice_locator(loc1).equals(compObj.ice_locator(loc2)));

            const rtr1 = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("rtr1:default -p 10000"));
            const rtr2 = Ice.RouterPrx.uncheckedCast(communicator.stringToProxy("rtr2:default -p 10000"));
            test(compObj.ice_router(null).equals(compObj.ice_router(null)));
            test(compObj.ice_router(rtr1).equals(compObj.ice_router(rtr1)));
            test(!compObj.ice_router(rtr1).equals(compObj.ice_router(null)));
            test(!compObj.ice_router(null).equals(compObj.ice_router(rtr2)));
            test(!compObj.ice_router(rtr1).equals(compObj.ice_router(rtr2)));

            const ctx1 = new Map();
            ctx1.set("ctx1", "v1");
            const ctx2 = new Map();
            ctx2.set("ctx2", "v2");
            test(compObj.ice_context(null).equals(compObj.ice_context(null)));
            test(compObj.ice_context(ctx1).equals(compObj.ice_context(ctx1)));
            test(!compObj.ice_context(ctx1).equals(compObj.ice_context(null)));
            test(!compObj.ice_context(null).equals(compObj.ice_context(ctx2)));
            test(!compObj.ice_context(ctx1).equals(compObj.ice_context(ctx2)));

            test(compObj.ice_preferSecure(true).equals(compObj.ice_preferSecure(true)));
            test(!compObj.ice_preferSecure(true).equals(compObj.ice_preferSecure(false)));

            let compObj1 = communicator.stringToProxy("foo:" + defaultProtocol + " -h 127.0.0.1 -p 10000");
            let compObj2 = communicator.stringToProxy("foo:" + defaultProtocol + " -h 127.0.0.1 -p 10001");
            test(!compObj1.equals(compObj2));

            compObj1 = communicator.stringToProxy("foo@MyAdapter1");
            compObj2 = communicator.stringToProxy("foo@MyAdapter2");
            test(!compObj1.equals(compObj2));

            test(compObj1.ice_locatorCacheTimeout(20).equals(compObj1.ice_locatorCacheTimeout(20)));
            test(!compObj1.ice_locatorCacheTimeout(10).equals(compObj1.ice_locatorCacheTimeout(20)));

            test(compObj1.ice_invocationTimeout(20).equals(compObj1.ice_invocationTimeout(20)));
            test(!compObj1.ice_invocationTimeout(10).equals(compObj1.ice_invocationTimeout(20)));

            compObj1 = communicator.stringToProxy("foo:" + defaultProtocol + " -h 127.0.0.1 -p 1000");
            compObj2 = communicator.stringToProxy("foo@MyAdapter1");
            test(!compObj1.equals(compObj2));

            const endpts1 =
                  communicator.stringToProxy(`foo:${defaultProtocol} -h 127.0.0.1 -p 10000`).ice_getEndpoints();
            const endpts2 =
                  communicator.stringToProxy(`foo:${defaultProtocol} -h 127.0.0.1 -p 10001`).ice_getEndpoints();
            test(!endpts1[0].equals(endpts2[0]));
            test(endpts1[0].equals(
                communicator.stringToProxy(`foo:${defaultProtocol} -h 127.0.0.1 -p 10000`).ice_getEndpoints()[0]));

            test(compObj1.ice_encodingVersion(Ice.Encoding_1_0).equals(compObj1.ice_encodingVersion(Ice.Encoding_1_0)));
            test(!compObj1.ice_encodingVersion(Ice.Encoding_1_0).equals(
                compObj1.ice_encodingVersion(Ice.Encoding_1_1)));

            const baseConnection = await base.ice_getConnection();
            if(baseConnection !== null)
            {
                const baseConnection2 = await base.ice_connectionId("base2").ice_getConnection();
                compObj1 = compObj1.ice_fixed(baseConnection);
                compObj2 = compObj2.ice_fixed(baseConnection2);
                test(!compObj1.equals(compObj2));
            }
            out.writeLine("ok");

            out.write("testing checked cast... ");
            const cl = await Test.MyClassPrx.checkedCast(base);
            test(cl !== null);
            let derived = await Test.MyDerivedClassPrx.checkedCast(cl);
            test(derived !== null);
            test(cl.equals(base));
            test(derived.equals(base));
            test(cl.equals(derived));
            out.writeLine("ok");

            out.write("testing checked cast with context... ");
            let c = await cl.getContext();
            test(c === null || c.size == 0);
            c = new Map();
            c.set("one", "hello");
            c.set("two", "world");
            const clc = await Test.MyClassPrx.checkedCast(base, undefined, c);
            const c2 = await clc.getContext();
            test(Ice.MapUtil.equals(c, c2));
            out.writeLine("ok");

            out.write("testing ice_fixed... ");
            {
                const connection = await cl.ice_getConnection();
                if(connection !== null)
                {
                    test(!cl.ice_isFixed());
                    test(cl.ice_fixed(connection).ice_isFixed());
                    await cl.ice_fixed(connection).getContext();
                    test(cl.ice_secure(true).ice_fixed(connection).ice_isSecure());
                    test(cl.ice_facet("facet").ice_fixed(connection).ice_getFacet() == "facet");
                    test(cl.ice_oneway().ice_fixed(connection).ice_isOneway());
                    const ctx = new Map();
                    ctx.set("one", "hello");
                    ctx.set("two", "world");
                    test(cl.ice_fixed(connection).ice_getContext().size == 0);
                    test(cl.ice_context(ctx).ice_fixed(connection).ice_getContext().size == 2);
                    test(cl.ice_fixed(connection).ice_getInvocationTimeout() == -1);
                    test(cl.ice_invocationTimeout(10).ice_fixed(connection).ice_getInvocationTimeout() == 10);
                    test(await cl.ice_fixed(connection).ice_getConnection() == connection);
                    test(await cl.ice_fixed(connection).ice_fixed(connection).ice_getConnection() == connection);
                    test(cl.ice_fixed(connection).ice_getTimeout() === undefined);
                    const fixedConnection = await cl.ice_connectionId("ice_fixed").ice_getConnection();
                    test(await cl.ice_fixed(connection).ice_fixed(fixedConnection).ice_getConnection() ==
                         fixedConnection);
                    try
                    {
                        await cl.ice_secure(!connection.getEndpoint().getInfo().secure()).ice_fixed(connection).ice_ping();
                    }
                    catch(ex)
                    {
                        test(ex instanceof Ice.NoEndpointException);
                    }
                    try
                    {
                        await cl.ice_datagram().ice_fixed(connection).ice_ping();
                    }
                    catch(ex)
                    {
                        test(ex instanceof Ice.NoEndpointException);
                    }

                }
                else
                {
                    try
                    {
                        cl.ice_fixed(connection);
                        test(false);
                    }
                    catch(ex)
                    {
                        // Expected with null connection.
                    }
                }
            }
            out.writeLine("ok");

            out.write("testing encoding versioning... ");

            let ref20 = "test -e 2.0:" + this.getTestEndpoint();
            let cl20 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref20));
            try
            {
                await cl20.ice_ping();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnsupportedEncodingException, ex);
            }

            let ref10 = "test -e 1.0:" + this.getTestEndpoint();
            let cl10 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref10));

            await cl10.ice_ping();
            await cl10.ice_encodingVersion(Ice.Encoding_1_0).ice_ping();
            await cl.ice_encodingVersion(Ice.Encoding_1_0).ice_ping();

            // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
            // call will use the 1.1 encoding
            let ref13 = "test -e 1.3:" + this.getTestEndpoint();
            let cl13 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref13));
            await cl13.ice_ping();

            // TODO port ice_invoke test
            out.writeLine("ok");

            out.write("testing protocol versioning... ");
            ref20 = "test -p 2.0:" + this.getTestEndpoint();
            cl20 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref20));
            try
            {
                await cl20.ice_ping();
                test(false);
            }
            catch(ex)
            {
                // Server 2.0 proxy doesn't support 1.0 version.
                test(ex instanceof Ice.UnsupportedProtocolException, ex);
            }

            ref10 = "test -p 1.0:" + this.getTestEndpoint();
            cl10 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref10));
            await cl10.ice_ping();

            // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
            // call will use the 1.1 protocol
            ref13 = "test -p 1.3:" + this.getTestEndpoint();
            cl13 = Test.MyClassPrx.uncheckedCast(communicator.stringToProxy(ref13));
            await cl13.ice_ping();
            out.writeLine("ok");

            out.write("testing opaque endpoints... ");

            try
            {
                // Invalid -x option
                communicator.stringToProxy("id:opaque -t 99 -v abc -x abc");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Missing -t and -v
                communicator.stringToProxy("id:opaque");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Repeated -t
                communicator.stringToProxy("id:opaque -t 1 -t 1 -v abc");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Repeated -v
                communicator.stringToProxy("id:opaque -t 1 -v abc -v abc");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Missing -t
                communicator.stringToProxy("id:opaque -v abc");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Missing -v
                communicator.stringToProxy("id:opaque -t 1");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Missing arg for -t
                communicator.stringToProxy("id:opaque -t -v abc");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Missing arg for -v
                communicator.stringToProxy("id:opaque -t 1 -v");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Not a number for -t
                communicator.stringToProxy("id:opaque -t x -v abc");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // < 0 for -t
                communicator.stringToProxy("id:opaque -t -1 -v abc");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            try
            {
                // Invalid char for -v
                communicator.stringToProxy("id:opaque -t 99 -v x?c");
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.EndpointParseException, ex);
            }

            // Legal TCP endpoint expressed as opaque endpoint
            let p1 = communicator.stringToProxy("test -e 1.1:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
            let pstr = communicator.proxyToString(p1);
            test(pstr === "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

            // Legal WS endpoint expressed as opaque endpoint
            p1 = communicator.stringToProxy("test -e 1.1:opaque -t 4 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAAA=");
            pstr = communicator.proxyToString(p1);
            test(pstr === "test -t -e 1.1:ws -h 127.0.0.1 -p 12010 -t 10000");

            // Opaque endpoint encoded with 1.1 encoding.

            let p2 = communicator.stringToProxy("test:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
            test(communicator.proxyToString(p2) === "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

            p2 = communicator.stringToProxy("test:opaque -e 1.1 -t 4 -v CTEyNy4wLjAuMeouAAAQJwAAAAA=");
            test(communicator.proxyToString(p2) === "test -t -e 1.1:ws -h 127.0.0.1 -p 12010 -t 10000");

            if(communicator.getProperties().getPropertyAsInt("Ice.IPv6") === 0)
            {
                const ref = "test:" + this.getTestEndpoint();

                const ssl = communicator.getProperties().getProperty("Ice.Default.Protocol") === "ssl";
                // TODO: p1 contains 127.0.0.1 - OK to invoke?
                //   if(!ssl)
                //   {
                //       p1.ice_encodingVersion(Ice.Util.Encoding_1_0).ice_ping();
                //   }

                // Two legal TCP endpoints expressed as opaque endpoints
                p1 = communicator.stringToProxy("test -e 1.0:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
                pstr = communicator.proxyToString(p1);
                test(pstr === "test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

                p1 = communicator.stringToProxy("test -e 1.0:opaque -t 4 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAAA=:opaque -t 4 -e 1.0 -v CTEyNy4wLjAuMusuAAAQJwAAAAA=");
                pstr = communicator.proxyToString(p1);
                test(pstr === "test -t -e 1.0:ws -h 127.0.0.1 -p 12010 -t 10000:ws -h 127.0.0.2 -p 12011 -t 10000");

                //
                // Test that an SSL endpoint and a nonsense endpoint get
                // written back out as an opaque endpoint.
                //
                p1 = communicator.stringToProxy("test -e 1.0:opaque -e 1.0 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch");
                pstr = communicator.proxyToString(p1);
                test(pstr === "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");

                //
                // Try to invoke on the SSL endpoint to verify that we get a
                // NoEndpointException (or ConnectFailedException when
                // running with SSL).
                //
                try
                {
                    await p1.ice_encodingVersion(Ice.Encoding_1_0).ice_ping();
                    test(false);
                }
                catch(ex)
                {
                    if(ex instanceof Ice.NoEndpointException)
                    {
                        test(!ssl);
                    }
                    else if(ex instanceof Ice.ConnectFailedException)
                    {
                        test(ssl);
                    }
                    else
                    {
                        throw ex;
                    }
                }
                //
                // Test that the proxy with an SSL endpoint and a nonsense
                // endpoint (which the server doesn't understand either) can
                // be sent over the wire and returned by the server without
                // losing the opaque endpoints.
                //
                derived = Test.MyDerivedClassPrx.uncheckedCast(
                    communicator.stringToProxy("test -e 1.0:" + this.getTestEndpoint()));
                p2 = await derived.echo(p1);

                pstr = communicator.proxyToString(p2);
                test(pstr === "test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");

                let p = communicator.stringToProxy("test:" + this.getTestEndpoint());
                if(defaultProtocol === "tcp")
                {
                    test(p.ice_getEndpoints()[0].getInfo() instanceof Ice.TCPEndpointInfo);
                }
                else if(defaultProtocol === "ws" || defaultProtocol === "wss")
                {
                    test(p.ice_getEndpoints()[0].getInfo() instanceof Ice.WSEndpointInfo);
                }

                let con = await p.ice_getConnection();
                if(defaultProtocol === "tcp")
                {
                    test(con.getInfo() instanceof Ice.TCPConnectionInfo);
                }
                else if(defaultProtocol === "ws" || defaultProtocol === "wss")
                {
                    test(con.getInfo() instanceof Ice.WSConnectionInfo);
                }

                //
                // Ensure that non connectable endpoints are skipped.
                //
                p = (typeof window === 'undefined' && typeof WorkerGlobalScope === 'undefined') ?
                    communicator.stringToProxy("test:" + this.getTestEndpoint("ws") + ":" + this.getTestEndpoint()) :
                    communicator.stringToProxy("test:" + this.getTestEndpoint("tcp") + ":" + this.getTestEndpoint());

                p = p.ice_endpointSelection(Ice.EndpointSelectionType.Ordered);
                await p.ice_ping();

                out.writeLine("ok");

                out.write("testing proxyToString... ");
                b1 = communicator.stringToProxy(ref);
                let b2 = communicator.stringToProxy(communicator.proxyToString(b1));
                test(b1.equals(b2));

                con = await b1.ice_getConnection();
                b2 = con.createProxy(Ice.stringToIdentity("fixed"));
                const str = communicator.proxyToString(b2);
                test(b2.toString() === str);
                const str2 = b1.ice_identity(b2.ice_getIdentity()).ice_secure(b2.ice_isSecure()).toString();

                // Verify that the stringified fixed proxy is the same as a regular stringified proxy
                // but without endpoints
                test(str2.startsWith(str));
                test(str2.charAt(str.length) === ':');

                out.writeLine("ok");

                if(defaultProtocol === "ws" || defaultProtocol === "wss")
                {
                    out.write("testing ping invalid WS proxies... ");
                    //
                    // Invocation in a WS or WSS proxy that has not hostname set
                    // will fail creating the WebSocket object.
                    //
                    const communicator2 = Ice.initialize();
                    const invalid = communicator2.stringToProxy("test:" + this.getTestEndpoint());
                    try
                    {
                        await invalid.ice_ping();
                        test(false);
                    }
                    catch(ex)
                    {
                        // expected
                    }
                    finally
                    {
                        await communicator2.destroy();
                    }
                    out.writeLine("ok");
                }

                out.write("testing communicator shutdown/destroy... ");
                {
                    const c = Ice.initialize();
                    c.shutdown();
                    test(c.isShutdown());
                    await c.waitForShutdown();
                    await c.destroy();
                    c.shutdown();
                    test(c.isShutdown());
                    await c.waitForShutdown();
                    await c.destroy();
                }
                out.writeLine("ok");

                derived = Test.MyDerivedClassPrx.uncheckedCast(
                    communicator.stringToProxy("test:" + this.getTestEndpoint()));
                await derived.shutdown();
            }
        }

        async run(args)
        {
            let communicator;
            try
            {
                [communicator] = this.initialize(args);
                await this.allTests();
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Client = Client;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
