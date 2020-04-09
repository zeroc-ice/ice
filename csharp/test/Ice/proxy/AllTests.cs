//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice.proxy
{
    public class AllTests : global::Test.AllTests
    {
        public static Test.IMyClassPrx allTests(global::Test.TestHelper helper)
        {
            var communicator = helper.communicator();
            var output = helper.getWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            string rf = "test:" + helper.getTestEndpoint(0);
            var baseProxy = IObjectPrx.Parse(rf, communicator);
            test(baseProxy != null);

            var b1 = IObjectPrx.Parse("test", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.AdapterId.Length == 0 && b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("test ", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse(" test ", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse(" test", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("'test -f facet'", communicator);
            test(b1.Identity.Name.Equals("test -f facet") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            try
            {
                b1 = IObjectPrx.Parse("\"test -f facet'", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("\"test -f facet\"", communicator);
            test(b1.Identity.Name.Equals("test -f facet") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("\"test -f facet@test\"", communicator);
            test(b1.Identity.Name.Equals("test -f facet@test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("\"test -f facet@test @test\"", communicator);
            test(b1.Identity.Name.Equals("test -f facet@test @test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            try
            {
                b1 = IObjectPrx.Parse("test test", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test\\040test", communicator);
            test(b1.Identity.Name.Equals("test test") && b1.Identity.Category.Length == 0);
            try
            {
                b1 = IObjectPrx.Parse("test\\777", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test\\40test", communicator);
            test(b1.Identity.Name.Equals("test test"));

            // Test some octal and hex corner cases.
            b1 = IObjectPrx.Parse("test\\4test", communicator);
            test(b1.Identity.Name.Equals("test\u0004test"));
            b1 = IObjectPrx.Parse("test\\04test", communicator);
            test(b1.Identity.Name.Equals("test\u0004test"));
            b1 = IObjectPrx.Parse("test\\004test", communicator);
            test(b1.Identity.Name.Equals("test\u0004test"));
            b1 = IObjectPrx.Parse("test\\1114test", communicator);
            test(b1.Identity.Name.Equals("test\u00494test"));

            b1 = IObjectPrx.Parse("test\\b\\f\\n\\r\\t\\'\\\"\\\\test", communicator);
            test(b1.Identity.Name.Equals("test\b\f\n\r\t\'\"\\test") && b1.Identity.Category.Length == 0);

            b1 = IObjectPrx.Parse("category/test", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
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
            catch (FormatException)
            {
            }

            try
            {
                b1 = IObjectPrx.Parse("\"\"", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                b1 = IObjectPrx.Parse("\"\" test", communicator); // Invalid trailing characters.
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                b1 = IObjectPrx.Parse("test:", communicator); // Missing endpoint.
                test(false);
            }
            catch (FormatException)
            {
            }

            b1 = IObjectPrx.Parse("test@adapter", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.AdapterId.Equals("adapter"));
            try
            {
                b1 = IObjectPrx.Parse("id@adapter test", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("category/test@adapter", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("category/test@adapter:tcp", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter:tcp"));
            b1 = IObjectPrx.Parse("'category 1/test'@adapter", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category 1") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("'category/test 1'@adapter", communicator);
            test(b1.Identity.Name.Equals("test 1") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("'category/test'@'adapter 1'", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter 1"));
            b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@adapter", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category /test@foo") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@\"adapter:tcp\"", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category /test@foo") &&
                    b1.AdapterId.Equals("adapter:tcp"));

            b1 = IObjectPrx.Parse("id -f facet", communicator);
            test(b1.Identity.Name.Equals("id") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet"));
            b1 = IObjectPrx.Parse("id -f 'facet x'", communicator);
            test(b1.Identity.Name.Equals("id") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet x"));
            b1 = IObjectPrx.Parse("id -f \"facet x\"", communicator);
            test(b1.Identity.Name.Equals("id") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet x"));
            try
            {
                b1 = IObjectPrx.Parse("id -f \"facet x", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }
            try
            {
                b1 = IObjectPrx.Parse("id -f \'facet x", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test -f facet:tcp", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet") && b1.AdapterId.Length == 0);
            b1 = IObjectPrx.Parse("test -f \"facet:tcp\"", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet:tcp") && b1.AdapterId.Length == 0);
            b1 = IObjectPrx.Parse("test -f facet@test", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet") && b1.AdapterId.Equals("test"));
            b1 = IObjectPrx.Parse("test -f 'facet@test'", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet@test") && b1.AdapterId.Length == 0);
            b1 = IObjectPrx.Parse("test -f 'facet@test'@test", communicator);
            test(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet@test") && b1.AdapterId.Equals("test"));
            try
            {
                b1 = IObjectPrx.Parse("test -f facet@test @test", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test", communicator);
            test(!b1.IsOneway);
            b1 = IObjectPrx.Parse("test -t", communicator);
            test(!b1.IsOneway);
            b1 = IObjectPrx.Parse("test -o", communicator);
            test(b1.IsOneway);
            b1 = IObjectPrx.Parse("test -O", communicator);
            test(b1.InvocationMode == InvocationMode.BatchOneway);
            b1 = IObjectPrx.Parse("test -d", communicator);
            test(b1.InvocationMode == InvocationMode.Datagram);
            b1 = IObjectPrx.Parse("test -D", communicator);
            test(b1.InvocationMode == InvocationMode.BatchDatagram);
            b1 = IObjectPrx.Parse("test", communicator);

            test(b1.Encoding.Equals(Encoding.Latest));

            b1 = IObjectPrx.Parse("test -e 1.0", communicator);
            test(b1.Encoding.Major == 1 && b1.Encoding.Minor == 0);

            b1 = IObjectPrx.Parse("test -e 6.5", communicator);
            test(b1.Encoding.Major == 6 && b1.Encoding.Minor == 5);

            b1 = IObjectPrx.Parse("test -p ice1 -e 1.0", communicator);
            test(b1.ToString().Equals("test -t -p ice1 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p 1.0 -e 1.0", communicator);
            test(b1.ToString().Equals("test -t -p ice1 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p ice2 -e 1.0", communicator);
            test(b1.ToString().Equals("test -t -p ice2 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p 2.0 -e 1.0", communicator);
            test(b1.ToString().Equals("test -t -p ice2 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p 6 -e 1.0", communicator);
            test(b1.ToString().Equals("test -t -p 6 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p 6.0 -e 1.0", communicator);
            test(b1.ToString().Equals("test -t -p 6 -e 1.0"));

            try
            {
                IObjectPrx.Parse("test:tcp@adapterId", communicator);
                test(false);
            }
            catch (FormatException)
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
            catch (FormatException)
            {
            }

            //
            // Test invalid endpoint syntax
            //
            try
            {
                communicator.CreateObjectAdapterWithEndpoints("BadAdapter", " : ");
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                communicator.CreateObjectAdapterWithEndpoints("BadAdapter", "tcp: ");
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                communicator.CreateObjectAdapterWithEndpoints("BadAdapter", ":tcp");
                test(false);
            }
            catch (FormatException)
            {
            }

            //
            // Test for bug ICE-5543: escaped escapes in stringToIdentity
            //
            var id = new Identity("test", ",X2QNUAzSBcJ_e$AV;E\\");
            var id2 = Identity.Parse(id.ToString(communicator.ToStringMode));
            test(id.Equals(id2));

            id = new Identity("test", ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\");
            id2 = Identity.Parse(id.ToString(communicator.ToStringMode));
            test(id.Equals(id2));

            id = new Identity("/test", "cat/");
            string idStr = id.ToString(communicator.ToStringMode);

            test(idStr == "cat\\//\\/test");
            id2 = Identity.Parse(idStr);
            test(id.Equals(id2));

            // Input string with various pitfalls
            id = Identity.Parse("\\342\\x82\\254\\60\\x9\\60\\");
            test(id.Name == "€0\t0\\" && id.Category == "");

            try
            {
                // Illegal character < 32
                id = Identity.Parse("xx\01FooBar");
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Illegal surrogate
                id = Identity.Parse("xx\\ud911");
                test(false);
            }
            catch (FormatException)
            {
            }

            // Testing bytes 127(\x7F, \177) and €
            id = new Identity("test", "\x7f€");

            idStr = id.ToString(ToStringMode.Unicode);
            test(idStr == "\\u007f€/test");
            id2 = Identity.Parse(idStr);
            test(id.Equals(id2));
            test(id.ToString() == idStr);

            idStr = id.ToString(ToStringMode.ASCII);
            test(idStr == "\\u007f\\u20ac/test");
            id2 = Identity.Parse(idStr);
            test(id.Equals(id2));

            idStr = id.ToString(ToStringMode.Compat);
            test(idStr == "\\177\\342\\202\\254/test");
            id2 = Identity.Parse(idStr);
            test(id.Equals(id2));

            id2 = Identity.Parse(id.ToString(communicator.ToStringMode));
            test(id.Equals(id2));

            // More unicode character
            id = new Identity("banana \x0E-\ud83c\udf4c\u20ac\u00a2\u0024", "greek \ud800\udd6a");

            idStr = id.ToString(ToStringMode.Unicode);
            test(idStr == "greek \ud800\udd6a/banana \\u000e-\ud83c\udf4c\u20ac\u00a2$");
            id2 = Identity.Parse(idStr);
            test(id.Equals(id2));

            idStr = id.ToString(ToStringMode.ASCII);
            test(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$");
            id2 = Identity.Parse(idStr);
            test(id.Equals(id2));

            idStr = id.ToString(ToStringMode.Compat);
            id2 = Identity.Parse(idStr);
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
                b2 = b1.GetConnection().CreateProxy(Identity.Parse("fixed"), IObjectPrx.Factory);
                string str = b2.ToString();
                test(b2.ToString() == str);
            }
            output.WriteLine("ok");

            output.Write("testing propertyToProxy... ");
            output.Flush();

            string propertyPrefix = "Foo.Proxy";
            communicator.SetProperty(propertyPrefix, "test:" + helper.getTestEndpoint(0));
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1 != null &&
                    b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.AdapterId.Length == 0 && b1.Facet.Length == 0);

            string property;

            property = propertyPrefix + ".Locator";
            test(b1.Locator == null);
            communicator.SetProperty(property, "locator:default -p 10000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1 != null && b1.Locator != null && b1.Locator.Identity.Name.Equals("locator"));
            communicator.SetProperty(property, "");
            property = propertyPrefix + ".LocatorCacheTimeout";
            test(b1.LocatorCacheTimeout == -1);
            communicator.SetProperty(property, "1");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1 != null && b1.LocatorCacheTimeout == 1);
            communicator.SetProperty(property, "");

            // Now retest with an indirect proxy.
            communicator.SetProperty(propertyPrefix, "test");
            property = propertyPrefix + ".Locator";
            communicator.SetProperty(property, "locator:default -p 10000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1 != null && b1.Locator != null && b1.Locator.Identity.Name.Equals("locator"));
            communicator.SetProperty(property, "");

            property = propertyPrefix + ".LocatorCacheTimeout";
            test(b1.LocatorCacheTimeout == -1);
            communicator.SetProperty(property, "1");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1 != null && b1.LocatorCacheTimeout == 1);
            communicator.SetProperty(property, "");

            // This cannot be tested so easily because the property is cached
            // on communicator initialization.
            //
            //communicator.SetProperty("Default.LocatorCacheTimeout", "60");
            //b1 = communicator.propertyToProxy(propertyPrefix);
            //test(b1.LocatorCacheTimeout == 60);
            //communicator.SetProperty("Default.LocatorCacheTimeout", "");

            communicator.SetProperty(propertyPrefix, "test:" + helper.getTestEndpoint(0));

            property = propertyPrefix + ".Router";
            test(b1.Router == null);
            communicator.SetProperty(property, "router:default -p 10000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1.Router != null && b1.Router.Identity.Name.Equals("router"));
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".PreferNonSecure";
            test(b1.PreferNonSecure);
            communicator.SetProperty(property, "0");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(!b1.PreferNonSecure);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".ConnectionCached";
            test(b1.IsConnectionCached);
            communicator.SetProperty(property, "0");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(!b1.IsConnectionCached);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".InvocationTimeout";
            test(b1.InvocationTimeout == -1);
            communicator.SetProperty(property, "1000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1.InvocationTimeout == 1000);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".EndpointSelection";
            test(b1.EndpointSelection == EndpointSelectionType.Random);
            communicator.SetProperty(property, "Random");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1.EndpointSelection == EndpointSelectionType.Random);
            communicator.SetProperty(property, "Ordered");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1.EndpointSelection == EndpointSelectionType.Ordered);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".CollocationOptimized";
            test(b1.IsCollocationOptimized);
            communicator.SetProperty(property, "0");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(!b1.IsCollocationOptimized);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".Context.c1";
            test(!b1.Context.ContainsKey("c1"));
            communicator.SetProperty(property, "TEST");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1.Context["c1"].Equals("TEST"));

            property = propertyPrefix + ".Context.c2";
            test(!b1.Context.ContainsKey("c2"));
            communicator.SetProperty(property, "TEST");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            test(b1.Context["c2"].Equals("TEST"));

            communicator.SetProperty(propertyPrefix + ".Context.c1", "");
            communicator.SetProperty(propertyPrefix + ".Context.c2", "");

            output.WriteLine("ok");

            output.Write("testing proxyToProperty... ");
            output.Flush();

            var router = IRouterPrx.Parse("router", communicator).Clone(
                collocationOptimized: false,
                cacheConnection: true,
                preferNonSecure: true,
                endpointSelection: EndpointSelectionType.Random,
                locatorCacheTimeout: 200,
                invocationTimeout: 1500);

            var locator = ILocatorPrx.Parse("locator", communicator).Clone(
                collocationOptimized: true,
                cacheConnection: false,
                preferNonSecure: true,
                endpointSelection: EndpointSelectionType.Random,
                locatorCacheTimeout: 300,
                invocationTimeout: 1500,
                router: router);

            b1 = IObjectPrx.Parse("test", communicator).Clone(
                collocationOptimized: true,
                cacheConnection: true,
                preferNonSecure: false,
                endpointSelection: EndpointSelectionType.Ordered,
                locatorCacheTimeout: 100,
                invocationTimeout: 1234,
                encoding: Encoding.V2_0,
                locator: locator);

            Dictionary<string, string> proxyProps = b1.ToProperty("Test");
            test(proxyProps.Count == 21);

            test(proxyProps["Test"].Equals("test -t -p ice1 -e 2.0"));
            test(proxyProps["Test.CollocationOptimized"].Equals("1"));
            test(proxyProps["Test.ConnectionCached"].Equals("1"));
            test(proxyProps["Test.PreferNonSecure"].Equals("0"));
            test(proxyProps["Test.EndpointSelection"].Equals("Ordered"));
            test(proxyProps["Test.LocatorCacheTimeout"].Equals("100"));
            test(proxyProps["Test.InvocationTimeout"].Equals("1234"));

            test(proxyProps["Test.Locator"].Equals($"locator -t -p ice1 -e {Encoding.V2_0}"));
            // Locator collocation optimization is always disabled.
            //test(proxyProps["Test.Locator.CollocationOptimized"].Equals("1"));
            test(proxyProps["Test.Locator.ConnectionCached"].Equals("0"));
            test(proxyProps["Test.Locator.PreferNonSecure"].Equals("1"));
            test(proxyProps["Test.Locator.EndpointSelection"].Equals("Random"));
            test(proxyProps["Test.Locator.LocatorCacheTimeout"].Equals("300"));
            test(proxyProps["Test.Locator.InvocationTimeout"].Equals("1500"));

            test(proxyProps["Test.Locator.Router"].Equals(
                        "router -t -p ice1 -e " + Encoding.Latest.ToString()));
            test(proxyProps["Test.Locator.Router.CollocationOptimized"].Equals("0"));
            test(proxyProps["Test.Locator.Router.ConnectionCached"].Equals("1"));
            test(proxyProps["Test.Locator.Router.PreferNonSecure"].Equals("1"));
            test(proxyProps["Test.Locator.Router.EndpointSelection"].Equals("Random"));
            test(proxyProps["Test.Locator.Router.LocatorCacheTimeout"].Equals("200"));
            test(proxyProps["Test.Locator.Router.InvocationTimeout"].Equals("1500"));

            output.WriteLine("ok");

            output.Write("testing ice_getCommunicator... ");
            output.Flush();
            test(baseProxy.Communicator == communicator);
            output.WriteLine("ok");

            output.Write("testing proxy methods... ");

            test(baseProxy.Clone(facet: "facet", IObjectPrx.Factory).Facet.Equals("facet"));
            test(baseProxy.Clone(adapterId: "id").AdapterId.Equals("id"));
            test(!baseProxy.Clone(invocationMode: InvocationMode.Twoway).IsOneway);
            test(baseProxy.Clone(invocationMode: InvocationMode.Oneway).IsOneway);
            test(baseProxy.Clone(invocationMode: InvocationMode.Datagram).IsOneway);
            test(baseProxy.Clone(invocationMode: InvocationMode.BatchOneway).InvocationMode == InvocationMode.BatchOneway);
            test(baseProxy.Clone(invocationMode: InvocationMode.Datagram).InvocationMode == InvocationMode.Datagram);
            test(baseProxy.Clone(invocationMode: InvocationMode.BatchDatagram).InvocationMode == InvocationMode.BatchDatagram);
            test(baseProxy.Clone(collocationOptimized: true).IsCollocationOptimized);
            test(!baseProxy.Clone(collocationOptimized: false).IsCollocationOptimized);
            test(baseProxy.Clone(preferNonSecure: true).PreferNonSecure);
            test(!baseProxy.Clone(preferNonSecure: false).PreferNonSecure);

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
            }
            catch (ArgumentException)
            {
                test(false);
            }

            try
            {
                baseProxy.Clone(invocationTimeout: -2);
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

            test(object.Equals(IObjectPrx.Parse("foo", communicator), IObjectPrx.Parse("foo", communicator)));
            test(!IObjectPrx.Parse("foo", communicator).Equals(IObjectPrx.Parse("foo2", communicator)));

            var compObj = IObjectPrx.Parse("foo", communicator);

            test(compObj.Clone(facet: "facet", IObjectPrx.Factory).Equals(
                compObj.Clone(facet: "facet", IObjectPrx.Factory)));
            test(!compObj.Clone(facet: "facet", IObjectPrx.Factory).Equals(
                compObj.Clone(facet: "facet1", IObjectPrx.Factory)));

            test(compObj.Clone(invocationMode: InvocationMode.Oneway).Equals(
                compObj.Clone(invocationMode: InvocationMode.Oneway)));
            test(!compObj.Clone(invocationMode: InvocationMode.Oneway).Equals(
                compObj.Clone(invocationMode: InvocationMode.Twoway)));

            test(compObj.Clone(collocationOptimized: true).Equals(compObj.Clone(collocationOptimized: true)));
            test(!compObj.Clone(collocationOptimized: false).Equals(compObj.Clone(collocationOptimized: true)));

            test(compObj.Clone(cacheConnection: true).Equals(compObj.Clone(cacheConnection: true)));
            test(!compObj.Clone(cacheConnection: false).Equals(compObj.Clone(cacheConnection: true)));

            test(compObj.Clone(endpointSelection: EndpointSelectionType.Random).Equals(
                compObj.Clone(endpointSelection: EndpointSelectionType.Random)));
            test(!compObj.Clone(endpointSelection: EndpointSelectionType.Random).Equals(
                compObj.Clone(endpointSelection: EndpointSelectionType.Ordered)));

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

            ILocatorPrx loc1 = ILocatorPrx.Parse("loc1:default -p 10000", communicator);
            ILocatorPrx loc2 = ILocatorPrx.Parse("loc2:default -p 10000", communicator);
            test(compObj.Clone(clearLocator: true).Equals(compObj.Clone(clearLocator: true)));
            test(compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc1)));
            test(!compObj.Clone(locator: loc1).Equals(compObj.Clone(clearLocator: true)));
            test(!compObj.Clone(clearLocator: true).Equals(compObj.Clone(locator: loc2)));
            test(!compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc2)));

            IRouterPrx rtr1 = IRouterPrx.Parse("rtr1:default -p 10000", communicator);
            IRouterPrx rtr2 = IRouterPrx.Parse("rtr2:default -p 10000", communicator);
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

            test(compObj.Clone(preferNonSecure: true).Equals(compObj.Clone(preferNonSecure: true)));
            test(!compObj.Clone(preferNonSecure: true).Equals(compObj.Clone(preferNonSecure: false)));

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
            Test.IMyClassPrx cl = Test.IMyClassPrx.CheckedCast(baseProxy);
            test(cl != null);
            Test.IMyDerivedClassPrx derived = Test.IMyDerivedClassPrx.CheckedCast(cl);
            test(derived != null);
            test(cl.Equals(baseProxy));
            test(derived.Equals(baseProxy));
            test(cl.Equals(derived));
            try
            {
                Test.IMyDerivedClassPrx.CheckedCast(cl.Clone(facet: "facet", IObjectPrx.Factory));
                test(false);
            }
            catch (ObjectNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing checked cast with context... ");
            output.Flush();

            Dictionary<string, string> c = cl.getContext();
            test(c == null || c.Count == 0);

            c = new Dictionary<string, string>();
            c["one"] = "hello";
            c["two"] = "world";
            cl = Test.IMyClassPrx.CheckedCast(baseProxy, c);
            Dictionary<string, string> c2 = cl.getContext();
            test(global::Test.Collections.Equals(c, c2));
            output.WriteLine("ok");

            output.Write("testing ice_fixed... ");
            output.Flush();
            {
                Connection connection = cl.GetConnection();
                if (connection != null)
                {
                    test(!cl.IsFixed);
                    Test.IMyClassPrx prx = cl.Clone(fixedConnection: connection);
                    test(prx.IsFixed);
                    prx.IcePing();
                    test(cl.Clone("facet", IObjectPrx.Factory, fixedConnection: connection).Facet.Equals("facet"));
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
            string ref13 = "test -e 1.3:" + helper.getTestEndpoint(0);
            Test.IMyClassPrx cl13 = Test.IMyClassPrx.Parse(ref13, communicator);
            try
            {
                cl13.IcePing();
                test(false);
            }
            catch (NotSupportedException)
            {
                // expected
            }
            output.WriteLine("ok");

            output.Write("testing protocol versioning... ");
            output.Flush();
            string ref3 = "test -p 3:" + helper.getTestEndpoint(0);
            var cl3 = Test.IMyClassPrx.Parse(ref3, communicator);
            try
            {
                cl3.IcePing();
                test(false);
            }
            catch (NotSupportedException)
            {
                // expected
            }
            output.WriteLine("ok");

            output.Write("testing opaque endpoints... ");
            output.Flush();

            try
            {
                // Invalid -x option
                IObjectPrx.Parse("id:opaque -t 99 -v abcd -x abc", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing -t and -v
                IObjectPrx.Parse("id:opaque", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Repeated -t
                IObjectPrx.Parse("id:opaque -t 1 -t 1 -v abcd", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Repeated -v
                IObjectPrx.Parse("id:opaque -t 1 -v abcd -v abcd", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing -t
                IObjectPrx.Parse("id:opaque -v abcd", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing -v
                IObjectPrx.Parse("id:opaque -t 1", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing arg for -t
                IObjectPrx.Parse("id:opaque -t -v abcd", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing arg for -v
                IObjectPrx.Parse("id:opaque -t 1 -v", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Not a number for -t
                IObjectPrx.Parse("id:opaque -t x -v abcd", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // < 0 for -t
                IObjectPrx.Parse("id:opaque -t -1 -v abcd", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Invalid char for -v
                IObjectPrx.Parse("id:opaque -t 99 -v x?c", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Invalid lenght for base64 input
                IObjectPrx.Parse("id:opaque -t 99 -v xc", communicator);
                test(false);
            }
            catch (FormatException)
            {
            }

            // Legal TCP endpoint expressed as opaque endpoint
            // Opaque endpoint encoded with 1.1 encoding.
            var p1 = IObjectPrx.Parse("test -e 1.1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==",
                communicator);
            test(p1.ToString().Equals("test -t -p ice1 -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000"));

            if ((communicator.GetPropertyAsInt("Ice.IPv6") ?? 0) == 0)
            {
                // Working?
                bool ssl = communicator.GetProperty("Ice.Default.Transport") == "ssl";
                bool tcp = communicator.GetProperty("Ice.Default.Transport") == "tcp";

                // Two legal TCP endpoints expressed as opaque endpoints
                p1 = IObjectPrx.Parse("test -e 1.1:" + "" +
                    "opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:" +
                    "opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==", communicator);
                var pstr = p1.ToString();
                test(pstr.Equals("test -t -p ice1 -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000"));

                // Test that an SSL endpoint and a nonsense endpoint get written back out as an opaque endpoint.
                p1 = IObjectPrx.Parse("test -e 1.1:opaque -e 1.1 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -e 1.1 -t 99 -v abch",
                    communicator);
                pstr = p1.ToString();
                if (ssl)
                {
                    test(pstr.Equals("test -t -p ice1 -e 1.1:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.1 -v abch"));
                }
                else if (tcp)
                {
                    test(pstr.Equals(
                        "test -t -p ice1 -e 1.1:opaque -t 2 -e 1.1 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -t 99 -e 1.1 -v abch"));
                }
            }

            output.WriteLine("ok");

            output.Write("testing communicator shutdown/destroy... ");
            output.Flush();
            {
                Communicator com = new Communicator();
                com.Shutdown();
                test(com.IsShutdown());
                com.WaitForShutdown();
                com.Destroy();
                com.Shutdown();
                test(com.IsShutdown());
                com.WaitForShutdown();
                com.Destroy();
            }
            output.WriteLine("ok");

            return cl;
        }
    }
}
