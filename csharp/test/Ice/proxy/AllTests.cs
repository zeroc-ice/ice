//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice
{
    namespace proxy
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.MyClassPrx allTests(global::Test.TestHelper helper)
            {
                var communicator = helper.communicator();
                var output = helper.getWriter();
                output.Write("testing stringToProxy... ");
                output.Flush();
                string rf = "test:" + helper.getTestEndpoint(0);
                var baseProxy = IObjectPrx.Parse(rf, communicator);
                test(baseProxy != null);

                var b1 = IObjectPrx.Parse("test", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.AdapterId.Length == 0 && b1.Facet.Length == 0);
                b1 = IObjectPrx.Parse("test ", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Length == 0);
                b1 = IObjectPrx.Parse(" test ", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Length == 0);
                b1 = IObjectPrx.Parse(" test", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Length == 0);
                b1 = IObjectPrx.Parse("'test -f facet'", communicator);
                test(b1.Identity.name.Equals("test -f facet") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Length == 0);
                try
                {
                    b1 = IObjectPrx.Parse("\"test -f facet'", communicator);
                    test(false);
                }
                catch (ProxyParseException)
                {
                }
                b1 = IObjectPrx.Parse("\"test -f facet\"", communicator);
                test(b1.Identity.name.Equals("test -f facet") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Length == 0);
                b1 = IObjectPrx.Parse("\"test -f facet@test\"", communicator);
                test(b1.Identity.name.Equals("test -f facet@test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Length == 0);
                b1 = IObjectPrx.Parse("\"test -f facet@test @test\"", communicator);
                test(b1.Identity.name.Equals("test -f facet@test @test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Length == 0);
                try
                {
                    b1 = IObjectPrx.Parse("test test", communicator);
                    test(false);
                }
                catch (ProxyParseException)
                {
                }
                b1 = IObjectPrx.Parse("test\\040test", communicator);
                test(b1.Identity.name.Equals("test test") && b1.Identity.category.Length == 0);
                try
                {
                    b1 = IObjectPrx.Parse("test\\777", communicator);
                    test(false);
                }
                catch (IdentityParseException)
                {
                }
                b1 = IObjectPrx.Parse("test\\40test", communicator);
                test(b1.Identity.name.Equals("test test"));

                // Test some octal and hex corner cases.
                b1 = IObjectPrx.Parse("test\\4test", communicator);
                test(b1.Identity.name.Equals("test\u0004test"));
                b1 = IObjectPrx.Parse("test\\04test", communicator);
                test(b1.Identity.name.Equals("test\u0004test"));
                b1 = IObjectPrx.Parse("test\\004test", communicator);
                test(b1.Identity.name.Equals("test\u0004test"));
                b1 = IObjectPrx.Parse("test\\1114test", communicator);
                test(b1.Identity.name.Equals("test\u00494test"));

                b1 = IObjectPrx.Parse("test\\b\\f\\n\\r\\t\\'\\\"\\\\test", communicator);
                test(b1.Identity.name.Equals("test\b\f\n\r\t\'\"\\test") && b1.Identity.category.Length == 0);

                b1 = IObjectPrx.Parse("category/test", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Equals("category") &&
                     b1.AdapterId.Length == 0);

                b1 = IObjectPrx.Parse("test:tcp --sourceAddress \"::1\"", communicator);
                test(b1.Equals(IObjectPrx.Parse(b1.ToString(), communicator)));

                b1 = IObjectPrx.Parse("test:udp --sourceAddress \"::1\" --interface \"0:0:0:0:0:0:0:1%lo\"", communicator);
                test(b1.Equals(IObjectPrx.Parse(b1.ToString(), communicator)));

                try
                {
                    b1 = IObjectPrx.Parse("", communicator);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    b1 = IObjectPrx.Parse("\"\"", communicator);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    b1 = IObjectPrx.Parse("\"\" test", communicator); // Invalid trailing characters.
                    test(false);
                }
                catch (ProxyParseException)
                {
                }

                try
                {
                    b1 = IObjectPrx.Parse("test:", communicator); // Missing endpoint.
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                b1 = IObjectPrx.Parse("test@adapter", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.AdapterId.Equals("adapter"));
                try
                {
                    b1 = IObjectPrx.Parse("id@adapter test", communicator);
                    test(false);
                }
                catch (ProxyParseException)
                {
                }
                b1 = IObjectPrx.Parse("category/test@adapter", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Equals("category") &&
                     b1.AdapterId.Equals("adapter"));
                b1 = IObjectPrx.Parse("category/test@adapter:tcp", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Equals("category") &&
                     b1.AdapterId.Equals("adapter:tcp"));
                b1 = IObjectPrx.Parse("'category 1/test'@adapter", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Equals("category 1") &&
                     b1.AdapterId.Equals("adapter"));
                b1 = IObjectPrx.Parse("'category/test 1'@adapter", communicator);
                test(b1.Identity.name.Equals("test 1") && b1.Identity.category.Equals("category") &&
                     b1.AdapterId.Equals("adapter"));
                b1 = IObjectPrx.Parse("'category/test'@'adapter 1'", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Equals("category") &&
                     b1.AdapterId.Equals("adapter 1"));
                b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@adapter", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Equals("category /test@foo") &&
                     b1.AdapterId.Equals("adapter"));
                b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@\"adapter:tcp\"", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Equals("category /test@foo") &&
                     b1.AdapterId.Equals("adapter:tcp"));

                b1 = IObjectPrx.Parse("id -f facet", communicator);
                test(b1.Identity.name.Equals("id") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet"));
                b1 = IObjectPrx.Parse("id -f 'facet x'", communicator);
                test(b1.Identity.name.Equals("id") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet x"));
                b1 = IObjectPrx.Parse("id -f \"facet x\"", communicator);
                test(b1.Identity.name.Equals("id") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet x"));
                try
                {
                    b1 = IObjectPrx.Parse("id -f \"facet x", communicator);
                    test(false);
                }
                catch (ProxyParseException)
                {
                }
                try
                {
                    b1 = IObjectPrx.Parse("id -f \'facet x", communicator);
                    test(false);
                }
                catch (ProxyParseException)
                {
                }
                b1 = IObjectPrx.Parse("test -f facet:tcp", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet") && b1.AdapterId.Length == 0);
                b1 = IObjectPrx.Parse("test -f \"facet:tcp\"", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet:tcp") && b1.AdapterId.Length == 0);
                b1 = IObjectPrx.Parse("test -f facet@test", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet") && b1.AdapterId.Equals("test"));
                b1 = IObjectPrx.Parse("test -f 'facet@test'", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet@test") && b1.AdapterId.Length == 0);
                b1 = IObjectPrx.Parse("test -f 'facet@test'@test", communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.Facet.Equals("facet@test") && b1.AdapterId.Equals("test"));
                try
                {
                    b1 = IObjectPrx.Parse("test -f facet@test @test", communicator);
                    test(false);
                }
                catch (ProxyParseException)
                {
                }
                b1 = IObjectPrx.Parse("test", communicator);
                test(b1.IsTwoway);
                b1 = IObjectPrx.Parse("test -t", communicator);
                test(b1.IsTwoway);
                b1 = IObjectPrx.Parse("test -o", communicator);
                test(b1.IsOneway);
                b1 = IObjectPrx.Parse("test -O", communicator);
                test(b1.InvocationMode == InvocationMode.BatchOneway);
                b1 = IObjectPrx.Parse("test -d", communicator);
                test(b1.InvocationMode == InvocationMode.Datagram);
                b1 = IObjectPrx.Parse("test -D", communicator);
                test(b1.InvocationMode == InvocationMode.BatchDatagram);
                b1 = IObjectPrx.Parse("test", communicator);
                test(!b1.IsSecure);
                b1 = IObjectPrx.Parse("test -s", communicator);
                test(b1.IsSecure);

                test(b1.EncodingVersion.Equals(Util.currentEncoding));

                b1 = IObjectPrx.Parse("test -e 1.0", communicator);
                test(b1.EncodingVersion.major == 1 && b1.EncodingVersion.minor == 0);

                b1 = IObjectPrx.Parse("test -e 6.5", communicator);
                test(b1.EncodingVersion.major == 6 && b1.EncodingVersion.minor == 5);

                b1 = IObjectPrx.Parse("test -p 1.0 -e 1.0", communicator);
                test(b1.ToString().Equals("test -t -e 1.0"));

                b1 = IObjectPrx.Parse("test -p 6.5 -e 1.0", communicator);
                test(b1.ToString().Equals("test -t -p 6.5 -e 1.0"));

                try
                {
                    IObjectPrx.Parse("test:tcp@adapterId", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }
                // This is an unknown endpoint warning, not a parse exception.
                //
                //try
                //{
                //   b1 = communicator.stringToProxy("test -f the:facet:tcp");
                //   test(false);
                //}
                //catch(EndpointParseException)
                //{
                //}
                try
                {
                    IObjectPrx.Parse("test: :tcp", communicator);
                    test(false);
                }
                catch (EndpointParseException)
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
                catch (EndpointParseException)
                {
                }

                try
                {
                    communicator.createObjectAdapterWithEndpoints("BadAdapter", "tcp: ");
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    communicator.createObjectAdapterWithEndpoints("BadAdapter", ":tcp");
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                //
                // Test for bug ICE-5543: escaped escapes in stringToIdentity
                //
                var id = new Identity("test", ",X2QNUAzSBcJ_e$AV;E\\");
                var id2 = Util.stringToIdentity(communicator.identityToString(id));
                test(id.Equals(id2));

                id = new Identity("test", ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\");
                id2 = Util.stringToIdentity(communicator.identityToString(id));
                test(id.Equals(id2));

                id = new Identity("/test", "cat/");
                string idStr = communicator.identityToString(id);
                test(idStr == "cat\\//\\/test");
                id2 = Util.stringToIdentity(idStr);
                test(id.Equals(id2));

                // Input string with various pitfalls
                id = Util.stringToIdentity("\\342\\x82\\254\\60\\x9\\60\\");
                test(id.name == "€0\t0\\" && id.category == "");

                try
                {
                    // Illegal character < 32
                    id = Util.stringToIdentity("xx\01FooBar");
                    test(false);
                }
                catch (IdentityParseException)
                {
                }

                try
                {
                    // Illegal surrogate
                    id = Util.stringToIdentity("xx\\ud911");
                    test(false);
                }
                catch (IdentityParseException)
                {
                }

                // Testing bytes 127(\x7F, \177) and €
                id = new Identity("test", "\x7f€");

                idStr = Util.identityToString(id, ToStringMode.Unicode);
                test(idStr == "\\u007f€/test");
                id2 = Util.stringToIdentity(idStr);
                test(id.Equals(id2));
                test(Util.identityToString(id) == idStr);

                idStr = Util.identityToString(id, ToStringMode.ASCII);
                test(idStr == "\\u007f\\u20ac/test");
                id2 = Util.stringToIdentity(idStr);
                test(id.Equals(id2));

                idStr = Util.identityToString(id, ToStringMode.Compat);
                test(idStr == "\\177\\342\\202\\254/test");
                id2 = Util.stringToIdentity(idStr);
                test(id.Equals(id2));

                id2 = Util.stringToIdentity(communicator.identityToString(id));
                test(id.Equals(id2));

                // More unicode character
                id = new Identity("banana \x0E-\ud83c\udf4c\u20ac\u00a2\u0024", "greek \ud800\udd6a");

                idStr = Util.identityToString(id, ToStringMode.Unicode);
                test(idStr == "greek \ud800\udd6a/banana \\u000e-\ud83c\udf4c\u20ac\u00a2$");
                id2 = Util.stringToIdentity(idStr);
                test(id.Equals(id2));

                idStr = Util.identityToString(id, ToStringMode.ASCII);
                test(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$");
                id2 = Util.stringToIdentity(idStr);
                test(id.Equals(id2));

                idStr = Util.identityToString(id, ToStringMode.Compat);
                id2 = Util.stringToIdentity(idStr);
                test(idStr == "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$");
                test(id.Equals(id2));

                output.WriteLine("ok");

                output.Write("testing proxyToString... ");
                output.Flush();
                b1 = IObjectPrx.Parse(rf, communicator);
                var b2 = IObjectPrx.Parse(b1.ToString(), communicator);
                test(b1.Equals(b2));

                if (b1.GetConnection() != null) // not colloc-optimized target
                {
                    b2 = b1.GetConnection().createProxy(Util.stringToIdentity("fixed"));
                    string str = b2.ToString();
                    test(b2.ToString() == str);
                    string str2 = b1.Clone(b2.Identity).Clone(secure: b2.IsSecure).ToString();

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
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.Identity.name.Equals("test") && b1.Identity.category.Length == 0 &&
                     b1.AdapterId.Length == 0 && b1.Facet.Length == 0);

                string property;

                property = propertyPrefix + ".Locator";
                test(b1.Locator == null);
                prop.setProperty(property, "locator:default -p 10000");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.Locator != null && b1.Locator.Identity.name.Equals("locator"));
                prop.setProperty(property, "");
                property = propertyPrefix + ".LocatorCacheTimeout";
                test(b1.LocatorCacheTimeout == -1);
                prop.setProperty(property, "1");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.LocatorCacheTimeout == 1);
                prop.setProperty(property, "");

                // Now retest with an indirect proxy.
                prop.setProperty(propertyPrefix, "test");
                property = propertyPrefix + ".Locator";
                prop.setProperty(property, "locator:default -p 10000");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.Locator != null && b1.Locator.Identity.name.Equals("locator"));
                prop.setProperty(property, "");

                property = propertyPrefix + ".LocatorCacheTimeout";
                test(b1.LocatorCacheTimeout == -1);
                prop.setProperty(property, "1");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.LocatorCacheTimeout == 1);
                prop.setProperty(property, "");

                // This cannot be tested so easily because the property is cached
                // on communicator initialization.
                //
                //prop.setProperty("Default.LocatorCacheTimeout", "60");
                //b1 = communicator.propertyToProxy(propertyPrefix);
                //test(b1.LocatorCacheTimeout == 60);
                //prop.setProperty("Default.LocatorCacheTimeout", "");

                prop.setProperty(propertyPrefix, "test:" + helper.getTestEndpoint(0));

                property = propertyPrefix + ".Router";
                test(b1.Router == null);
                prop.setProperty(property, "router:default -p 10000");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.Router != null && b1.Router.Identity.name.Equals("router"));
                prop.setProperty(property, "");

                property = propertyPrefix + ".PreferSecure";
                test(!b1.IsPreferSecure);
                prop.setProperty(property, "1");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.IsPreferSecure);
                prop.setProperty(property, "");

                property = propertyPrefix + ".ConnectionCached";
                test(b1.IsConnectionCached);
                prop.setProperty(property, "0");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(!b1.IsConnectionCached);
                prop.setProperty(property, "");

                property = propertyPrefix + ".InvocationTimeout";
                test(b1.InvocationTimeout == -1);
                prop.setProperty(property, "1000");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.InvocationTimeout == 1000);
                prop.setProperty(property, "");

                property = propertyPrefix + ".EndpointSelection";
                test(b1.EndpointSelection == EndpointSelectionType.Random);
                prop.setProperty(property, "Random");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.EndpointSelection == EndpointSelectionType.Random);
                prop.setProperty(property, "Ordered");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.EndpointSelection == EndpointSelectionType.Ordered);
                prop.setProperty(property, "");

                property = propertyPrefix + ".CollocationOptimized";
                test(b1.IsCollocationOptimized);
                prop.setProperty(property, "0");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(!b1.IsCollocationOptimized);
                prop.setProperty(property, "");

                property = propertyPrefix + ".Context.c1";
                test(!b1.Context.ContainsKey("c1"));
                prop.setProperty(property, "TEST");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.Context["c1"].Equals("TEST"));

                property = propertyPrefix + ".Context.c2";
                test(!b1.Context.ContainsKey("c2"));
                prop.setProperty(property, "TEST");
                b1 = IObjectPrx.ParseProperty(propertyPrefix, communicator);
                test(b1.Context["c2"].Equals("TEST"));

                prop.setProperty(propertyPrefix + ".Context.c1", "");
                prop.setProperty(propertyPrefix + ".Context.c2", "");

                output.WriteLine("ok");

                output.Write("testing proxyToProperty... ");
                output.Flush();

                var router = RouterPrx.Parse("router", communicator).Clone(
                    collocationOptimized: false,
                    connectionCached: true,
                    preferSecure: true,
                    endpointSelectionType: EndpointSelectionType.Random,
                    locatorCacheTimeout: 200,
                    invocationTimeout: 1500);

                var locator = LocatorPrx.Parse("locator", communicator).Clone(
                    collocationOptimized: true,
                    connectionCached: false,
                    preferSecure: true,
                    endpointSelectionType: EndpointSelectionType.Random,
                    locatorCacheTimeout: 300,
                    invocationTimeout: 1500,
                    router: router);

                b1 = IObjectPrx.Parse("test", communicator).Clone(
                    collocationOptimized: true,
                    connectionCached: true,
                    preferSecure: false,
                    endpointSelectionType: EndpointSelectionType.Ordered,
                    locatorCacheTimeout: 100,
                    invocationTimeout: 1234,
                    encodingVersion: new EncodingVersion(1, 0),
                    locator: locator);

                Dictionary<string, string> proxyProps = communicator.proxyToProperty(b1, "Test");
                test(proxyProps.Count == 21);

                test(proxyProps["Test"].Equals("test -t -e 1.0"));
                test(proxyProps["Test.CollocationOptimized"].Equals("1"));
                test(proxyProps["Test.ConnectionCached"].Equals("1"));
                test(proxyProps["Test.PreferSecure"].Equals("0"));
                test(proxyProps["Test.EndpointSelection"].Equals("Ordered"));
                test(proxyProps["Test.LocatorCacheTimeout"].Equals("100"));
                test(proxyProps["Test.InvocationTimeout"].Equals("1234"));

                test(proxyProps["Test.Locator"].Equals(
                         "locator -t -e " + Util.encodingVersionToString(Util.currentEncoding)));
                // Locator collocation optimization is always disabled.
                //test(proxyProps["Test.Locator.CollocationOptimized"].Equals("1"));
                test(proxyProps["Test.Locator.ConnectionCached"].Equals("0"));
                test(proxyProps["Test.Locator.PreferSecure"].Equals("1"));
                test(proxyProps["Test.Locator.EndpointSelection"].Equals("Random"));
                test(proxyProps["Test.Locator.LocatorCacheTimeout"].Equals("300"));
                test(proxyProps["Test.Locator.InvocationTimeout"].Equals("1500"));

                test(proxyProps["Test.Locator.Router"].Equals(
                         "router -t -e " + Util.encodingVersionToString(Util.currentEncoding)));
                test(proxyProps["Test.Locator.Router.CollocationOptimized"].Equals("0"));
                test(proxyProps["Test.Locator.Router.ConnectionCached"].Equals("1"));
                test(proxyProps["Test.Locator.Router.PreferSecure"].Equals("1"));
                test(proxyProps["Test.Locator.Router.EndpointSelection"].Equals("Random"));
                test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"].Equals("200"));
                test(proxyProps["Test.Locator.Router.InvocationTimeout"].Equals("1500"));

                output.WriteLine("ok");

                output.Write("testing ice_getCommunicator... ");
                output.Flush();
                test(baseProxy.Communicator == communicator);
                output.WriteLine("ok");

                output.Write("testing proxy methods... ");

                test(baseProxy.Clone(facet: "facet").Facet.Equals("facet"));
                test(baseProxy.Clone(adapterId: "id").AdapterId.Equals("id"));
                test(baseProxy.Clone(invocationMode: InvocationMode.Twoway).IsTwoway);
                test(baseProxy.Clone(invocationMode: InvocationMode.Oneway).IsOneway);
                test(baseProxy.Clone(invocationMode: InvocationMode.BatchOneway).InvocationMode == InvocationMode.BatchOneway);
                test(baseProxy.Clone(invocationMode: InvocationMode.Datagram).InvocationMode == InvocationMode.Datagram);
                test(baseProxy.Clone(invocationMode: InvocationMode.BatchDatagram).InvocationMode == InvocationMode.BatchDatagram);
                test(baseProxy.Clone(secure: true).IsSecure);
                test(!baseProxy.Clone(secure: false).IsSecure);
                test(baseProxy.Clone(collocationOptimized: true).IsCollocationOptimized);
                test(!baseProxy.Clone(collocationOptimized: false).IsCollocationOptimized);
                test(baseProxy.Clone(preferSecure: true).IsPreferSecure);
                test(!baseProxy.Clone(preferSecure: false).IsPreferSecure);

                try
                {
                    baseProxy.Clone(connectionTimeout: 0);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    baseProxy.Clone(connectionTimeout: -1);
                }
                catch (ArgumentException)
                {
                    test(false);
                }

                try
                {
                    baseProxy.Clone(connectionTimeout: -2);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    baseProxy.Clone(invocationTimeout: 0);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    baseProxy.Clone(invocationTimeout: -1);
                    baseProxy.Clone(invocationTimeout: -2);
                }
                catch (ArgumentException)
                {
                    test(false);
                }

                try
                {
                    baseProxy.Clone(invocationTimeout: -3);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                try
                {
                    baseProxy.Clone(locatorCacheTimeout: 0);
                }
                catch (ArgumentException)
                {
                    test(false);
                }

                try
                {
                    baseProxy.Clone(locatorCacheTimeout: -1);
                }
                catch (ArgumentException)
                {
                    test(false);
                }

                try
                {
                    baseProxy.Clone(locatorCacheTimeout: -2);
                    test(false);
                }
                catch (ArgumentException)
                {
                }

                output.WriteLine("ok");

                output.Write("testing proxy comparison... ");
                output.Flush();

                test(IObjectPrx.Parse("foo", communicator).Equals(IObjectPrx.Parse("foo", communicator)));
                test(!IObjectPrx.Parse("foo", communicator).Equals(IObjectPrx.Parse("foo2", communicator)));

                var compObj = IObjectPrx.Parse("foo", communicator);

                test(compObj.Clone(facet: "facet").Equals(compObj.Clone(facet: "facet")));
                test(!compObj.Clone(facet: "facet").Equals(compObj.Clone(facet: "facet1")));

                test(compObj.Clone(invocationMode: InvocationMode.Oneway).Equals(
                    compObj.Clone(invocationMode: InvocationMode.Oneway)));
                test(!compObj.Clone(invocationMode: InvocationMode.Oneway).Equals(
                    compObj.Clone(invocationMode: InvocationMode.Twoway)));

                test(compObj.Clone(secure: true).Equals(compObj.Clone(secure: true)));
                test(!compObj.Clone(secure: false).Equals(compObj.Clone(secure: true)));

                test(compObj.Clone(collocationOptimized: true).Equals(compObj.Clone(collocationOptimized: true)));
                test(!compObj.Clone(collocationOptimized: false).Equals(compObj.Clone(collocationOptimized: true)));

                test(compObj.Clone(connectionCached: true).Equals(compObj.Clone(connectionCached: true)));
                test(!compObj.Clone(connectionCached: false).Equals(compObj.Clone(connectionCached: true)));

                test(compObj.Clone(endpointSelectionType: EndpointSelectionType.Random).Equals(
                    compObj.Clone(endpointSelectionType: EndpointSelectionType.Random)));
                test(!compObj.Clone(endpointSelectionType: EndpointSelectionType.Random).Equals(
                    compObj.Clone(endpointSelectionType: EndpointSelectionType.Ordered)));

                test(compObj.Clone(connectionId: "id2").Equals(compObj.Clone(connectionId: "id2")));
                test(!compObj.Clone(connectionId: "id1").Equals(compObj.Clone(connectionId: "id2")));
                test(compObj.Clone(connectionId: "id1").ConnectionId.Equals("id1"));
                test(compObj.Clone(connectionId: "id2").ConnectionId.Equals("id2"));

                test(compObj.Clone(compress: true).Equals(compObj.Clone(compress: true)));
                test(!compObj.Clone(compress: false).Equals(compObj.Clone(compress: true)));

                test(!compObj.Compress.HasValue);
                test(compObj.Clone(compress: true).Compress.Value == true);
                test(compObj.Clone(compress: false).Compress.Value == false);

                test(compObj.Clone(connectionTimeout: 20).Equals(compObj.Clone(connectionTimeout: 20)));
                test(!compObj.Clone(connectionTimeout: 10).Equals(compObj.Clone(connectionTimeout: 20)));

                test(!compObj.ConnectionTimeout.HasValue);
                test(compObj.Clone(connectionTimeout: 10).ConnectionTimeout.Value == 10);
                test(compObj.Clone(connectionTimeout: 20).ConnectionTimeout.Value == 20);

                LocatorPrx loc1 = LocatorPrx.Parse("loc1:default -p 10000", communicator);
                LocatorPrx loc2 = LocatorPrx.Parse("loc2:default -p 10000", communicator);
                test(compObj.Clone(clearLocator: true).Equals(compObj.Clone(clearLocator: true)));
                test(compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc1)));
                test(!compObj.Clone(locator: loc1).Equals(compObj.Clone(clearLocator: true)));
                test(!compObj.Clone(clearLocator: true).Equals(compObj.Clone(locator: loc2)));
                test(!compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc2)));

                RouterPrx rtr1 = RouterPrx.Parse("rtr1:default -p 10000", communicator);
                RouterPrx rtr2 = RouterPrx.Parse("rtr2:default -p 10000", communicator);
                test(compObj.Clone(clearRouter: true).Equals(compObj.Clone(clearRouter: true)));
                test(compObj.Clone(router: rtr1).Equals(compObj.Clone(router: rtr1)));
                test(!compObj.Clone(router: rtr1).Equals(compObj.Clone(clearRouter: true)));
                test(!compObj.Clone(clearRouter: true).Equals(compObj.Clone(router: rtr2)));
                test(!compObj.Clone(router: rtr1).Equals(compObj.Clone(router: rtr2)));

                Dictionary<string, string> ctx1 = new Dictionary<string, string>();
                ctx1["ctx1"] = "v1";
                Dictionary<string, string> ctx2 = new Dictionary<string, string>();
                ctx2["ctx2"] = "v2";
                test(compObj.Clone(context: new Dictionary<string, string>()).Equals(
                    compObj.Clone(context: new Dictionary<string, string>())));
                test(compObj.Clone(context: ctx1).Equals(compObj.Clone(context: ctx1)));
                test(!compObj.Clone(context: ctx1).Equals(
                    compObj.Clone(context: new Dictionary<string, string>())));
                test(!compObj.Clone(context: new Dictionary<string, string>()).Equals(
                    compObj.Clone(context: ctx2)));
                test(!compObj.Clone(context: ctx1).Equals(compObj.Clone(context: ctx2)));

                test(compObj.Clone(preferSecure: true).Equals(compObj.Clone(preferSecure: true)));
                test(!compObj.Clone(preferSecure: true).Equals(compObj.Clone(preferSecure: false)));

                var compObj1 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10000", communicator);
                var compObj2 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10001", communicator);
                test(!compObj1.Equals(compObj2));

                compObj1 = IObjectPrx.Parse("foo@MyAdapter1", communicator);
                compObj2 = IObjectPrx.Parse("foo@MyAdapter2", communicator);
                test(!compObj1.Equals(compObj2));

                test(compObj1.Clone(locatorCacheTimeout: 20).Equals(compObj1.Clone(locatorCacheTimeout: 20)));
                test(!compObj1.Clone(locatorCacheTimeout: 10).Equals(compObj1.Clone(locatorCacheTimeout: 20)));

                test(compObj1.Clone(invocationTimeout: 20).Equals(compObj1.Clone(invocationTimeout: 20)));
                test(!compObj1.Clone(invocationTimeout: 10).Equals(compObj1.Clone(invocationTimeout: 20)));

                compObj1 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 1000", communicator);
                compObj2 = IObjectPrx.Parse("foo@MyAdapter1", communicator);
                test(!compObj1.Equals(compObj2));

                Endpoint[] endpts1 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10000", communicator).Endpoints;
                Endpoint[] endpts2 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10001", communicator).Endpoints;
                test(!endpts1[0].Equals(endpts2[0]));
                test(endpts1[0].Equals(IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10000", communicator).Endpoints[0]));

                Connection baseConnection = baseProxy.GetConnection();
                if (baseConnection != null)
                {
                    Connection baseConnection2 = baseProxy.Clone(connectionId: "base2").GetConnection();
                    compObj1 = compObj1.Clone(fixedConnection: baseConnection);
                    compObj2 = compObj2.Clone(fixedConnection: baseConnection2);
                    test(!compObj1.Equals(compObj2));
                }
                output.WriteLine("ok");

                output.Write("testing checked cast... ");
                output.Flush();
                Test.MyClassPrx cl = Test.MyClassPrx.CheckedCast(baseProxy);
                test(cl != null);
                Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrx.CheckedCast(cl);
                test(derived != null);
                test(cl.Equals(baseProxy));
                test(derived.Equals(baseProxy));
                test(cl.Equals(derived));
                test(Test.MyDerivedClassPrx.CheckedCast(cl.Clone(facet: "facet")) == null);
                output.WriteLine("ok");

                output.Write("testing checked cast with context... ");
                output.Flush();

                Dictionary<string, string> c = cl.getContext();
                test(c == null || c.Count == 0);

                c = new Dictionary<string, string>();
                c["one"] = "hello";
                c["two"] = "world";
                cl = Test.MyClassPrx.CheckedCast(baseProxy, c);
                Dictionary<string, string> c2 = cl.getContext();
                test(CollectionComparer.Equals(c, c2));
                output.WriteLine("ok");

                output.Write("testing ice_fixed... ");
                output.Flush();
                {
                    Connection connection = cl.GetConnection();
                    if (connection != null)
                    {
                        test(!cl.IsFixed);
                        Test.MyClassPrx prx = cl.Clone(fixedConnection: connection);
                        test(prx.IsFixed);
                        prx.IcePing();
                        try
                        {
                            cl.Clone(secure: true, fixedConnection: connection);
                            test(false);
                        }
                        catch (ArgumentException)
                        {
                        }
                        test(cl.Clone(facet: "facet", fixedConnection: connection).Facet.Equals("facet"));
                        test(cl.Clone(invocationMode: InvocationMode.Oneway, fixedConnection: connection).IsOneway);
                        Dictionary<string, string> ctx = new Dictionary<string, string>();
                        ctx["one"] = "hello";
                        ctx["two"] = "world";
                        test(cl.Clone(fixedConnection: connection).Context.Count == 0);
                        test(cl.Clone(context: ctx, fixedConnection: connection).Context.Count == 2);
                        test(cl.Clone(fixedConnection: connection).InvocationTimeout == -1);
                        test(cl.Clone(invocationTimeout: 10, fixedConnection: connection).InvocationTimeout == 10);
                        test(cl.Clone(fixedConnection: connection).GetConnection() == connection);
                        test(cl.Clone(fixedConnection: connection).Clone(fixedConnection: connection).GetConnection() == connection);
                        test(!cl.Clone(fixedConnection: connection).ConnectionTimeout.HasValue);
                        test(cl.Clone(compress: true, fixedConnection: connection).Compress.Value);
                        Connection fixedConnection = cl.Clone(connectionId: "ice_fixed").GetConnection();
                        test(cl.Clone(fixedConnection: connection).Clone(fixedConnection: fixedConnection).GetConnection() == fixedConnection);
                        try
                        {
                            cl.Clone(secure: !connection.getEndpoint().getInfo().secure(),
                                fixedConnection: connection);
                            test(false);
                        }
                        catch (ArgumentException)
                        {
                        }
                        try
                        {
                            cl.Clone(invocationMode: InvocationMode.Datagram, fixedConnection: connection);
                            test(false);
                        }
                        catch (ArgumentException)
                        {
                        }
                    }
                }
                output.WriteLine("ok");

                output.Write("testing encoding versioning... ");
                output.Flush();
                string ref20 = "test -e 2.0:" + helper.getTestEndpoint(0);
                Test.MyClassPrx cl20 = Test.MyClassPrx.Parse(ref20, communicator);
                try
                {
                    cl20.IcePing();
                    test(false);
                }
                catch (UnsupportedEncodingException)
                {
                    // Server 2.0 endpoint doesn't support 1.1 version.
                }

                string ref10 = "test -e 1.0:" + helper.getTestEndpoint(0);
                Test.MyClassPrx cl10 = Test.MyClassPrx.Parse(ref10, communicator);
                cl10.IcePing();
                cl10.Clone(encodingVersion: Util.Encoding_1_0).IcePing();
                cl.Clone(encodingVersion: Util.Encoding_1_0).IcePing();

                // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
                // call will use the 1.1 encoding
                string ref13 = "test -e 1.3:" + helper.getTestEndpoint(0);
                Test.MyClassPrx cl13 = Test.MyClassPrx.Parse(ref13, communicator);
                cl13.IcePing();
                cl13.IcePingAsync().Wait();

                try
                {
                    // Send request with bogus 1.2 encoding.
                    EncodingVersion version = new EncodingVersion(1, 2);
                    OutputStream os = new OutputStream(communicator);
                    os.startEncapsulation();
                    os.endEncapsulation();
                    byte[] inEncaps = os.finished();
                    inEncaps[4] = version.major;
                    inEncaps[5] = version.minor;
                    byte[] outEncaps;
                    cl.Invoke("ice_ping", OperationMode.Normal, inEncaps, out outEncaps);
                    test(false);
                }
                catch (UnknownLocalException ex)
                {
                    // The server thrown an UnsupportedEncodingException
                    test(ex.unknown.IndexOf("UnsupportedEncodingException") > 0);
                }

                try
                {
                    // Send request with bogus 2.0 encoding.
                    EncodingVersion version = new EncodingVersion(2, 0);
                    OutputStream os = new OutputStream(communicator);
                    os.startEncapsulation();
                    os.endEncapsulation();
                    byte[] inEncaps = os.finished();
                    inEncaps[4] = version.major;
                    inEncaps[5] = version.minor;
                    byte[] outEncaps;
                    cl.Invoke("ice_ping", OperationMode.Normal, inEncaps, out outEncaps);
                    test(false);
                }
                catch (UnknownLocalException ex)
                {
                    // The server thrown an UnsupportedEncodingException
                    test(ex.unknown.IndexOf("UnsupportedEncodingException") > 0);
                }

                output.WriteLine("ok");

                output.Write("testing protocol versioning... ");
                output.Flush();
                ref20 = "test -p 2.0:" + helper.getTestEndpoint(0);
                cl20 = Test.MyClassPrx.Parse(ref20, communicator);
                try
                {
                    cl20.IcePing();
                    test(false);
                }
                catch (UnsupportedProtocolException)
                {
                    // Server 2.0 proxy doesn't support 1.0 version.
                }

                ref10 = "test -p 1.0:" + helper.getTestEndpoint(0);
                cl10 = Test.MyClassPrx.Parse(ref10, communicator);
                cl10.IcePing();

                // 1.3 isn't supported but since a 1.3 proxy supports 1.1, the
                // call will use the 1.1 protocol
                ref13 = "test -p 1.3:" + helper.getTestEndpoint(0);
                cl13 = Test.MyClassPrx.Parse(ref13, communicator);
                cl13.IcePing();
                cl13.IcePingAsync().Wait();
                output.WriteLine("ok");

                output.Write("testing opaque endpoints... ");
                output.Flush();

                try
                {
                    // Invalid -x option
                    IObjectPrx.Parse("id:opaque -t 99 -v abcd -x abc", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Missing -t and -v
                    IObjectPrx.Parse("id:opaque", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Repeated -t
                    IObjectPrx.Parse("id:opaque -t 1 -t 1 -v abcd", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Repeated -v
                    IObjectPrx.Parse("id:opaque -t 1 -v abcd -v abcd", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Missing -t
                    IObjectPrx.Parse("id:opaque -v abcd", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Missing -v
                    IObjectPrx.Parse("id:opaque -t 1", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Missing arg for -t
                    IObjectPrx.Parse("id:opaque -t -v abcd", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Missing arg for -v
                    IObjectPrx.Parse("id:opaque -t 1 -v", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Not a number for -t
                    IObjectPrx.Parse("id:opaque -t x -v abcd", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // < 0 for -t
                    IObjectPrx.Parse("id:opaque -t -1 -v abcd", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Invalid char for -v
                    IObjectPrx.Parse("id:opaque -t 99 -v x?c", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                try
                {
                    // Invalid lenght for base64 input
                    IObjectPrx.Parse("id:opaque -t 99 -v xc", communicator);
                    test(false);
                }
                catch (EndpointParseException)
                {
                }

                // Legal TCP endpoint expressed as opaque endpoint
                var p1 = IObjectPrx.Parse("test -e 1.1:opaque -t 1 -e 1.0 -v CTEyNy4wLjAuMeouAAAQJwAAAA==",
                    communicator);
                string pstr = p1.ToString();
                test(pstr.Equals("test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000"));

                // Opaque endpoint encoded with 1.1 encoding.
                var p2 = IObjectPrx.Parse("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==",
                    communicator);
                test(p2.ToString().Equals("test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000"));

                if (communicator.getProperties().getPropertyAsInt("IPv6") == 0)
                {
                    // Working?
                    bool ssl = communicator.getProperties().getProperty("Default.Protocol").Equals("ssl");
                    bool tcp = communicator.getProperties().getProperty("Default.Protocol").Equals("tcp");

                    // Two legal TCP endpoints expressed as opaque endpoints
                    p1 = IObjectPrx.Parse("test -e 1.0:" + "" +
                        "opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:" +
                        "opaque -e 1.0 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==", communicator);
                    pstr = p1.ToString();
                    test(pstr.Equals("test -t -e 1.0:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000"));

                    // Test that an SSL endpoint and a nonsense endpoint get written back out as an opaque endpoint.
                    p1 = IObjectPrx.Parse("test -e 1.0:opaque -e 1.0 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -e 1.0 -t 99 -v abch",
                        communicator);
                    pstr = p1.ToString();
                    if (ssl)
                    {
                        test(pstr.Equals("test -t -e 1.0:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.0 -v abch"));
                    }
                    else if (tcp)
                    {
                        test(pstr.Equals(
                            "test -t -e 1.0:opaque -t 2 -e 1.0 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.0 -v abch"));
                    }
                }

                output.WriteLine("ok");

                output.Write("testing communicator shutdown/destroy... ");
                output.Flush();
                {
                    Communicator com = Util.initialize();
                    com.shutdown();
                    test(com.isShutdown());
                    com.waitForShutdown();
                    com.destroy();
                    com.shutdown();
                    test(com.isShutdown());
                    com.waitForShutdown();
                    com.destroy();
                }
                output.WriteLine("ok");

                return cl;
            }
        }
    }
}
