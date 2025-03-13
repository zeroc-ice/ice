// Copyright (c) ZeroC, Inc.

namespace Ice.proxy;

public class AllTests : global::Test.AllTests
{
    public static async Task<Test.MyClassPrx> allTests(global::Test.TestHelper helper)
    {
        var communicator = helper.communicator();
        var output = helper.getWriter();
        output.Write("testing stringToProxy... ");
        output.Flush();
        string rf = "test:" + helper.getTestEndpoint(0);
        var baseProxy = communicator.stringToProxy(rf);
        test(baseProxy != null);

        var b1 = communicator.stringToProxy("test");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getAdapterId().Length == 0 && b1.ice_getFacet().Length == 0);
        b1 = communicator.stringToProxy("test ");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet().Length == 0);
        b1 = communicator.stringToProxy(" test ");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet().Length == 0);
        b1 = communicator.stringToProxy(" test");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet().Length == 0);
        b1 = communicator.stringToProxy("'test -f facet'");
        test(b1.ice_getIdentity().name == "test -f facet" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet().Length == 0);
        try
        {
            b1 = communicator.stringToProxy("\"test -f facet'");
            test(false);
        }
        catch (ParseException)
        {
        }
        b1 = communicator.stringToProxy("\"test -f facet\"");
        test(b1.ice_getIdentity().name == "test -f facet" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet().Length == 0);
        b1 = communicator.stringToProxy("\"test -f facet@test\"");
        test(b1.ice_getIdentity().name == "test -f facet@test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet().Length == 0);
        b1 = communicator.stringToProxy("\"test -f facet@test @test\"");
        test(b1.ice_getIdentity().name == "test -f facet@test @test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet().Length == 0);
        try
        {
            b1 = communicator.stringToProxy("test test");
            test(false);
        }
        catch (ParseException)
        {
        }
        b1 = communicator.stringToProxy("test\\040test");
        test(b1.ice_getIdentity().name == "test test" && b1.ice_getIdentity().category.Length == 0);
        try
        {
            b1 = communicator.stringToProxy("test\\777");
            test(false);
        }
        catch (ParseException)
        {
        }
        b1 = communicator.stringToProxy("test\\40test");
        test(b1.ice_getIdentity().name == "test test");

        // Test some octal and hex corner cases.
        b1 = communicator.stringToProxy("test\\4test");
        test(b1.ice_getIdentity().name == "test\u0004test");
        b1 = communicator.stringToProxy("test\\04test");
        test(b1.ice_getIdentity().name == "test\u0004test");
        b1 = communicator.stringToProxy("test\\004test");
        test(b1.ice_getIdentity().name == "test\u0004test");
        b1 = communicator.stringToProxy("test\\1114test");
        test(b1.ice_getIdentity().name == "test\u00494test");

        b1 = communicator.stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test");
        test(b1.ice_getIdentity().name.Equals("test\b\f\n\r\t\'\"\\test") && b1.ice_getIdentity().category.Length == 0);

        b1 = communicator.stringToProxy("category/test");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" &&
             b1.ice_getAdapterId().Length == 0);

        b1 = communicator.stringToProxy("test:tcp --sourceAddress \"::1\"");
        test(b1.Equals(communicator.stringToProxy(b1.ToString())));

        b1 = communicator.stringToProxy("test:udp --sourceAddress \"::1\" --interface \"0:0:0:0:0:0:0:1%lo\"");
        test(b1.Equals(communicator.stringToProxy(b1.ToString())));

        b1 = communicator.stringToProxy("");
        test(b1 == null);
        b1 = communicator.stringToProxy("\"\"");
        test(b1 == null);
        try
        {
            b1 = communicator.stringToProxy("\"\" test"); // Invalid trailing characters.
            test(false);
        }
        catch (ParseException)
        {
        }
        try
        {
            b1 = communicator.stringToProxy("test:"); // Missing endpoint.
            test(false);
        }
        catch (ParseException)
        {
        }

        b1 = communicator.stringToProxy("test@adapter");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getAdapterId() == "adapter");
        try
        {
            b1 = communicator.stringToProxy("id@adapter test");
            test(false);
        }
        catch (ParseException)
        {
        }
        b1 = communicator.stringToProxy("category/test@adapter");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" &&
             b1.ice_getAdapterId() == "adapter");
        b1 = communicator.stringToProxy("category/test@adapter:tcp");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" &&
             b1.ice_getAdapterId() == "adapter:tcp");
        b1 = communicator.stringToProxy("'category 1/test'@adapter");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category 1" &&
             b1.ice_getAdapterId() == "adapter");
        b1 = communicator.stringToProxy("'category/test 1'@adapter");
        test(b1.ice_getIdentity().name == "test 1" && b1.ice_getIdentity().category == "category" &&
             b1.ice_getAdapterId() == "adapter");
        b1 = communicator.stringToProxy("'category/test'@'adapter 1'");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category" &&
             b1.ice_getAdapterId() == "adapter 1");
        b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@adapter");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category /test@foo" &&
             b1.ice_getAdapterId() == "adapter");
        b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category == "category /test@foo" &&
             b1.ice_getAdapterId() == "adapter:tcp");

        b1 = communicator.stringToProxy("id -f facet");
        test(b1.ice_getIdentity().name == "id" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet");
        b1 = communicator.stringToProxy("id -f 'facet x'");
        test(b1.ice_getIdentity().name == "id" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet x");
        b1 = communicator.stringToProxy("id -f \"facet x\"");
        test(b1.ice_getIdentity().name == "id" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet x");
        try
        {
            b1 = communicator.stringToProxy("id -f \"facet x");
            test(false);
        }
        catch (ParseException)
        {
        }
        try
        {
            b1 = communicator.stringToProxy("id -f \'facet x");
            test(false);
        }
        catch (ParseException)
        {
        }
        b1 = communicator.stringToProxy("test -f facet:tcp");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet" && b1.ice_getAdapterId().Length == 0);
        b1 = communicator.stringToProxy("test -f \"facet:tcp\"");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet:tcp" && b1.ice_getAdapterId().Length == 0);
        b1 = communicator.stringToProxy("test -f facet@test");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet" && b1.ice_getAdapterId() == "test");
        b1 = communicator.stringToProxy("test -f 'facet@test'");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet@test" && b1.ice_getAdapterId().Length == 0);
        b1 = communicator.stringToProxy("test -f 'facet@test'@test");
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getFacet() == "facet@test" && b1.ice_getAdapterId() == "test");
        try
        {
            b1 = communicator.stringToProxy("test -f facet@test @test");
            test(false);
        }
        catch (ParseException)
        {
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

        test(b1.ice_getEncodingVersion().Equals(Ice.Util.currentEncoding));

        b1 = communicator.stringToProxy("test -e 1.0");
        test(b1.ice_getEncodingVersion().major == 1 && b1.ice_getEncodingVersion().minor == 0);

        b1 = communicator.stringToProxy("test -e 6.5");
        test(b1.ice_getEncodingVersion().major == 6 && b1.ice_getEncodingVersion().minor == 5);

        b1 = communicator.stringToProxy("test -p 1.0 -e 1.0");
        test(b1.ToString() == "test -e 1.0");

        b1 = communicator.stringToProxy("test -p 6.5 -e 1.0");
        test(b1.ToString() == "test -p 6.5 -e 1.0");

        try
        {
            communicator.stringToProxy("test:tcp@adapterId");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            communicator.stringToProxy("test: :tcp");
            test(false);
        }
        catch (ParseException)
        {
        }

        //
        // Test invalid endpoint syntax
        //
        try
        {
            communicator.createObjectAdapterWithEndpoints("BadAdapter", " : ");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            communicator.createObjectAdapterWithEndpoints("BadAdapter", "tcp: ");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            communicator.createObjectAdapterWithEndpoints("BadAdapter", ":tcp");
            test(false);
        }
        catch (ParseException)
        {
        }

        //
        // Test for bug ICE-5543: escaped escapes in stringToIdentity
        //
        var id = new Ice.Identity("test", ",X2QNUAzSBcJ_e$AV;E\\");
        var id2 = Ice.Util.stringToIdentity(communicator.identityToString(id));
        test(id.Equals(id2));

        id = new Ice.Identity("test", ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\");
        id2 = Ice.Util.stringToIdentity(communicator.identityToString(id));
        test(id.Equals(id2));

        id = new Ice.Identity("/test", "cat/");
        string idStr = communicator.identityToString(id);
        test(idStr == "cat\\//\\/test");
        id2 = Ice.Util.stringToIdentity(idStr);
        test(id.Equals(id2));

        // Input string with various pitfalls
        id = Ice.Util.stringToIdentity("\\342\\x82\\254\\60\\x9\\60\\");
        test(id.name == "€0\t0\\" && id.category.Length == 0);

        try
        {
            // Illegal character < 32
            id = Ice.Util.stringToIdentity("xx\01FooBar");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Illegal surrogate
            id = Ice.Util.stringToIdentity("xx\\ud911");
            test(false);
        }
        catch (ParseException)
        {
        }

        // Testing bytes 127(\x7F, \177) and €
        id = new Ice.Identity("test", "\x7f€");

        idStr = Ice.Util.identityToString(id, Ice.ToStringMode.Unicode);
        test(idStr == "\\u007f€/test");
        id2 = Ice.Util.stringToIdentity(idStr);
        test(id.Equals(id2));
        test(Ice.Util.identityToString(id) == idStr);

        idStr = Ice.Util.identityToString(id, Ice.ToStringMode.ASCII);
        test(idStr == "\\u007f\\u20ac/test");
        id2 = Ice.Util.stringToIdentity(idStr);
        test(id.Equals(id2));

        idStr = Ice.Util.identityToString(id, Ice.ToStringMode.Compat);
        test(idStr == "\\177\\342\\202\\254/test");
        id2 = Ice.Util.stringToIdentity(idStr);
        test(id.Equals(id2));

        id2 = Ice.Util.stringToIdentity(communicator.identityToString(id));
        test(id.Equals(id2));

        // More unicode character
        id = new Ice.Identity("banana \x0E-\ud83c\udf4c\u20ac\u00a2\u0024", "greek \ud800\udd6a");

        idStr = Ice.Util.identityToString(id, Ice.ToStringMode.Unicode);
        test(idStr == "greek \ud800\udd6a/banana \\u000e-\ud83c\udf4c\u20ac\u00a2$");
        id2 = Ice.Util.stringToIdentity(idStr);
        test(id.Equals(id2));

        idStr = Ice.Util.identityToString(id, Ice.ToStringMode.ASCII);
        test(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$");
        id2 = Ice.Util.stringToIdentity(idStr);
        test(id.Equals(id2));

        idStr = Ice.Util.identityToString(id, Ice.ToStringMode.Compat);
        id2 = Ice.Util.stringToIdentity(idStr);
        test(idStr == "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$");
        test(id.Equals(id2));

        output.WriteLine("ok");

        output.Write("testing proxyToString... ");
        output.Flush();
        b1 = communicator.stringToProxy(rf);
        var b2 = communicator.stringToProxy(communicator.proxyToString(b1));
        test(b1.Equals(b2));

        if (b1.ice_getConnection() != null) // not colloc-optimized target
        {
            b2 = b1.ice_getConnection().createProxy(Ice.Util.stringToIdentity("fixed"));
            string str = communicator.proxyToString(b2);
            test(b2.ToString() == str);
            string str2 = b1.ice_identity(b2.ice_getIdentity()).ice_secure(b2.ice_isSecure()).ToString();

            // Verify that the stringified fixed proxy is the same as a regular stringified proxy
            // but without endpoints
            test(str2.StartsWith(str));
            test(str2[str.Length] == ':');
        }
        output.WriteLine("ok");

        output.Write("testing propertyToProxy... ");
        output.Flush();
        var prop = communicator.getProperties();
        string propertyPrefix = "Foo.Proxy";
        prop.setProperty(propertyPrefix, "test:" + helper.getTestEndpoint(0));
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getIdentity().name == "test" && b1.ice_getIdentity().category.Length == 0 &&
             b1.ice_getAdapterId().Length == 0 && b1.ice_getFacet().Length == 0);

        string property;

        property = propertyPrefix + ".Locator";
        test(b1.ice_getLocator() == null);
        prop.setProperty(property, "locator:default -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocator() != null && b1.ice_getLocator().ice_getIdentity().name == "locator");
        prop.setProperty(property, "");
        property = propertyPrefix + ".LocatorCacheTimeout";
        test(b1.ice_getLocatorCacheTimeout() == TimeSpan.FromSeconds(-1));
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocatorCacheTimeout() == TimeSpan.FromSeconds(1));
        prop.setProperty(property, "");

        // Now retest with an indirect proxy.
        prop.setProperty(propertyPrefix, "test");
        property = propertyPrefix + ".Locator";
        prop.setProperty(property, "locator:default -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocator() != null && b1.ice_getLocator().ice_getIdentity().name == "locator");
        prop.setProperty(property, "");

        property = propertyPrefix + ".LocatorCacheTimeout";
        test(b1.ice_getLocatorCacheTimeout() == TimeSpan.FromSeconds(-1));
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocatorCacheTimeout() == TimeSpan.FromSeconds(1));
        prop.setProperty(property, "");

        // This cannot be tested so easily because the property is cached
        // on communicator initialization.
        //
        // prop.setProperty("Ice.Default.LocatorCacheTimeout", "60");
        // b1 = communicator.propertyToProxy(propertyPrefix);
        // test(b1.ice_getLocatorCacheTimeout() == 60);
        // prop.setProperty("Ice.Default.LocatorCacheTimeout", "");

        prop.setProperty(propertyPrefix, "test:" + helper.getTestEndpoint(0));

        property = propertyPrefix + ".Router";
        test(b1.ice_getRouter() == null);
        prop.setProperty(property, "router:default -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getRouter() != null && b1.ice_getRouter().ice_getIdentity().name == "router");
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
        test(b1.ice_getInvocationTimeout() == TimeSpan.FromMilliseconds(-1));
        prop.setProperty(property, "1000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getInvocationTimeout() == TimeSpan.FromSeconds(1));
        prop.setProperty(property, "");

        property = propertyPrefix + ".EndpointSelection";
        test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);
        prop.setProperty(property, "Random");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);
        prop.setProperty(property, "Ordered");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
        prop.setProperty(property, "");

        property = propertyPrefix + ".CollocationOptimized";
        test(b1.ice_isCollocationOptimized());
        prop.setProperty(property, "0");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(!b1.ice_isCollocationOptimized());
        prop.setProperty(property, "");

        property = propertyPrefix + ".Context.c1";
        test(!b1.ice_getContext().ContainsKey("c1"));
        prop.setProperty(property, "TEST");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getContext()["c1"] == "TEST");

        property = propertyPrefix + ".Context.c2";
        test(!b1.ice_getContext().ContainsKey("c2"));
        prop.setProperty(property, "TEST");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getContext()["c2"] == "TEST");

        prop.setProperty(propertyPrefix + ".Context.c1", "");
        prop.setProperty(propertyPrefix + ".Context.c2", "");

        output.WriteLine("ok");

        output.Write("testing proxyToProperty... ");
        output.Flush();

        b1 = communicator.stringToProxy("test");
        b1 = b1.ice_collocationOptimized(true);
        b1 = b1.ice_connectionCached(true);
        b1 = b1.ice_preferSecure(false);
        b1 = b1.ice_endpointSelection(Ice.EndpointSelectionType.Ordered);
        b1 = b1.ice_locatorCacheTimeout(100);
        b1 = b1.ice_invocationTimeout(1234);
        b1 = b1.ice_encodingVersion(new Ice.EncodingVersion(1, 0));

        Ice.ObjectPrx router = communicator.stringToProxy("router");
        router = router.ice_collocationOptimized(false);
        router = router.ice_connectionCached(true);
        router = router.ice_preferSecure(true);
        router = router.ice_endpointSelection(Ice.EndpointSelectionType.Random);
        router = router.ice_locatorCacheTimeout(200);
        router = router.ice_invocationTimeout(1500);

        Ice.ObjectPrx locator = communicator.stringToProxy("locator");
        locator = locator.ice_collocationOptimized(true);
        locator = locator.ice_connectionCached(false);
        locator = locator.ice_preferSecure(true);
        locator = locator.ice_endpointSelection(Ice.EndpointSelectionType.Random);
        locator = locator.ice_locatorCacheTimeout(300);
        locator = locator.ice_invocationTimeout(1500);

        locator = locator.ice_router(Ice.RouterPrxHelper.uncheckedCast(router));
        b1 = b1.ice_locator(Ice.LocatorPrxHelper.uncheckedCast(locator));

        Dictionary<string, string> proxyProps = communicator.proxyToProperty(b1, "Test");
        test(proxyProps.Count == 21);

        test(proxyProps["Test"] == "test -e 1.0");
        test(proxyProps["Test.CollocationOptimized"] == "1");
        test(proxyProps["Test.ConnectionCached"] == "1");
        test(proxyProps["Test.PreferSecure"] == "0");
        test(proxyProps["Test.EndpointSelection"] == "Ordered");
        test(proxyProps["Test.LocatorCacheTimeout"] == "100");
        test(proxyProps["Test.InvocationTimeout"] == "1234");

        test(proxyProps["Test.Locator"] == "locator");
        // Locator collocation optimization is always disabled.
        // test(proxyProps["Test.Locator.CollocationOptimized"] == "1");
        test(proxyProps["Test.Locator.ConnectionCached"] == "0");
        test(proxyProps["Test.Locator.PreferSecure"] == "1");
        test(proxyProps["Test.Locator.EndpointSelection"] == "Random");
        test(proxyProps["Test.Locator.LocatorCacheTimeout"] == "300");
        test(proxyProps["Test.Locator.InvocationTimeout"] == "1500");

        test(proxyProps["Test.Locator.Router"] == "router");
        test(proxyProps["Test.Locator.Router.CollocationOptimized"] == "0");
        test(proxyProps["Test.Locator.Router.ConnectionCached"] == "1");
        test(proxyProps["Test.Locator.Router.PreferSecure"] == "1");
        test(proxyProps["Test.Locator.Router.EndpointSelection"] == "Random");
        test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200");
        test(proxyProps["Test.Locator.Router.InvocationTimeout"] == "1500");

        output.WriteLine("ok");

        output.Write("testing ice_getCommunicator... ");
        output.Flush();
        test(baseProxy.ice_getCommunicator() == communicator);
        output.WriteLine("ok");

        output.Write("testing proxy methods... ");

        test(communicator.identityToString(
            baseProxy.ice_identity(Util.stringToIdentity("other")).ice_getIdentity()) == "other");
        test(baseProxy.ice_facet("facet").ice_getFacet() == "facet");
        test(baseProxy.ice_adapterId("id").ice_getAdapterId() == "id");
        test(baseProxy.ice_twoway().ice_isTwoway());
        test(baseProxy.ice_oneway().ice_isOneway());
        test(baseProxy.ice_batchOneway().ice_isBatchOneway());
        test(baseProxy.ice_datagram().ice_isDatagram());
        test(baseProxy.ice_batchDatagram().ice_isBatchDatagram());
        test(baseProxy.ice_secure(true).ice_isSecure());
        test(!baseProxy.ice_secure(false).ice_isSecure());
        test(baseProxy.ice_collocationOptimized(true).ice_isCollocationOptimized());
        test(!baseProxy.ice_collocationOptimized(false).ice_isCollocationOptimized());
        test(baseProxy.ice_preferSecure(true).ice_isPreferSecure());
        test(!baseProxy.ice_preferSecure(false).ice_isPreferSecure());

        test(baseProxy.ice_invocationTimeout(0).ice_getInvocationTimeout() == TimeSpan.Zero);
        test(baseProxy.ice_invocationTimeout(-1).ice_getInvocationTimeout() == TimeSpan.FromMilliseconds(-1));
        test(baseProxy.ice_invocationTimeout(-2).ice_getInvocationTimeout() == TimeSpan.FromMilliseconds(-2));

        test(baseProxy.ice_locatorCacheTimeout(0).ice_getLocatorCacheTimeout() == TimeSpan.Zero);
        test(baseProxy.ice_locatorCacheTimeout(-1).ice_getLocatorCacheTimeout() == TimeSpan.FromSeconds(-1));
        test(baseProxy.ice_locatorCacheTimeout(-2).ice_getLocatorCacheTimeout() == TimeSpan.FromSeconds(-2));

        output.WriteLine("ok");

        output.Write("testing proxy comparison... ");
        output.Flush();

        test(communicator.stringToProxy("foo").Equals(communicator.stringToProxy("foo")));
        test(!communicator.stringToProxy("foo").Equals(communicator.stringToProxy("foo2")));

        Ice.ObjectPrx compObj = communicator.stringToProxy("foo");

        test(compObj.ice_facet("facet").Equals(compObj.ice_facet("facet")));
        test(!compObj.ice_facet("facet").Equals(compObj.ice_facet("facet1")));

        test(compObj.ice_oneway().Equals(compObj.ice_oneway()));
        test(!compObj.ice_oneway().Equals(compObj.ice_twoway()));

        test(compObj.ice_secure(true).Equals(compObj.ice_secure(true)));
        test(!compObj.ice_secure(false).Equals(compObj.ice_secure(true)));

        test(compObj.ice_collocationOptimized(true).Equals(compObj.ice_collocationOptimized(true)));
        test(!compObj.ice_collocationOptimized(false).Equals(compObj.ice_collocationOptimized(true)));

        test(compObj.ice_connectionCached(true).Equals(compObj.ice_connectionCached(true)));
        test(!compObj.ice_connectionCached(false).Equals(compObj.ice_connectionCached(true)));

        test(compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).Equals(
                 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random)));
        test(!compObj.ice_endpointSelection(Ice.EndpointSelectionType.Random).Equals(
                 compObj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered)));

        test(compObj.ice_connectionId("id2").Equals(compObj.ice_connectionId("id2")));
        test(!compObj.ice_connectionId("id1").Equals(compObj.ice_connectionId("id2")));
        test(compObj.ice_connectionId("id1").ice_getConnectionId() == "id1");
        test(compObj.ice_connectionId("id2").ice_getConnectionId() == "id2");

        test(compObj.ice_compress(true).Equals(compObj.ice_compress(true)));
        test(!compObj.ice_compress(false).Equals(compObj.ice_compress(true)));

        test(!compObj.ice_getCompress().HasValue);
        test(compObj.ice_compress(true).ice_getCompress().Value == true);
        test(compObj.ice_compress(false).ice_getCompress().Value == false);

        Ice.LocatorPrx loc1 = Ice.LocatorPrxHelper.uncheckedCast(communicator.stringToProxy("loc1:default -p 10000"));
        Ice.LocatorPrx loc2 = Ice.LocatorPrxHelper.uncheckedCast(communicator.stringToProxy("loc2:default -p 10000"));
        test(compObj.ice_locator(null).Equals(compObj.ice_locator(null)));
        test(compObj.ice_locator(loc1).Equals(compObj.ice_locator(loc1)));
        test(!compObj.ice_locator(loc1).Equals(compObj.ice_locator(null)));
        test(!compObj.ice_locator(null).Equals(compObj.ice_locator(loc2)));
        test(!compObj.ice_locator(loc1).Equals(compObj.ice_locator(loc2)));

        Ice.RouterPrx rtr1 = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("rtr1:default -p 10000"));
        Ice.RouterPrx rtr2 = Ice.RouterPrxHelper.uncheckedCast(communicator.stringToProxy("rtr2:default -p 10000"));
        test(compObj.ice_router(null).Equals(compObj.ice_router(null)));
        test(compObj.ice_router(rtr1).Equals(compObj.ice_router(rtr1)));
        test(!compObj.ice_router(rtr1).Equals(compObj.ice_router(null)));
        test(!compObj.ice_router(null).Equals(compObj.ice_router(rtr2)));
        test(!compObj.ice_router(rtr1).Equals(compObj.ice_router(rtr2)));

        var ctx1 = new Dictionary<string, string>
        {
            ["ctx1"] = "v1"
        };
        var ctx2 = new Dictionary<string, string>
        {
            ["ctx2"] = "v2"
        };
        test(compObj.ice_context(null).Equals(compObj.ice_context(null)));
        test(compObj.ice_context(ctx1).Equals(compObj.ice_context(ctx1)));
        test(!compObj.ice_context(ctx1).Equals(compObj.ice_context(null)));
        test(!compObj.ice_context(null).Equals(compObj.ice_context(ctx2)));
        test(!compObj.ice_context(ctx1).Equals(compObj.ice_context(ctx2)));

        test(compObj.ice_preferSecure(true).Equals(compObj.ice_preferSecure(true)));
        test(!compObj.ice_preferSecure(true).Equals(compObj.ice_preferSecure(false)));

        Ice.ObjectPrx compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
        Ice.ObjectPrx compObj2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
        test(!compObj1.Equals(compObj2));

        compObj1 = communicator.stringToProxy("foo@MyAdapter1");
        compObj2 = communicator.stringToProxy("foo@MyAdapter2");
        test(!compObj1.Equals(compObj2));

        test(compObj1.ice_locatorCacheTimeout(20).Equals(compObj1.ice_locatorCacheTimeout(20)));
        test(!compObj1.ice_locatorCacheTimeout(10).Equals(compObj1.ice_locatorCacheTimeout(20)));

        test(compObj1.ice_invocationTimeout(20).Equals(compObj1.ice_invocationTimeout(20)));
        test(!compObj1.ice_invocationTimeout(10).Equals(compObj1.ice_invocationTimeout(20)));

        compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
        compObj2 = communicator.stringToProxy("foo@MyAdapter1");
        test(!compObj1.Equals(compObj2));

        Ice.Endpoint[] endpts1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000").ice_getEndpoints();
        Ice.Endpoint[] endpts2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001").ice_getEndpoints();
        test(!endpts1[0].Equals(endpts2[0]));
        test(endpts1[0].Equals(communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000").ice_getEndpoints()[0]));

        Ice.Connection baseConnection = baseProxy.ice_getConnection();
        if (baseConnection != null)
        {
            Ice.Connection baseConnection2 = baseProxy.ice_connectionId("base2").ice_getConnection();
            compObj1 = compObj1.ice_fixed(baseConnection);
            compObj2 = compObj2.ice_fixed(baseConnection2);
            test(!compObj1.Equals(compObj2));
        }
        output.WriteLine("ok");

        output.Write("testing checked cast... ");
        output.Flush();
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        test(cl != null);
        Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(cl);
        test(derived != null);
        test(cl.Equals(baseProxy));
        test(derived.Equals(baseProxy));
        test(cl.Equals(derived));
        try
        {
            Test.MyDerivedClassPrxHelper.checkedCast(cl, "facet");
            test(false);
        }
        catch (Ice.FacetNotExistException)
        {
            // expected
        }
        output.WriteLine("ok");

        output.Write("testing checked cast with context... ");
        output.Flush();

        Dictionary<string, string> c = cl.getContext();
        test(c == null || c.Count == 0);

        c = new Dictionary<string, string>
        {
            ["one"] = "hello",
            ["two"] = "world"
        };
        cl = Test.MyClassPrxHelper.checkedCast(baseProxy, c);
        Dictionary<string, string> c2 = cl.getContext();
        test(Internal.DictionaryExtensions.DictionaryEqual(c, c2));
        output.WriteLine("ok");

        output.Write("testing ice_fixed... ");
        output.Flush();
        {
            Connection connection = cl.ice_getConnection();
            if (connection != null)
            {
                test(!cl.ice_isFixed());
                var prx = (Test.MyClassPrx)cl.ice_fixed(connection);
                test(prx.ice_isFixed());
                prx.ice_ping();
                test(cl.ice_secure(true).ice_fixed(connection).ice_isSecure());
                test(cl.ice_facet("facet").ice_fixed(connection).ice_getFacet() == "facet");
                test(cl.ice_oneway().ice_fixed(connection).ice_isOneway());
                var ctx = new Dictionary<string, string>
                {
                    ["one"] = "hello",
                    ["two"] = "world"
                };
                test(cl.ice_fixed(connection).ice_getContext().Count == 0);
                test(cl.ice_context(ctx).ice_fixed(connection).ice_getContext().Count == 2);
                test(cl.ice_fixed(connection).ice_getInvocationTimeout() == TimeSpan.FromMilliseconds(-1));
                test(cl.ice_invocationTimeout(10).ice_fixed(connection).ice_getInvocationTimeout() == TimeSpan.FromMilliseconds(10));
                test(cl.ice_fixed(connection).ice_getConnection() == connection);
                test(cl.ice_fixed(connection).ice_fixed(connection).ice_getConnection() == connection);
                test(cl.ice_compress(true).ice_fixed(connection).ice_getCompress().Value);
                Ice.Connection fixedConnection = cl.ice_connectionId("ice_fixed").ice_getConnection();
                test(cl.ice_fixed(connection).ice_fixed(fixedConnection).ice_getConnection() == fixedConnection);
                try
                {
                    cl.ice_secure(!connection.getEndpoint().getInfo().secure()).ice_fixed(connection).ice_ping();
                }
                catch (Ice.NoEndpointException)
                {
                }
                try
                {
                    cl.ice_datagram().ice_fixed(connection).ice_ping();
                }
                catch (Ice.NoEndpointException)
                {
                }
            }
            else
            {
                try
                {
                    cl.ice_fixed(connection);
                    test(false);
                }
                catch (ArgumentException)
                {
                    // Expected with null connection.
                }
            }
        }
        output.WriteLine("ok");

        output.Write("testing encoding versioning... ");
        output.Flush();
        string ref20 = "test -e 2.0:" + helper.getTestEndpoint(0);
        Test.MyClassPrx cl20 = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy(ref20));
        try
        {
            cl20.ice_ping();
            test(false);
        }
        catch (MarshalException)
        {
            // Cannot marshal with the 2.0 encoding version.
        }

        string ref10 = "test -e 1.0:" + helper.getTestEndpoint(0);
        Test.MyClassPrx cl10 = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy(ref10));
        cl10.ice_ping();
        cl10.ice_encodingVersion(Ice.Util.Encoding_1_0).ice_ping();
        cl.ice_encodingVersion(Ice.Util.Encoding_1_0).ice_ping();

        // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
        // call will use the 1.1 encoding
        string ref13 = "test -e 1.3:" + helper.getTestEndpoint(0);
        Test.MyClassPrx cl13 = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy(ref13));
        cl13.ice_ping();
        await cl13.ice_pingAsync();

        try
        {
            // Send request with bogus 1.2 encoding.
            var version = new EncodingVersion(1, 2);
            var os = new OutputStream(communicator);
            os.startEncapsulation();
            os.endEncapsulation();
            byte[] inEncaps = os.finished();
            inEncaps[4] = version.major;
            inEncaps[5] = version.minor;
            cl.ice_invoke("ice_ping", OperationMode.Normal, inEncaps, out byte[] outEncaps);
            test(false);
        }
        catch (UnknownLocalException ex)
        {
            test(
                ex.Message.Contains("::Ice::MarshalException") ||
                ex.Message.Contains("Ice.MarshalException"));
        }

        try
        {
            // Send request with bogus 2.0 encoding.
            var version = new EncodingVersion(2, 0);
            var os = new OutputStream(communicator);
            os.startEncapsulation();
            os.endEncapsulation();
            byte[] inEncaps = os.finished();
            inEncaps[4] = version.major;
            inEncaps[5] = version.minor;
            cl.ice_invoke("ice_ping", OperationMode.Normal, inEncaps, out byte[] outEncaps);
            test(false);
        }
        catch (UnknownLocalException ex)
        {
            test(
                ex.Message.Contains("::Ice::MarshalException") ||
                ex.Message.Contains("Ice.MarshalException"));
        }

        output.WriteLine("ok");

        output.Write("testing protocol versioning... ");
        output.Flush();
        ref20 = "test -p 2.0:" + helper.getTestEndpoint(0);
        cl20 = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy(ref20));
        try
        {
            cl20.ice_ping();
            test(false);
        }
        catch (FeatureNotSupportedException)
        {
            // Server 2.0 proxy doesn't support 1.0 version.
        }

        ref10 = "test -p 1.0:" + helper.getTestEndpoint(0);
        cl10 = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy(ref10));
        cl10.ice_ping();

        // 1.3 isn't supported but since a 1.3 proxy supports 1.0, the
        // call will use the 1.0 protocol
        ref13 = "test -p 1.3:" + helper.getTestEndpoint(0);
        cl13 = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy(ref13));
        cl13.ice_ping();
        await cl13.ice_pingAsync();
        output.WriteLine("ok");

        output.Write("testing opaque endpoints... ");
        output.Flush();

        try
        {
            // Invalid -x option
            communicator.stringToProxy("id:opaque -t 99 -v abcd -x abc");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Missing -t and -v
            communicator.stringToProxy("id:opaque");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Repeated -t
            communicator.stringToProxy("id:opaque -t 1 -t 1 -v abcd");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Repeated -v
            communicator.stringToProxy("id:opaque -t 1 -v abcd -v abcd");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Missing -t
            communicator.stringToProxy("id:opaque -v abcd");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Missing -v
            communicator.stringToProxy("id:opaque -t 1");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Missing arg for -t
            communicator.stringToProxy("id:opaque -t -v abcd");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Missing arg for -v
            communicator.stringToProxy("id:opaque -t 1 -v");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Not a number for -t
            communicator.stringToProxy("id:opaque -t x -v abcd");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // < 0 for -t
            communicator.stringToProxy("id:opaque -t -1 -v abcd");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Invalid char for -v
            communicator.stringToProxy("id:opaque -t 99 -v x?c");
            test(false);
        }
        catch (ParseException)
        {
        }

        try
        {
            // Invalid lenght for base64 input
            communicator.stringToProxy("id:opaque -t 99 -v xc");
            test(false);
        }
        catch (ParseException)
        {
        }

        // Legal TCP endpoint expressed as opaque endpoint
        Ice.ObjectPrx p1 = communicator.stringToProxy("test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
        string pstr = communicator.proxyToString(p1);
        test(pstr == "test:tcp -h 127.0.0.1 -p 12010 -t 10000");

        // Opaque endpoint encoded with 1.1 encoding.
        Ice.ObjectPrx p2 = communicator.stringToProxy("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
        test(communicator.proxyToString(p2) == "test:tcp -h 127.0.0.1 -p 12010 -t 10000");

        if (communicator.getProperties().getIcePropertyAsInt("Ice.IPv6") == 0)
        {
            // Two legal TCP endpoints expressed as opaque endpoints
            p1 = communicator.stringToProxy("test -e 1.0:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
            pstr = communicator.proxyToString(p1);
            test(pstr == "test -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

            // Test that an SSL endpoint and a nonsense endpoint get written back out as an opaque endpoint.
            p1 = communicator.stringToProxy("test -e 1.0:opaque -e 1.0 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -e 1.0 -t 99 -v abch");
            pstr = communicator.proxyToString(p1);
            test(pstr == "test -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch");
        }

        output.WriteLine("ok");

        output.Write("testing communicator shutdown/destroy... ");
        output.Flush();
        {
            Ice.Communicator com = Ice.Util.initialize();
            com.shutdown();
            test(com.isShutdown());
            await com.shutdownCompleted;
            com.destroy();
            com.shutdown();
            test(com.isShutdown());
            await com.shutdownCompleted;
            com.destroy();
        }
        output.WriteLine("ok");

        return cl;
    }
}
