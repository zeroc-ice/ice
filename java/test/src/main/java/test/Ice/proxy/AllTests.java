// Copyright (c) ZeroC, Inc.

package test.Ice.proxy;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Connection;
import com.zeroc.Ice.EncodingVersion;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.EndpointSelectionType;
import com.zeroc.Ice.FacetNotExistException;
import com.zeroc.Ice.FeatureNotSupportedException;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocatorPrx;
import com.zeroc.Ice.MarshalException;
import com.zeroc.Ice.NoEndpointException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationMode;
import com.zeroc.Ice.OutputStream;
import com.zeroc.Ice.ParseException;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.RouterPrx;
import com.zeroc.Ice.ToStringMode;
import com.zeroc.Ice.UnknownLocalException;
import com.zeroc.Ice.Util;

import test.Ice.proxy.Test.DiamondClassPrx;
import test.Ice.proxy.Test.MyClassPrx;
import test.Ice.proxy.Test.MyDerivedClassPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.time.Duration;
import java.util.HashMap;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static MyClassPrx allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        final boolean bluetooth =
            communicator.getProperties().getIceProperty("Ice.Default.Protocol").indexOf("bt")
                == 0;
        PrintWriter out = helper.getWriter();

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test:" + helper.getTestEndpoint(0);
        ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);

        ObjectPrx b1 = communicator.stringToProxy("test");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getAdapterId().isEmpty()
                && b1.ice_getFacet().isEmpty());
        b1 = communicator.stringToProxy("test ");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getFacet().isEmpty());
        b1 = communicator.stringToProxy(" test ");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getFacet().isEmpty());
        b1 = communicator.stringToProxy(" test");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getFacet().isEmpty());
        b1 = communicator.stringToProxy("'test -f facet'");
        test(
            "test -f facet".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getFacet().isEmpty());
        try {
            b1 = communicator.stringToProxy("\"test -f facet'");
            test(false);
        } catch (ParseException ex) {}
        b1 = communicator.stringToProxy("\"test -f facet\"");
        test(
            "test -f facet".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getFacet().isEmpty());
        b1 = communicator.stringToProxy("\"test -f facet@test\"");
        test(
            "test -f facet@test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getFacet().isEmpty());
        b1 = communicator.stringToProxy("\"test -f facet@test @test\"");
        test(
            "test -f facet@test @test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getFacet().isEmpty());
        try {
            b1 = communicator.stringToProxy("test test");
            test(false);
        } catch (ParseException ex) {}
        b1 = communicator.stringToProxy("test\\040test");
        test(
            "test test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty());
        try {
            b1 = communicator.stringToProxy("test\\777");
            test(false);
        } catch (ParseException ex) {}
        b1 = communicator.stringToProxy("test\\40test");
        test("test test".equals(b1.ice_getIdentity().name));

        // Test some octal corner cases.
        b1 = communicator.stringToProxy("test\\4test");
        test("test\4test".equals(b1.ice_getIdentity().name));
        b1 = communicator.stringToProxy("test\\04test");
        test("test\4test".equals(b1.ice_getIdentity().name));
        b1 = communicator.stringToProxy("test\\004test");
        test("test\4test".equals(b1.ice_getIdentity().name));
        b1 = communicator.stringToProxy("test\\1114test");
        test("test\1114test".equals(b1.ice_getIdentity().name));

        b1 = communicator.stringToProxy("test\\b\\f\\n\\r\\t\\'\\\"\\\\test");
        test(
            "test\b\f\n\r\t\'\"\\test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty());

        b1 = communicator.stringToProxy("category/test");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && "category".equals(b1.ice_getIdentity().category)
                && b1.ice_getAdapterId().isEmpty());

        b1 = communicator.stringToProxy("test:tcp --sourceAddress \"::1\"");
        test(b1.equals(communicator.stringToProxy(b1.toString())));

        b1 =
            communicator.stringToProxy(
                "test:udp --sourceAddress \"::1\" --interface \"0:0:0:0:0:0:0:1%lo\"");
        test(b1.equals(communicator.stringToProxy(b1.toString())));

        b1 = communicator.stringToProxy("");
        test(b1 == null);
        b1 = communicator.stringToProxy("\"\"");
        test(b1 == null);
        try {
            b1 = communicator.stringToProxy("\"\" test"); // Invalid trailing characters.
            test(false);
        } catch (ParseException ex) {}
        try {
            b1 = communicator.stringToProxy("test:"); // Missing endpoint.
            test(false);
        } catch (ParseException ex) {}

        b1 = communicator.stringToProxy("test@adapter");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "adapter".equals(b1.ice_getAdapterId()));
        try {
            b1 = communicator.stringToProxy("id@adapter test");
            test(false);
        } catch (ParseException ex) {}
        b1 = communicator.stringToProxy("category/test@adapter");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && "category".equals(b1.ice_getIdentity().category)
                && "adapter".equals(b1.ice_getAdapterId()));
        b1 = communicator.stringToProxy("category/test@adapter:tcp");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && "category".equals(b1.ice_getIdentity().category)
                && "adapter:tcp".equals(b1.ice_getAdapterId()));
        b1 = communicator.stringToProxy("'category 1/test'@adapter");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && "category 1".equals(b1.ice_getIdentity().category)
                && "adapter".equals(b1.ice_getAdapterId()));
        b1 = communicator.stringToProxy("'category/test 1'@adapter");
        test(
            "test 1".equals(b1.ice_getIdentity().name)
                && "category".equals(b1.ice_getIdentity().category)
                && "adapter".equals(b1.ice_getAdapterId()));
        b1 = communicator.stringToProxy("'category/test'@'adapter 1'");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && "category".equals(b1.ice_getIdentity().category)
                && "adapter 1".equals(b1.ice_getAdapterId()));
        b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@adapter");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && "category /test@foo".equals(b1.ice_getIdentity().category)
                && "adapter".equals(b1.ice_getAdapterId()));
        b1 = communicator.stringToProxy("\"category \\/test@foo/test\"@\"adapter:tcp\"");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && "category /test@foo".equals(b1.ice_getIdentity().category)
                && "adapter:tcp".equals(b1.ice_getAdapterId()));

        b1 = communicator.stringToProxy("id -f facet");
        test(
            "id".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet".equals(b1.ice_getFacet()));
        b1 = communicator.stringToProxy("id -f 'facet x'");
        test(
            "id".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet x".equals(b1.ice_getFacet()));
        b1 = communicator.stringToProxy("id -f \"facet x\"");
        test(
            "id".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet x".equals(b1.ice_getFacet()));
        try {
            b1 = communicator.stringToProxy("id -f \"facet x");
            test(false);
        } catch (ParseException ex) {}
        try {
            b1 = communicator.stringToProxy("id -f \'facet x");
            test(false);
        } catch (ParseException ex) {}
        b1 = communicator.stringToProxy("test -f facet:tcp");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet".equals(b1.ice_getFacet())
                && b1.ice_getAdapterId().isEmpty());
        b1 = communicator.stringToProxy("test -f \"facet:tcp\"");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet:tcp".equals(b1.ice_getFacet())
                && b1.ice_getAdapterId().isEmpty());
        b1 = communicator.stringToProxy("test -f facet@test");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet".equals(b1.ice_getFacet())
                && "test".equals(b1.ice_getAdapterId()));
        b1 = communicator.stringToProxy("test -f 'facet@test'");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet@test".equals(b1.ice_getFacet())
                && b1.ice_getAdapterId().isEmpty());
        b1 = communicator.stringToProxy("test -f 'facet@test'@test");
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && "facet@test".equals(b1.ice_getFacet())
                && "test".equals(b1.ice_getAdapterId()));
        try {
            b1 = communicator.stringToProxy("test -f facet@test @test");
            test(false);
        } catch (ParseException ex) {}
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

        test(b1.ice_getEncodingVersion().equals(Util.currentEncoding()));

        b1 = communicator.stringToProxy("test -e 1.0");
        test(b1.ice_getEncodingVersion().major == 1 && b1.ice_getEncodingVersion().minor == 0);

        b1 = communicator.stringToProxy("test -e 6.5");
        test(b1.ice_getEncodingVersion().major == 6 && b1.ice_getEncodingVersion().minor == 5);

        b1 = communicator.stringToProxy("test -p 1.0 -e 1.0");
        test("test -e 1.0".equals(b1.toString()));

        b1 = communicator.stringToProxy("test -p 6.5 -e 1.0");
        test("test -p 6.5 -e 1.0".equals(b1.toString()));

        try {
            communicator.stringToProxy("test:tcp@adapterId");
            test(false);
        } catch (ParseException ex) {}
        // This is an unknown endpoint warning, not a parse exception.
        //
        // try
        // {
        //   b1 = communicator.stringToProxy("test -f the:facet:tcp");
        //   test(false);
        // }
        // catch(ParseException ex)
        // {
        // }
        try {
            communicator.stringToProxy("test: :tcp");
            test(false);
        } catch (ParseException ex) {}

        //
        // Test invalid endpoint syntax
        //
        try {
            communicator.createObjectAdapterWithEndpoints("BadAdapter", " : ");
            test(false);
        } catch (ParseException ex) {}

        try {
            communicator.createObjectAdapterWithEndpoints("BadAdapter", "tcp: ");
            test(false);
        } catch (ParseException ex) {}

        try {
            communicator.createObjectAdapterWithEndpoints("BadAdapter", ":tcp");
            test(false);
        } catch (ParseException ex) {}

        //CHECKSTYLE:OFF: AvoidEscapedUnicodeCharacters
        //
        // Test for bug ICE-5543: escaped escapes in stringToIdentity
        //
        Identity id = new Identity("test", ",X2QNUAzSBcJ_e$AV;E\\");
        Identity id2 =
            Util.stringToIdentity(communicator.identityToString(id));
        test(id.equals(id2));

        id = new Identity("test", ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\");
        id2 = Util.stringToIdentity(communicator.identityToString(id));
        test(id.equals(id2));

        id = new Identity("/test", "cat/");
        String idStr = communicator.identityToString(id);
        test("cat\\//\\/test".equals(idStr));
        id2 = Util.stringToIdentity(idStr);
        test(id.equals(id2));

        // Input string with various pitfalls
        id = Util.stringToIdentity("\\342\\x82\\254\\60\\x9\\60\\");
        // Use the Unicode value instead of a literal Euro symbol
        test("\u20ac0\t0\\".equals(id.name) && id.category.isEmpty());

        try {
            // Illegal character < 32
            id = Util.stringToIdentity("xx\01FooBar");
            test(false);
        } catch (ParseException e) {}

        try {
            // Illegal surrogate
            id = Util.stringToIdentity("xx\\ud911");
            test(false);
        } catch (ParseException e) {}

        // Testing bytes 127 (\x7F, \177) and €
        // Use the Unicode value instead of a literal Euro symbol
        id = new Identity("test", "\177\u20ac");

        idStr = Util.identityToString(id, ToStringMode.Unicode);
        test("\\u007f\u20ac/test".equals(idStr));
        id2 = Util.stringToIdentity(idStr);
        test(id.equals(id2));
        test(Util.identityToString(id).equals(idStr));

        idStr = Util.identityToString(id, ToStringMode.ASCII);
        test("\\u007f\\u20ac/test".equals(idStr));
        id2 = Util.stringToIdentity(idStr);
        test(id.equals(id2));

        idStr = Util.identityToString(id, ToStringMode.Compat);
        test("\\177\\342\\202\\254/test".equals(idStr));
        id2 = Util.stringToIdentity(idStr);
        test(id.equals(id2));

        id2 = Util.stringToIdentity(communicator.identityToString(id));
        test(id.equals(id2));

        // More unicode character
        id =
            new Identity(
                "banana \016-\ud83c\udf4c\u20ac\u00a2\u0024", "greek \ud800\udd6a");

        idStr = Util.identityToString(id, ToStringMode.Unicode);
        test("greek \ud800\udd6a/banana \\u000e-\ud83c\udf4c\u20ac\u00a2$".equals(idStr));
        id2 = Util.stringToIdentity(idStr);
        test(id.equals(id2));

        idStr = Util.identityToString(id, ToStringMode.ASCII);
        test("greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$".equals(idStr));
        id2 = Util.stringToIdentity(idStr);
        test(id.equals(id2));

        idStr = Util.identityToString(id, ToStringMode.Compat);
        id2 = Util.stringToIdentity(idStr);
        test(
            idStr.equals(
                "greek \\360\\220\\205\\252/banana"
                    + " \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$"));
        test(id.equals(id2));

        out.println("ok");
        //CHECKSTYLE:ON: AvoidEscapedUnicodeCharacters

        out.print("testing proxyToString... ");
        out.flush();
        b1 = communicator.stringToProxy(ref);
        ObjectPrx b2 = communicator.stringToProxy(communicator.proxyToString(b1));
        test(b1.equals(b2));

        if (b1.ice_getConnection() != null) // not colloc-optimized target
            {
                b2 = b1.ice_getConnection().createProxy(Util.stringToIdentity("fixed"));
                String str = communicator.proxyToString(b2);
                test(b2.toString().equals(str));
                String str2 =
                    b1.ice_identity(b2.ice_getIdentity()).ice_secure(b2.ice_isSecure()).toString();

                // Verify that the stringified fixed proxy is the same as a regular stringified proxy
                // but without endpoints
                test(str2.startsWith(str));
                test(str2.charAt(str.length()) == ':');
            }
        out.println("ok");

        out.print("testing propertyToProxy... ");
        out.flush();
        Properties prop = communicator.getProperties();
        String propertyPrefix = "Foo.Proxy";
        prop.setProperty(propertyPrefix, "test:" + helper.getTestEndpoint(0));
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(
            "test".equals(b1.ice_getIdentity().name)
                && b1.ice_getIdentity().category.isEmpty()
                && b1.ice_getAdapterId().isEmpty()
                && b1.ice_getFacet().isEmpty());

        String property;

        property = propertyPrefix + ".Locator";
        test(b1.ice_getLocator() == null);
        prop.setProperty(property, "locator:tcp -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(
            b1.ice_getLocator() != null
                && "locator".equals(b1.ice_getLocator().ice_getIdentity().name));
        prop.setProperty(property, "");

        property = propertyPrefix + ".LocatorCacheTimeout";
        test(b1.ice_getLocatorCacheTimeout().equals(Duration.ofSeconds(-1)));
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocatorCacheTimeout().equals(Duration.ofSeconds(1)));
        prop.setProperty(property, "");

        // Now retest with an indirect proxy.
        prop.setProperty(propertyPrefix, "test");
        property = propertyPrefix + ".Locator";
        prop.setProperty(property, "locator:tcp -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(
            b1.ice_getLocator() != null
                && "locator".equals(b1.ice_getLocator().ice_getIdentity().name));
        prop.setProperty(property, "");

        property = propertyPrefix + ".LocatorCacheTimeout";
        test(b1.ice_getLocatorCacheTimeout().equals(Duration.ofSeconds(-1)));
        prop.setProperty(property, "1");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getLocatorCacheTimeout().equals(Duration.ofSeconds(1)));
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
        prop.setProperty(property, "router:tcp -p 10000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(
            b1.ice_getRouter() != null
                && "router".equals(b1.ice_getRouter().ice_getIdentity().name));
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
        test(b1.ice_getInvocationTimeout().equals(Duration.ofMillis(-1)));
        prop.setProperty(property, "1000");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getInvocationTimeout().equals(Duration.ofSeconds(1)));
        prop.setProperty(property, "");

        property = propertyPrefix + ".EndpointSelection";
        test(b1.ice_getEndpointSelection() == EndpointSelectionType.Random);
        prop.setProperty(property, "Random");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getEndpointSelection() == EndpointSelectionType.Random);
        prop.setProperty(property, "Ordered");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(b1.ice_getEndpointSelection() == EndpointSelectionType.Ordered);
        prop.setProperty(property, "");

        property = propertyPrefix + ".CollocationOptimized";
        test(b1.ice_isCollocationOptimized());
        prop.setProperty(property, "0");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test(!b1.ice_isCollocationOptimized());
        prop.setProperty(property, "");

        property = propertyPrefix + ".Context.c1";
        test(b1.ice_getContext().get("c1") == null);
        prop.setProperty(property, "TEST");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test("TEST".equals(b1.ice_getContext().get("c1")));

        property = propertyPrefix + ".Context.c2";
        test(b1.ice_getContext().get("c2") == null);
        prop.setProperty(property, "TEST");
        b1 = communicator.propertyToProxy(propertyPrefix);
        test("TEST".equals(b1.ice_getContext().get("c2")));

        prop.setProperty(propertyPrefix + ".Context.c1", "");
        prop.setProperty(propertyPrefix + ".Context.c2", "");

        out.println("ok");

        out.print("testing proxyToProperty... ");
        out.flush();

        b1 = communicator.stringToProxy("test");
        b1 = b1.ice_collocationOptimized(true);
        b1 = b1.ice_connectionCached(true);
        b1 = b1.ice_preferSecure(false);
        b1 = b1.ice_endpointSelection(EndpointSelectionType.Ordered);
        b1 = b1.ice_locatorCacheTimeout(100);
        b1 = b1.ice_invocationTimeout(1234);
        b1 = b1.ice_encodingVersion(new EncodingVersion((byte) 1, (byte) 0));

        ObjectPrx router = communicator.stringToProxy("router");
        router = router.ice_collocationOptimized(false);
        router = router.ice_connectionCached(true);
        router = router.ice_preferSecure(true);
        router = router.ice_endpointSelection(EndpointSelectionType.Random);
        router = router.ice_locatorCacheTimeout(200);
        router = router.ice_invocationTimeout(1500);

        ObjectPrx locator = communicator.stringToProxy("locator");
        locator = locator.ice_collocationOptimized(true);
        locator = locator.ice_connectionCached(false);
        locator = locator.ice_preferSecure(true);
        locator = locator.ice_endpointSelection(EndpointSelectionType.Random);
        locator = locator.ice_locatorCacheTimeout(300);
        locator = locator.ice_invocationTimeout(1500);

        locator = locator.ice_router(RouterPrx.uncheckedCast(router));
        b1 = b1.ice_locator(LocatorPrx.uncheckedCast(locator));

        Map<String, String> proxyProps = communicator.proxyToProperty(b1, "Test");
        test(proxyProps.size() == 21);

        test("test -e 1.0".equals(proxyProps.get("Test")));
        test("1".equals(proxyProps.get("Test.CollocationOptimized")));
        test("1".equals(proxyProps.get("Test.ConnectionCached")));
        test("0".equals(proxyProps.get("Test.PreferSecure")));
        test("Ordered".equals(proxyProps.get("Test.EndpointSelection")));
        test("100".equals(proxyProps.get("Test.LocatorCacheTimeout")));
        test("1234".equals(proxyProps.get("Test.InvocationTimeout")));

        test("locator".equals(proxyProps.get("Test.Locator")));
        // Locator collocation optimization is always disabled.
        // test(proxyProps.get("Test.Locator.CollocationOptimized").equals("1"));
        test("0".equals(proxyProps.get("Test.Locator.ConnectionCached")));
        test("1".equals(proxyProps.get("Test.Locator.PreferSecure")));
        test("Random".equals(proxyProps.get("Test.Locator.EndpointSelection")));
        test("300".equals(proxyProps.get("Test.Locator.LocatorCacheTimeout")));
        test("1500".equals(proxyProps.get("Test.Locator.InvocationTimeout")));

        test("router".equals(proxyProps.get("Test.Locator.Router")));
        test("0".equals(proxyProps.get("Test.Locator.Router.CollocationOptimized")));
        test("1".equals(proxyProps.get("Test.Locator.Router.ConnectionCached")));
        test("1".equals(proxyProps.get("Test.Locator.Router.PreferSecure")));
        test("Random".equals(proxyProps.get("Test.Locator.Router.EndpointSelection")));
        test("200".equals(proxyProps.get("Test.Locator.Router.LocatorCacheTimeout")));
        test("1500".equals(proxyProps.get("Test.Locator.Router.InvocationTimeout")));

        out.println("ok");

        out.print("testing ice_getCommunicator... ");
        out.flush();
        test(base.ice_getCommunicator() == communicator);
        out.println("ok");

        out.print("testing proxy methods... ");
        out.flush();
        test("facet".equals(base.ice_facet("facet").ice_getFacet()));
        test("id".equals(base.ice_adapterId("id").ice_getAdapterId()));
        test(base.ice_twoway().ice_isTwoway());
        test(base.ice_oneway().ice_isOneway());
        test(base.ice_batchOneway().ice_isBatchOneway());
        test(base.ice_datagram().ice_isDatagram());
        test(base.ice_batchDatagram().ice_isBatchDatagram());
        test(base.ice_secure(true).ice_isSecure());
        test(!base.ice_secure(false).ice_isSecure());
        test(base.ice_collocationOptimized(true).ice_isCollocationOptimized());
        test(!base.ice_collocationOptimized(false).ice_isCollocationOptimized());
        test(base.ice_preferSecure(true).ice_isPreferSecure());
        test(!base.ice_preferSecure(false).ice_isPreferSecure());
        test(
            base.ice_encodingVersion(Util.Encoding_1_0)
                .ice_getEncodingVersion()
                .equals(Util.Encoding_1_0));
        test(
            base.ice_encodingVersion(Util.Encoding_1_1)
                .ice_getEncodingVersion()
                .equals(Util.Encoding_1_1));
        test(
            !base.ice_encodingVersion(Util.Encoding_1_0)
                .ice_getEncodingVersion()
                .equals(Util.Encoding_1_1));

        test(
            base.ice_invocationTimeout(10)
                .ice_getInvocationTimeout()
                .equals(Duration.ofMillis(10)));
        test(base.ice_invocationTimeout(0).ice_getInvocationTimeout().equals(Duration.ZERO));
        test(
            base.ice_invocationTimeout(-1)
                .ice_getInvocationTimeout()
                .equals(Duration.ofMillis(-1)));
        test(
            base.ice_invocationTimeout(-2)
                .ice_getInvocationTimeout()
                .equals(Duration.ofMillis(-2)));

        test(
            base.ice_locatorCacheTimeout(10)
                .ice_getLocatorCacheTimeout()
                .equals(Duration.ofSeconds(10)));
        test(base.ice_locatorCacheTimeout(0).ice_getLocatorCacheTimeout().equals(Duration.ZERO));
        test(
            base.ice_locatorCacheTimeout(-1)
                .ice_getLocatorCacheTimeout()
                .equals(Duration.ofSeconds(-1)));
        test(
            base.ice_locatorCacheTimeout(-2)
                .ice_getLocatorCacheTimeout()
                .equals(Duration.ofSeconds(-2)));

        // Ensure that the proxy methods can be called unambiguously with the correct return type.
        var diamondClass = DiamondClassPrx.uncheckedCast(base);
        var onewayDiamondClass = diamondClass.ice_oneway();
        test(onewayDiamondClass instanceof DiamondClassPrx);

        out.println("ok");

        out.print("testing proxy comparison... ");
        out.flush();

        test(communicator.stringToProxy("foo").equals(communicator.stringToProxy("foo")));
        test(!communicator.stringToProxy("foo").equals(communicator.stringToProxy("foo2")));

        ObjectPrx compObj = communicator.stringToProxy("foo");

        test(compObj.ice_facet("facet").equals(compObj.ice_facet("facet")));
        test(!compObj.ice_facet("facet").equals(compObj.ice_facet("facet1")));

        test(compObj.ice_oneway().equals(compObj.ice_oneway()));
        test(!compObj.ice_oneway().equals(compObj.ice_twoway()));

        test(compObj.ice_secure(true).equals(compObj.ice_secure(true)));
        test(!compObj.ice_secure(false).equals(compObj.ice_secure(true)));

        test(compObj.ice_collocationOptimized(true).equals(compObj.ice_collocationOptimized(true)));
        test(
            !compObj.ice_collocationOptimized(false)
                .equals(compObj.ice_collocationOptimized(true)));

        test(compObj.ice_connectionCached(true).equals(compObj.ice_connectionCached(true)));
        test(!compObj.ice_connectionCached(false).equals(compObj.ice_connectionCached(true)));

        test(
            compObj.ice_endpointSelection(EndpointSelectionType.Random)
                .equals(compObj.ice_endpointSelection(EndpointSelectionType.Random)));
        test(
            !compObj.ice_endpointSelection(EndpointSelectionType.Random)
                .equals(compObj.ice_endpointSelection(EndpointSelectionType.Ordered)));

        test(compObj.ice_connectionId("id2").equals(compObj.ice_connectionId("id2")));
        test(!compObj.ice_connectionId("id1").equals(compObj.ice_connectionId("id2")));

        test("id1".equals(compObj.ice_connectionId("id1").ice_getConnectionId()));
        test("id2".equals(compObj.ice_connectionId("id2").ice_getConnectionId()));

        test(compObj.ice_compress(true).equals(compObj.ice_compress(true)));
        test(!compObj.ice_compress(false).equals(compObj.ice_compress(true)));

        test(!compObj.ice_getCompress().isPresent());
        test(compObj.ice_compress(true).ice_getCompress().get() == true);
        test(compObj.ice_compress(false).ice_getCompress().get() == false);

        var loc1 = LocatorPrx.createProxy(communicator, "loc1:tcp -p 10000");
        var loc2 = LocatorPrx.createProxy(communicator, "loc2:tcp -p 10000");
        test(compObj.ice_locator(null).equals(compObj.ice_locator(null)));
        test(compObj.ice_locator(loc1).equals(compObj.ice_locator(loc1)));
        test(!compObj.ice_locator(loc1).equals(compObj.ice_locator(null)));
        test(!compObj.ice_locator(null).equals(compObj.ice_locator(loc2)));
        test(!compObj.ice_locator(loc1).equals(compObj.ice_locator(loc2)));

        var rtr1 = RouterPrx.createProxy(communicator, "rtr1:tcp -p 10000");
        var rtr2 = RouterPrx.createProxy(communicator, "rtr2:tcp -p 10000");
        test(compObj.ice_router(null).equals(compObj.ice_router(null)));
        test(compObj.ice_router(rtr1).equals(compObj.ice_router(rtr1)));
        test(!compObj.ice_router(rtr1).equals(compObj.ice_router(null)));
        test(!compObj.ice_router(null).equals(compObj.ice_router(rtr2)));
        test(!compObj.ice_router(rtr1).equals(compObj.ice_router(rtr2)));

        Map<String, String> ctx1 = new HashMap<>();
        ctx1.put("ctx1", "v1");
        Map<String, String> ctx2 = new HashMap<>();
        ctx2.put("ctx2", "v2");
        test(compObj.ice_context(null).equals(compObj.ice_context(null)));
        test(compObj.ice_context(ctx1).equals(compObj.ice_context(ctx1)));
        test(!compObj.ice_context(ctx1).equals(compObj.ice_context(null)));
        test(!compObj.ice_context(null).equals(compObj.ice_context(ctx2)));
        test(!compObj.ice_context(ctx1).equals(compObj.ice_context(ctx2)));

        test(compObj.ice_preferSecure(true).equals(compObj.ice_preferSecure(true)));
        test(!compObj.ice_preferSecure(true).equals(compObj.ice_preferSecure(false)));

        ObjectPrx compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
        ObjectPrx compObj2 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
        test(!compObj1.equals(compObj2));

        compObj1 = communicator.stringToProxy("foo@MyAdapter1");
        compObj2 = communicator.stringToProxy("foo@MyAdapter2");
        test(!compObj1.equals(compObj2));

        test(compObj1.ice_locatorCacheTimeout(20).equals(compObj1.ice_locatorCacheTimeout(20)));
        test(!compObj1.ice_locatorCacheTimeout(10).equals(compObj1.ice_locatorCacheTimeout(20)));

        test(compObj1.ice_invocationTimeout(20).equals(compObj1.ice_invocationTimeout(20)));
        test(!compObj1.ice_invocationTimeout(10).equals(compObj1.ice_invocationTimeout(20)));

        compObj1 = communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
        compObj2 = communicator.stringToProxy("foo@MyAdapter1");
        test(!compObj1.equals(compObj2));

        Endpoint[] endpts1 =
            communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000").ice_getEndpoints();
        Endpoint[] endpts2 =
            communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10001").ice_getEndpoints();
        test(!endpts1[0].equals(endpts2[0]));
        test(
            endpts1[0].equals(
                communicator.stringToProxy("foo:tcp -h 127.0.0.1 -p 10000")
                    .ice_getEndpoints()[0]));

        test(
            compObj1.ice_encodingVersion(Util.Encoding_1_0)
                .equals(compObj1.ice_encodingVersion(Util.Encoding_1_0)));
        test(
            !compObj1.ice_encodingVersion(Util.Encoding_1_0)
                .equals(compObj1.ice_encodingVersion(Util.Encoding_1_1)));

        Connection baseConnection = base.ice_getConnection();
        if (baseConnection != null && !bluetooth) {
            Connection baseConnection2 =
                base.ice_connectionId("base2").ice_getConnection();
            compObj1 = compObj1.ice_fixed(baseConnection);
            compObj2 = compObj2.ice_fixed(baseConnection2);
            test(!compObj1.equals(compObj2));
        }

        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        MyClassPrx cl = MyClassPrx.checkedCast(base);
        test(cl != null);
        MyDerivedClassPrx derived = MyDerivedClassPrx.checkedCast(cl);
        test(derived != null);
        test(cl.equals(base));
        test(derived.equals(base));
        test(cl.equals(derived));
        try {
            MyDerivedClassPrx.checkedCast(cl, "facet");
            test(false);
        } catch (FacetNotExistException ex) {
            // expected
        }
        out.println("ok");

        out.print("testing checked cast with context... ");
        out.flush();

        Map<String, String> c = cl.getContext();
        test(c == null || c.isEmpty());

        c = new HashMap<>();
        c.put("one", "hello");
        c.put("two", "world");
        cl = MyClassPrx.checkedCast(base, c);
        Map<String, String> c2 = cl.getContext();
        test(c.equals(c2));
        out.println("ok");

        if (!bluetooth) {
            out.print("testing ice_fixed... ");
            out.flush();
            {
                Connection connection = cl.ice_getConnection();
                if (connection != null) {
                    test(!cl.ice_isFixed());
                    MyClassPrx prx = cl.ice_fixed(connection); // Test proxy return type.
                    test(prx.ice_isFixed());
                    prx.ice_ping();
                    test(cl.ice_secure(true).ice_fixed(connection).ice_isSecure());
                    test(
                        "facet"
                            .equals(cl.ice_facet("facet")
                                .ice_fixed(connection)
                                .ice_getFacet()));
                    test(cl.ice_oneway().ice_fixed(connection).ice_isOneway());
                    Map<String, String> ctx = new HashMap<String, String>();
                    ctx.put("one", "hello");
                    ctx.put("two", "world");
                    test(cl.ice_fixed(connection).ice_getContext().isEmpty());
                    test(cl.ice_context(ctx).ice_fixed(connection).ice_getContext().size() == 2);
                    test(
                        cl.ice_fixed(connection)
                            .ice_getInvocationTimeout()
                            .equals(Duration.ofMillis(-1)));
                    test(
                        cl.ice_invocationTimeout(10)
                            .ice_fixed(connection)
                            .ice_getInvocationTimeout()
                            .equals(Duration.ofMillis(10)));
                    test(cl.ice_fixed(connection).ice_getConnection() == connection);
                    test(
                        cl.ice_fixed(connection).ice_fixed(connection).ice_getConnection()
                            == connection);
                    test(cl.ice_compress(true).ice_fixed(connection).ice_getCompress().get());
                    Connection fixedConnection =
                        cl.ice_connectionId("ice_fixed").ice_getConnection();
                    test(
                        cl.ice_fixed(connection).ice_fixed(fixedConnection).ice_getConnection()
                            == fixedConnection);
                    try {
                        cl.ice_secure(!connection.getEndpoint().getInfo().secure())
                            .ice_fixed(connection)
                            .ice_ping();
                    } catch (NoEndpointException ex) {}
                    try {
                        cl.ice_datagram().ice_fixed(connection).ice_ping();
                    } catch (NoEndpointException ex) {}
                } else {
                    try {
                        cl.ice_fixed(connection);
                        test(false);
                    } catch (IllegalArgumentException e) {
                        // Expected with null connection.
                    }
                }
            }
            out.println("ok");
        }

        out.print("testing encoding versioning... ");
        out.flush();
        String ref20 = "test -e 2.0:" + helper.getTestEndpoint(0);
        var cl20 = MyClassPrx.createProxy(communicator, ref20);
        try {
            cl20.ice_ping();
            test(false);
        } catch (MarshalException ex) {
            // Cannot marshal with the 2.0 encoding version.
        }

        String ref10 = "test -e 1.0:" + helper.getTestEndpoint(0);
        var cl10 = MyClassPrx.createProxy(communicator, ref10);
        cl10.ice_ping();
        cl10.ice_encodingVersion(Util.Encoding_1_0).ice_ping();
        cl.ice_encodingVersion(Util.Encoding_1_0).ice_ping();

        // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
        // call will use the 1.1 encoding
        String ref13 = "test -e 1.3:" + helper.getTestEndpoint(0);
        var cl13 = MyClassPrx.createProxy(communicator, ref13);
        cl13.ice_ping();
        cl13.ice_pingAsync().join();

        try {
            // Send request with bogus 1.2 encoding.
            EncodingVersion version = new EncodingVersion((byte) 1, (byte) 2);
            OutputStream os = new OutputStream(communicator);
            os.startEncapsulation();
            os.endEncapsulation();
            byte[] inEncaps = os.finished();
            inEncaps[4] = version.major;
            inEncaps[5] = version.minor;
            cl.ice_invoke("ice_ping", OperationMode.Normal, inEncaps);
            test(false);
        } catch (UnknownLocalException ex) {
            var message = ex.getMessage();
            test(
                message.contains("::Ice::MarshalException")
                    || message.contains("Ice.MarshalException"));
        }

        try {
            // Send request with bogus 2.0 encoding.
            EncodingVersion version = new EncodingVersion((byte) 2, (byte) 0);
            OutputStream os = new OutputStream(communicator);
            os.startEncapsulation();
            os.endEncapsulation();
            byte[] inEncaps = os.finished();
            inEncaps[4] = version.major;
            inEncaps[5] = version.minor;
            cl.ice_invoke("ice_ping", OperationMode.Normal, inEncaps);
            test(false);
        } catch (UnknownLocalException ex) {
            var message = ex.getMessage();
            test(
                message.contains("::Ice::MarshalException")
                    || message.contains("Ice.MarshalException"));
        }

        out.println("ok");

        out.print("testing protocol versioning... ");
        out.flush();
        ref20 = "test -p 2.0:" + helper.getTestEndpoint(0);
        cl20 = MyClassPrx.createProxy(communicator, ref20);
        try {
            cl20.ice_ping();
            test(false);
        } catch (FeatureNotSupportedException ex) {
            // Server 2.0 proxy doesn't support 1.0 version.
        }

        ref10 = "test -p 1.0:" + helper.getTestEndpoint(0);
        cl10 = MyClassPrx.createProxy(communicator, ref10);
        cl10.ice_ping();

        // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
        // call will use the 1.1 protocol
        ref13 = "test -p 1.3:" + helper.getTestEndpoint(0);
        cl13 = MyClassPrx.createProxy(communicator, ref13);
        cl13.ice_ping();
        cl13.ice_pingAsync().join();
        out.println("ok");

        out.print("testing opaque endpoints... ");
        out.flush();

        try {
            // Invalid -x option
            communicator.stringToProxy("id:opaque -t 99 -v abc -x abc");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Missing -t and -v
            communicator.stringToProxy("id:opaque");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Repeated -t
            communicator.stringToProxy("id:opaque -t 1 -t 1 -v abc");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Repeated -v
            communicator.stringToProxy("id:opaque -t 1 -v abc -v abc");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Missing -t
            communicator.stringToProxy("id:opaque -v abc");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Missing -v
            communicator.stringToProxy("id:opaque -t 1");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Missing arg for -t
            communicator.stringToProxy("id:opaque -t -v abc");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Missing arg for -v
            communicator.stringToProxy("id:opaque -t 1 -v");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Not a number for -t
            communicator.stringToProxy("id:opaque -t x -v abc");
            test(false);
        } catch (ParseException ex) {}

        try {
            // < 0 for -t
            communicator.stringToProxy("id:opaque -t -1 -v abc");
            test(false);
        } catch (ParseException ex) {}

        try {
            // Invalid char for -v
            communicator.stringToProxy("id:opaque -t 99 -v x?c");
            test(false);
        } catch (ParseException ex) {}

        // Legal TCP endpoint expressed as opaque endpoint
        ObjectPrx p1 =
            communicator.stringToProxy(
                "test -e 1.1:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
        String pstr = communicator.proxyToString(p1);
        test("test:tcp -h 127.0.0.1 -p 12010 -t 10000".equals(pstr));

        // Opaque endpoint encoded with 1.1 encoding.
        ObjectPrx p2 =
            communicator.stringToProxy(
                "test:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==");
        test("test:tcp -h 127.0.0.1 -p 12010 -t 10000".equals(communicator.proxyToString(p2)));

        if (communicator.getProperties().getIcePropertyAsInt("Ice.IPv6") == 0) {
            // Two legal TCP endpoints expressed as opaque endpoints
            p1 =
                communicator.stringToProxy(
                    "test -e 1.0:opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:opaque"
                        + " -e 1.0 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==");
            pstr = communicator.proxyToString(p1);
            test(
                pstr.equals(
                    "test -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p"
                        + " 12011 -t 10000"));

            //
            // Test that an SSL endpoint and a nonsense endpoint get
            // written back out as an opaque endpoint.
            //
            p1 =
                communicator.stringToProxy(
                    "test -e 1.0:opaque -e 1.0 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque"
                        + " -t 99 -e 1.0 -v abch");
            pstr = communicator.proxyToString(p1);
            test(
                pstr.equals(
                    "test -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e"
                        + " 1.0 -v abch"));

            //
            // Test that the proxy with an SSL endpoint and a nonsense
            // endpoint (which the server doesn't understand either) can
            // be sent over the wire and returned by the server without
            // losing the opaque endpoints.
            //
            p2 = derived.echo(p1);
            pstr = communicator.proxyToString(p2);
            test(
                pstr.equals(
                    "test -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e"
                        + " 1.0 -v abch"));
        }
        out.println("ok");

        out.print("testing communicator shutdown/destroy... ");
        out.flush();
        {
            Communicator co = Util.initialize();
            co.shutdown();
            test(co.isShutdown());
            co.waitForShutdown();
            co.destroy();
            co.shutdown();
            test(co.isShutdown());
            co.waitForShutdown();
            co.destroy();
        }
        out.println("ok");

        return cl;
    }

    private AllTests() {}
}
