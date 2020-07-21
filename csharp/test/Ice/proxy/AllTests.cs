//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Proxy
{
    public class AllTests
    {
        public static IMyClassPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            System.IO.TextWriter output = helper.GetWriter();
            output.Write("testing proxy parsing (URI)... ");
            output.Flush();

            string[] uriStringArray =
            {
                "ice+tcp://host.zeroc.com/identity#facet",
                "ice+tcp://host.zeroc.com:1000/category/name",
                "ice+tcp://host.zeroc.com/category/name%20with%20space",
                "ice+ws://host.zeroc.com//identity",
                "ice+ws://host.zeroc.com//identity?alt-endpoint=host2.zeroc.com",
                "ice+ws://host.zeroc.com//identity?alt-endpoint=host2.zeroc.com:10000",
                "ice+ws://host.zeroc.com//identity?protocol=ice1&source-address=[::2]&compress=true",
                "ice+ws://host.zeroc.com//identity?protocol=ice1&alt-endpoint=host2.zeroc.com?" +
                    "source-address=10.1.2.4$compress=true",
                "ice+tcp://[::1]:10000/identity?alt-endpoint=host1:10000,host2,host3,host4",
                "ice+ssl://[::1]:10000/identity?alt-endpoint=host1:10000&alt-endpoint=host2,host3&alt-endpoint=[::2]",
                "ice:location//identity#facet",
                "ice+tcp://host.zeroc.com//identity",
                "ice+tcp://host.zeroc.com:/identity", // another syntax for empty port
                "ice+universal://com.zeroc.ice/identity?transport=iaps&option=a,b%2Cb,c&option=d",
                "ice+universal://host.zeroc.com/identity?transport=100",
                "ice+universal://[::ab:cd:ef:00]/identity?transport=bt", // leading :: to make the address IPv6-like
                "ice+udp://[::1]/test?source-address=::1&interface=0:0:0:0:0:0:0:1%25lo", // unescaped as ..:1%lo
                "ice+ws://host.zeroc.com/identity?resource=/foo%2Fbar?/xyz",
                "ice+universal://host.zeroc.com:10000/identity?transport=tcp",
                "ice+universal://host.zeroc.com/identity?transport=ws&option=/foo%2520/bar",
            };

            foreach (string uriString in uriStringArray)
            {
                _ = IObjectPrx.Parse(uriString, communicator);
                // output.WriteLine($"{uriString} = {prx}");
            }

            string[] badUriStringArray =
            {
                "ice+tcp://host.zeroc.com:foo",     // missing host
                "ice+tcp:identity?protocol=invalid", // invalid protocol
                "ice+universal://host.zeroc.com", // missing transport
                "ice+universal://host.zeroc.com?transport=100&protocol=ice1", // invalid protocol
                "ice://host:1000/identity", // host not allowed
                "ice+universal:/identity" // missing host
            };

            foreach (string uriString in badUriStringArray)
            {
                try
                {
                    _ = IObjectPrx.Parse(uriString, communicator);
                    TestHelper.Assert(false);
                }
                catch (FormatException)
                {
                    // expected
                }
            }
            output.WriteLine("ok");

            output.Write("testing proxy parsing (old format)... ");

            string rf = "test:" + helper.GetTestEndpoint(0);
            var baseProxy = IObjectPrx.Parse(rf, communicator);
            TestHelper.Assert(baseProxy != null);

            IObjectPrx? b1;
            b1 = IObjectPrx.Parse("test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                              b1.AdapterId.Length == 0 && b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("test ", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse(" test ", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                              b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse(" test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("'test -f facet'", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test -f facet") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            try
            {
                b1 = IObjectPrx.Parse("\"test -f facet'", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("\"test -f facet\"", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test -f facet") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("\"test -f facet@test\"", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test -f facet@test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("\"test -f facet@test @test\"", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test -f facet@test @test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            try
            {
                b1 = IObjectPrx.Parse("test test", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test\\040test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test test") && b1.Identity.Category.Length == 0);
            try
            {
                b1 = IObjectPrx.Parse("test\\777", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test\\40test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test test"));

            // Test some octal and hex corner cases.
            b1 = IObjectPrx.Parse("test\\4test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test\u0004test"));
            b1 = IObjectPrx.Parse("test\\04test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test\u0004test"));
            b1 = IObjectPrx.Parse("test\\004test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test\u0004test"));
            b1 = IObjectPrx.Parse("test\\1114test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test\u00494test"));

            b1 = IObjectPrx.Parse("test\\b\\f\\n\\r\\t\\'\\\"\\\\test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test\b\f\n\r\t\'\"\\test") && b1.Identity.Category.Length == 0);

            b1 = IObjectPrx.Parse("category/test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Length == 0);

            b1 = IObjectPrx.Parse("test:tcp --sourceAddress \"::1\"", communicator);
            TestHelper.Assert(b1.Equals(IObjectPrx.Parse(b1.ToString()!, communicator)));

            b1 = IObjectPrx.Parse("test:udp --sourceAddress \"::1\" --interface \"0:0:0:0:0:0:0:1%lo\"", communicator);
            TestHelper.Assert(b1.Equals(IObjectPrx.Parse(b1.ToString()!, communicator)));

            try
            {
                b1 = IObjectPrx.Parse("", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                b1 = IObjectPrx.Parse("\"\"", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                b1 = IObjectPrx.Parse("\"\" test", communicator); // Invalid trailing characters.
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                b1 = IObjectPrx.Parse("test:", communicator); // Missing endpoint.
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            b1 = IObjectPrx.Parse("test@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.AdapterId.Equals("adapter"));
            try
            {
                b1 = IObjectPrx.Parse("id@adapter test", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("category/test@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("category/test@adapter:tcp", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter:tcp"));
            b1 = IObjectPrx.Parse("'category 1/test'@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category 1") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("'category/test 1'@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test 1") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("'category/test'@'adapter 1'", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category") &&
                    b1.AdapterId.Equals("adapter 1"));
            b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category /test@foo") &&
                    b1.AdapterId.Equals("adapter"));
            b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@\"adapter:tcp\"", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Equals("category /test@foo") &&
                    b1.AdapterId.Equals("adapter:tcp"));

            b1 = IObjectPrx.Parse("id -f facet", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("id") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet"));
            b1 = IObjectPrx.Parse("id -f 'facet x'", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("id") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet x"));
            b1 = IObjectPrx.Parse("id -f \"facet x\"", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("id") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet x"));
            try
            {
                b1 = IObjectPrx.Parse("id -f \"facet x", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            try
            {
                b1 = IObjectPrx.Parse("id -f \'facet x", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test -f facet:tcp", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet") && b1.AdapterId.Length == 0);
            b1 = IObjectPrx.Parse("test -f \"facet:tcp\"", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet:tcp") && b1.AdapterId.Length == 0);
            b1 = IObjectPrx.Parse("test -f facet@test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet") && b1.AdapterId.Equals("test"));
            b1 = IObjectPrx.Parse("test -f 'facet@test'", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet@test") && b1.AdapterId.Length == 0);
            b1 = IObjectPrx.Parse("test -f 'facet@test'@test", communicator);
            TestHelper.Assert(b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Equals("facet@test") && b1.AdapterId.Equals("test"));
            try
            {
                b1 = IObjectPrx.Parse("test -f facet@test @test", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test", communicator);
            TestHelper.Assert(!b1.IsOneway);
            b1 = IObjectPrx.Parse("test -t", communicator);
            TestHelper.Assert(!b1.IsOneway);
            b1 = IObjectPrx.Parse("test -o", communicator);
            TestHelper.Assert(b1.IsOneway);
            b1 = IObjectPrx.Parse("test -O", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.BatchOneway);
            b1 = IObjectPrx.Parse("test -d", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.Datagram);
            b1 = IObjectPrx.Parse("test -D", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.BatchDatagram);
            b1 = IObjectPrx.Parse("test", communicator);

            TestHelper.Assert(b1.Encoding.Equals(communicator.DefaultEncoding));

            b1 = IObjectPrx.Parse("test -e 1.0", communicator);
            TestHelper.Assert(b1.Encoding.Major == 1 && b1.Encoding.Minor == 0);

            b1 = IObjectPrx.Parse("test -e 6.5", communicator);
            TestHelper.Assert(b1.Encoding.Major == 6 && b1.Encoding.Minor == 5);

            b1 = IObjectPrx.Parse("test -p ice1 -e 1.0", communicator);
            TestHelper.Assert(b1.ToString()!.Equals("test -t -p ice1 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p ice1 -e 1.0", communicator);
            TestHelper.Assert(b1.ToString()!.Equals("test -t -p ice1 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p ice2 -e 1.0", communicator);
            TestHelper.Assert(b1.ToString()!.Equals("test -t -p ice2 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p 2.0 -e 1.0", communicator);
            TestHelper.Assert(b1.ToString()!.Equals("test -t -p ice2 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p 6 -e 1.0", communicator);
            TestHelper.Assert(b1.ToString()!.Equals("test -t -p 6 -e 1.0"));

            b1 = IObjectPrx.Parse("test -p 6.0 -e 1.0", communicator);
            TestHelper.Assert(b1.ToString()!.Equals("test -t -p 6 -e 1.0"));

            try
            {
                IObjectPrx.Parse("test:tcp@adapterId", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            // This is an unknown endpoint warning, not a parse exception.
            //
            //try
            //{
            //   b1 = communicator.stringToProxy("test -f the:facet:tcp");
            //   TestHelper.Assert(false);
            //}
            //catch(EndpointParseException)
            //{
            //}
            try
            {
                IObjectPrx.Parse("test: :tcp", communicator);
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                communicator.CreateObjectAdapterWithEndpoints("BadAdapter", "tcp: ");
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                communicator.CreateObjectAdapterWithEndpoints("BadAdapter", ":tcp");
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            //
            // Test for bug ICE-5543: escaped escapes in stringToIdentity
            //
            var id = new Identity("test", ",X2QNUAzSBcJ_e$AV;E\\");
            var id2 = Identity.Parse(id.ToString(communicator.ToStringMode));
            TestHelper.Assert(id.Equals(id2));

            id = new Identity("test", ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\");
            id2 = Identity.Parse(id.ToString(communicator.ToStringMode));
            TestHelper.Assert(id.Equals(id2));

            id = new Identity("/test", "cat/");
            string idStr = id.ToString(communicator.ToStringMode);

            TestHelper.Assert(idStr == "cat\\//\\/test");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id.Equals(id2));

            // Input string with various pitfalls
            id = Identity.Parse("\\342\\x82\\254\\60\\x9\\60\\");
            TestHelper.Assert(id.Name == "€0\t0\\" && id.Category.Length == 0);

            try
            {
                // Illegal character < 32
                id = Identity.Parse("xx\01FooBar");
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Illegal surrogate
                id = Identity.Parse("xx\\ud911");
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            // Testing bytes 127(\x7F, \177) and €
            id = new Identity("test", "\x7f€");

            idStr = id.ToString(ToStringMode.Unicode);
            TestHelper.Assert(idStr == "\\u007f€/test");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id.Equals(id2));
            TestHelper.Assert(id.ToString() == idStr);

            idStr = id.ToString(ToStringMode.ASCII);
            TestHelper.Assert(idStr == "\\u007f\\u20ac/test");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id.Equals(id2));

            idStr = id.ToString(ToStringMode.Compat);
            TestHelper.Assert(idStr == "\\177\\342\\202\\254/test");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id.Equals(id2));

            id2 = Identity.Parse(id.ToString(communicator.ToStringMode));
            TestHelper.Assert(id.Equals(id2));

            // More unicode character
            id = new Identity("banana \x0E-\ud83c\udf4c\u20ac\u00a2\u0024", "greek \ud800\udd6a");

            idStr = id.ToString(ToStringMode.Unicode);
            TestHelper.Assert(idStr == "greek \ud800\udd6a/banana \\u000e-\ud83c\udf4c\u20ac\u00a2$");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id.Equals(id2));

            idStr = id.ToString(ToStringMode.ASCII);
            TestHelper.Assert(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id.Equals(id2));

            idStr = id.ToString(ToStringMode.Compat);
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(idStr == "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$");
            TestHelper.Assert(id.Equals(id2));

            output.WriteLine("ok");

            output.Write("testing proxyToString... ");
            output.Flush();
            b1 = IObjectPrx.Parse(rf, communicator);
            var b2 = IObjectPrx.Parse(b1.ToString()!, communicator);
            TestHelper.Assert(b1.Equals(b2));

            if (b1.GetConnection() is Connection connection) // not coloc-optimized target
            {
                b2 = connection.CreateProxy(Identity.Parse("fixed"), IObjectPrx.Factory);
                TestHelper.Assert(b2.ToString() == b2.ToString());
            }
            output.WriteLine("ok");

            output.Write("testing propertyToProxy... ");
            output.Flush();

            string propertyPrefix = "Foo.Proxy";
            communicator.SetProperty(propertyPrefix, "test:" + helper.GetTestEndpoint(0));
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null &&
                    b1.Identity.Name.Equals("test") && b1.Identity.Category.Length == 0 &&
                    b1.AdapterId.Length == 0 && b1.Facet.Length == 0);

            string property;

            property = propertyPrefix + ".Locator";
            TestHelper.Assert(b1.Locator == null);
            communicator.SetProperty(property, "locator:default -p 10000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.Locator != null && b1.Locator.Identity.Name.Equals("locator"));
            communicator.SetProperty(property, "");
            property = propertyPrefix + ".LocatorCacheTimeout";
            TestHelper.Assert(b1.LocatorCacheTimeout == Timeout.InfiniteTimeSpan);
            communicator.SetProperty(property, "1s");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.LocatorCacheTimeout == TimeSpan.FromSeconds(1));
            communicator.SetProperty(property, "");

            // Now retest with an indirect proxy.
            communicator.SetProperty(propertyPrefix, "test");
            property = propertyPrefix + ".Locator";
            communicator.SetProperty(property, "locator:default -p 10000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.Locator != null && b1.Locator.Identity.Name.Equals("locator"));
            communicator.SetProperty(property, "");

            property = propertyPrefix + ".LocatorCacheTimeout";
            TestHelper.Assert(b1.LocatorCacheTimeout == Timeout.InfiniteTimeSpan);
            communicator.SetProperty(property, "1s");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.LocatorCacheTimeout == TimeSpan.FromSeconds(1));
            communicator.SetProperty(property, "");

            // This cannot be tested so easily because the property is cached
            // on communicator initialization.
            //
            //communicator.SetProperty("Default.LocatorCacheTimeout", "60");
            //b1 = communicator.propertyToProxy(propertyPrefix);
            //TestHelper.Assert(b1.LocatorCacheTimeout == 60);
            //communicator.SetProperty("Default.LocatorCacheTimeout", "");

            communicator.SetProperty(propertyPrefix, "test:" + helper.GetTestEndpoint(0));

            property = propertyPrefix + ".Router";
            TestHelper.Assert(b1.Router == null);
            communicator.SetProperty(property, "router:default -p 10000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.Router != null && b1.Router.Identity.Name.Equals("router"));
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".PreferNonSecure";
            TestHelper.Assert(b1.PreferNonSecure);
            communicator.SetProperty(property, "0");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && !b1.PreferNonSecure);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".ConnectionCached";
            TestHelper.Assert(b1.IsConnectionCached);
            communicator.SetProperty(property, "0");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && !b1.IsConnectionCached);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".InvocationTimeout";
            TestHelper.Assert(b1.InvocationTimeout == -1);
            communicator.SetProperty(property, "1000");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.InvocationTimeout == 1000);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".EndpointSelection";
            TestHelper.Assert(b1.EndpointSelection == EndpointSelectionType.Random);
            communicator.SetProperty(property, "Random");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.EndpointSelection == EndpointSelectionType.Random);
            communicator.SetProperty(property, "Ordered");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.EndpointSelection == EndpointSelectionType.Ordered);
            communicator.RemoveProperty(property);

            property = propertyPrefix + ".Context.c1";
            TestHelper.Assert(!b1.Context.ContainsKey("c1"));
            communicator.SetProperty(property, "TEST");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.Context["c1"].Equals("TEST"));

            property = propertyPrefix + ".Context.c2";
            TestHelper.Assert(!b1.Context.ContainsKey("c2"));
            communicator.SetProperty(property, "TEST");
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory);
            TestHelper.Assert(b1 != null && b1.Context["c2"].Equals("TEST"));

            communicator.SetProperty(propertyPrefix + ".Context.c1", "");
            communicator.SetProperty(propertyPrefix + ".Context.c2", "");

            output.WriteLine("ok");

            output.Write("testing proxyToProperty... ");
            output.Flush();

            var router = IRouterPrx.Parse("router", communicator).Clone(
                cacheConnection: true,
                preferNonSecure: true,
                endpointSelection: EndpointSelectionType.Random,
                locatorCacheTimeout: TimeSpan.FromSeconds(200),
                invocationTimeout: 1500);

            var locator = ILocatorPrx.Parse("locator", communicator).Clone(
                cacheConnection: false,
                preferNonSecure: true,
                endpointSelection: EndpointSelectionType.Random,
                locatorCacheTimeout: TimeSpan.FromSeconds(300),
                invocationTimeout: 1500,
                router: router);

            b1 = IObjectPrx.Parse("test", communicator).Clone(
                cacheConnection: true,
                preferNonSecure: false,
                endpointSelection: EndpointSelectionType.Ordered,
                locatorCacheTimeout: TimeSpan.FromSeconds(100),
                invocationTimeout: 1234,
                encoding: Encoding.V2_0,
                locator: locator);

            Dictionary<string, string> proxyProps = b1.ToProperty("Test");
            TestHelper.Assert(proxyProps.Count == 18);

            string defaultProtocolName = communicator.DefaultProtocol.GetName();
            TestHelper.Assert(proxyProps["Test"] == $"test -t -p {defaultProtocolName} -e 2.0");
            TestHelper.Assert(proxyProps["Test.ConnectionCached"] == "1");
            TestHelper.Assert(proxyProps["Test.PreferNonSecure"] == "0");
            TestHelper.Assert(proxyProps["Test.EndpointSelection"] == "Ordered");
            TestHelper.Assert(proxyProps["Test.LocatorCacheTimeout"] == "100s");
            TestHelper.Assert(proxyProps["Test.InvocationTimeout"] == "1234");

            TestHelper.Assert(proxyProps["Test.Locator"] == $"locator -t -p {defaultProtocolName} -e {Encoding.V2_0}");
            TestHelper.Assert(proxyProps["Test.Locator.ConnectionCached"] == "0");
            TestHelper.Assert(proxyProps["Test.Locator.PreferNonSecure"] == "1");
            TestHelper.Assert(proxyProps["Test.Locator.EndpointSelection"] == "Random");
            TestHelper.Assert(proxyProps["Test.Locator.LocatorCacheTimeout"] == "5m");
            TestHelper.Assert(proxyProps["Test.Locator.InvocationTimeout"] == "1500");

            TestHelper.Assert(proxyProps["Test.Locator.Router"] ==
                $"router -t -p {defaultProtocolName} -e {communicator.DefaultEncoding}");
            TestHelper.Assert(proxyProps["Test.Locator.Router.ConnectionCached"] == "1");
            TestHelper.Assert(proxyProps["Test.Locator.Router.PreferNonSecure"] == "1");
            TestHelper.Assert(proxyProps["Test.Locator.Router.EndpointSelection"] == "Random");
            TestHelper.Assert(proxyProps["Test.Locator.Router.LocatorCacheTimeout"] == "200s");
            TestHelper.Assert(proxyProps["Test.Locator.Router.InvocationTimeout"] == "1500");

            output.WriteLine("ok");

            output.Write("testing ice_getCommunicator... ");
            output.Flush();
            TestHelper.Assert(baseProxy.Communicator == communicator);
            output.WriteLine("ok");

            output.Write("testing proxy methods... ");

            TestHelper.Assert(baseProxy.Clone(facet: "facet", IObjectPrx.Factory).Facet.Equals("facet"));
            TestHelper.Assert(baseProxy.Clone(adapterId: "id").AdapterId.Equals("id"));
            TestHelper.Assert(!baseProxy.Clone(invocationMode: InvocationMode.Twoway).IsOneway);
            TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.Oneway).IsOneway);
            TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.Datagram).IsOneway);
            TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.BatchOneway).InvocationMode ==
                InvocationMode.BatchOneway);
            TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.Datagram).InvocationMode ==
                InvocationMode.Datagram);
            TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.BatchDatagram).InvocationMode ==
                InvocationMode.BatchDatagram);
            TestHelper.Assert(baseProxy.Clone(preferNonSecure: true).PreferNonSecure);
            TestHelper.Assert(!baseProxy.Clone(preferNonSecure: false).PreferNonSecure);

            try
            {
                baseProxy.Clone(invocationTimeout: 0);
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
            }

            try
            {
                baseProxy.Clone(invocationTimeout: -2);
                TestHelper.Assert(false);
            }
            catch (ArgumentException)
            {
            }

            try
            {
                baseProxy.Clone(locatorCacheTimeout: TimeSpan.Zero);
            }
            catch (ArgumentException)
            {
                TestHelper.Assert(false);
            }

            try
            {
                baseProxy.Clone(locatorCacheTimeout: Timeout.InfiniteTimeSpan);
            }
            catch (ArgumentException)
            {
                TestHelper.Assert(false);
            }

            try
            {
                baseProxy.Clone(locatorCacheTimeout: TimeSpan.FromSeconds(-2));
                TestHelper.Assert(false);
            }
            catch (ArgumentException)
            {
            }

            output.WriteLine("ok");

            output.Write("testing proxy comparison... ");
            output.Flush();

            TestHelper.Assert(Equals(IObjectPrx.Parse("foo", communicator), IObjectPrx.Parse("foo", communicator)));
            TestHelper.Assert(!IObjectPrx.Parse("foo", communicator).Equals(IObjectPrx.Parse("foo2", communicator)));

            var compObj = IObjectPrx.Parse("foo", communicator);

            TestHelper.Assert(compObj.Clone(facet: "facet", IObjectPrx.Factory).Equals(
                              compObj.Clone(facet: "facet", IObjectPrx.Factory)));
            TestHelper.Assert(!compObj.Clone(facet: "facet", IObjectPrx.Factory).Equals(
                              compObj.Clone(facet: "facet1", IObjectPrx.Factory)));

            TestHelper.Assert(compObj.Clone(invocationMode: InvocationMode.Oneway).Equals(
                              compObj.Clone(invocationMode: InvocationMode.Oneway)));
            TestHelper.Assert(!compObj.Clone(invocationMode: InvocationMode.Oneway).Equals(
                              compObj.Clone(invocationMode: InvocationMode.Twoway)));

            TestHelper.Assert(compObj.Clone(cacheConnection: true).Equals(compObj.Clone(cacheConnection: true)));
            TestHelper.Assert(!compObj.Clone(cacheConnection: false).Equals(compObj.Clone(cacheConnection: true)));

            TestHelper.Assert(compObj.Clone(endpointSelection: EndpointSelectionType.Random).Equals(
                              compObj.Clone(endpointSelection: EndpointSelectionType.Random)));
            TestHelper.Assert(!compObj.Clone(endpointSelection: EndpointSelectionType.Random).Equals(
                              compObj.Clone(endpointSelection: EndpointSelectionType.Ordered)));

            TestHelper.Assert(compObj.Clone(connectionId: "id2").Equals(compObj.Clone(connectionId: "id2")));
            TestHelper.Assert(!compObj.Clone(connectionId: "id1").Equals(compObj.Clone(connectionId: "id2")));
            TestHelper.Assert(compObj.Clone(connectionId: "id1").ConnectionId.Equals("id1"));
            TestHelper.Assert(compObj.Clone(connectionId: "id2").ConnectionId.Equals("id2"));

            var loc1 = ILocatorPrx.Parse("loc1:default -p 10000", communicator);
            var loc2 = ILocatorPrx.Parse("loc2:default -p 10000", communicator);
            TestHelper.Assert(compObj.Clone(clearLocator: true).Equals(compObj.Clone(clearLocator: true)));
            TestHelper.Assert(compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc1)));
            TestHelper.Assert(!compObj.Clone(locator: loc1).Equals(compObj.Clone(clearLocator: true)));
            TestHelper.Assert(!compObj.Clone(clearLocator: true).Equals(compObj.Clone(locator: loc2)));
            TestHelper.Assert(!compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc2)));

            var rtr1 = IRouterPrx.Parse("rtr1:default -p 10000", communicator);
            var rtr2 = IRouterPrx.Parse("rtr2:default -p 10000", communicator);
            TestHelper.Assert(compObj.Clone(clearRouter: true).Equals(compObj.Clone(clearRouter: true)));
            TestHelper.Assert(compObj.Clone(router: rtr1).Equals(compObj.Clone(router: rtr1)));
            TestHelper.Assert(!compObj.Clone(router: rtr1).Equals(compObj.Clone(clearRouter: true)));
            TestHelper.Assert(!compObj.Clone(clearRouter: true).Equals(compObj.Clone(router: rtr2)));
            TestHelper.Assert(!compObj.Clone(router: rtr1).Equals(compObj.Clone(router: rtr2)));

            Dictionary<string, string> ctx1 = new Dictionary<string, string>();
            ctx1["ctx1"] = "v1";
            Dictionary<string, string> ctx2 = new Dictionary<string, string>();
            ctx2["ctx2"] = "v2";
            TestHelper.Assert(compObj.Clone(context: new Dictionary<string, string>()).Equals(
                              compObj.Clone(context: new Dictionary<string, string>())));
            TestHelper.Assert(compObj.Clone(context: ctx1).Equals(compObj.Clone(context: ctx1)));
            TestHelper.Assert(!compObj.Clone(context: ctx1).Equals(
                              compObj.Clone(context: new Dictionary<string, string>())));
            TestHelper.Assert(!compObj.Clone(context: new Dictionary<string, string>()).Equals(
                              compObj.Clone(context: ctx2)));
            TestHelper.Assert(!compObj.Clone(context: ctx1).Equals(compObj.Clone(context: ctx2)));

            TestHelper.Assert(compObj.Clone(preferNonSecure: true).Equals(compObj.Clone(preferNonSecure: true)));
            TestHelper.Assert(!compObj.Clone(preferNonSecure: true).Equals(compObj.Clone(preferNonSecure: false)));

            var compObj1 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10000", communicator);
            var compObj2 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10001", communicator);
            TestHelper.Assert(!compObj1.Equals(compObj2));

            compObj1 = IObjectPrx.Parse("foo@MyAdapter1", communicator);
            compObj2 = IObjectPrx.Parse("foo@MyAdapter2", communicator);
            TestHelper.Assert(!compObj1.Equals(compObj2));

            TestHelper.Assert(compObj1.Clone(locatorCacheTimeout: TimeSpan.FromSeconds(20))
                .Equals(compObj1.Clone(locatorCacheTimeout: TimeSpan.FromSeconds(20))));
            TestHelper.Assert(!compObj1.Clone(locatorCacheTimeout: TimeSpan.FromSeconds(10))
                .Equals(compObj1.Clone(locatorCacheTimeout: TimeSpan.FromSeconds(20))));

            TestHelper.Assert(compObj1.Clone(invocationTimeout: 20).Equals(compObj1.Clone(invocationTimeout: 20)));
            TestHelper.Assert(!compObj1.Clone(invocationTimeout: 10).Equals(compObj1.Clone(invocationTimeout: 20)));

            compObj1 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 1000", communicator);
            compObj2 = IObjectPrx.Parse("foo@MyAdapter1", communicator);
            TestHelper.Assert(!compObj1.Equals(compObj2));

            var endpts1 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10000", communicator).Endpoints;
            var endpts2 = IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10001", communicator).Endpoints;
            TestHelper.Assert(!endpts1[0].Equals(endpts2[0]));
            TestHelper.Assert(endpts1[0].Equals(IObjectPrx.Parse("foo:tcp -h 127.0.0.1 -p 10000", communicator).Endpoints[0]));

            if (baseProxy.GetConnection() is Connection baseConnection)
            {
                Connection baseConnection2 = baseProxy.Clone(connectionId: "base2").GetConnection()!;
                compObj1 = compObj1.Clone(fixedConnection: baseConnection);
                compObj2 = compObj2.Clone(fixedConnection: baseConnection2);
                TestHelper.Assert(!compObj1.Equals(compObj2));
            }
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            var cl = IMyClassPrx.CheckedCast(baseProxy);
            TestHelper.Assert(cl != null);
            var derived = IMyDerivedClassPrx.CheckedCast(cl);
            TestHelper.Assert(derived != null);
            TestHelper.Assert(cl.Equals(baseProxy));
            TestHelper.Assert(derived.Equals(baseProxy));
            TestHelper.Assert(cl.Equals(derived));
            try
            {
                IMyDerivedClassPrx.CheckedCast(cl.Clone(facet: "facet", IObjectPrx.Factory));
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing checked cast with context... ");
            output.Flush();

            Dictionary<string, string> c = cl.GetContext();
            TestHelper.Assert(c == null || c.Count == 0);

            c = new Dictionary<string, string>();
            c["one"] = "hello";
            c["two"] = "world";
            cl = IMyClassPrx.CheckedCast(baseProxy, c);
            Dictionary<string, string> c2 = cl!.GetContext();
            TestHelper.Assert(c.DictionaryEqual(c2));
            output.WriteLine("ok");

            output.Write("testing ice_fixed... ");
            output.Flush();
            {
                if (cl.GetConnection() is Connection connection2)
                {
                    TestHelper.Assert(!cl.IsFixed);
                    IMyClassPrx prx = cl.Clone(fixedConnection: connection2);
                    TestHelper.Assert(prx.IsFixed);
                    prx.IcePing();
                    TestHelper.Assert(cl.Clone("facet", IObjectPrx.Factory, fixedConnection: connection2).Facet.Equals("facet"));
                    TestHelper.Assert(cl.Clone(invocationMode: InvocationMode.Oneway, fixedConnection: connection2).IsOneway);
                    Dictionary<string, string> ctx = new Dictionary<string, string>();
                    ctx["one"] = "hello";
                    ctx["two"] = "world";
                    TestHelper.Assert(cl.Clone(fixedConnection: connection2).Context.Count == 0);
                    TestHelper.Assert(cl.Clone(context: ctx, fixedConnection: connection2).Context.Count == 2);
                    TestHelper.Assert(cl.Clone(fixedConnection: connection2).InvocationTimeout == -1);
                    TestHelper.Assert(cl.Clone(invocationTimeout: 10, fixedConnection: connection2).InvocationTimeout == 10);
                    TestHelper.Assert(cl.Clone(fixedConnection: connection2).GetConnection() == connection2);
                    TestHelper.Assert(cl.Clone(fixedConnection: connection2).Clone(fixedConnection: connection2).GetConnection() == connection2);
                    Connection? fixedConnection = cl.Clone(connectionId: "ice_fixed").GetConnection();
                    TestHelper.Assert(cl.Clone(fixedConnection: connection2).Clone(fixedConnection: fixedConnection).GetConnection() == fixedConnection);
                    try
                    {
                        cl.Clone(invocationMode: InvocationMode.Datagram, fixedConnection: connection2);
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                }
            }
            output.WriteLine("ok");

            output.Write("testing encoding versioning... ");
            string ref13 = "test -e 1.3:" + helper.GetTestEndpoint(0);
            var cl13 = IMyClassPrx.Parse(ref13, communicator);
            try
            {
                cl13.IcePing();
                TestHelper.Assert(false);
            }
            catch (NotSupportedException)
            {
                // expected
            }
            output.WriteLine("ok");

            output.Write("testing protocol versioning... ");
            output.Flush();
            string ref3 = "test -p 3:" + helper.GetTestEndpoint(0);
            var cl3 = IMyClassPrx.Parse(ref3, communicator);
            try
            {
                cl3.IcePing();
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing -t and -v
                IObjectPrx.Parse("id:opaque", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Repeated -t
                IObjectPrx.Parse("id:opaque -t 1 -t 1 -v abcd", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Repeated -v
                IObjectPrx.Parse("id:opaque -t 1 -v abcd -v abcd", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing -t
                IObjectPrx.Parse("id:opaque -v abcd", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing -v
                IObjectPrx.Parse("id:opaque -t 1", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing arg for -t
                IObjectPrx.Parse("id:opaque -t -v abcd", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Missing arg for -v
                IObjectPrx.Parse("id:opaque -t 1 -v", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Not a number for -t
                IObjectPrx.Parse("id:opaque -t x -v abcd", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // < 0 for -t
                IObjectPrx.Parse("id:opaque -t -1 -v abcd", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Invalid char for -v
                IObjectPrx.Parse("id:opaque -t 99 -v x?c", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Invalid lenght for base64 input
                IObjectPrx.Parse("id:opaque -t 99 -v xc", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            // Legal TCP endpoint expressed as opaque endpoint
            // Opaque endpoint encoded with 1.1 encoding.
            var p1 = IObjectPrx.Parse("test -e 1.1 -p ice1:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==",
                                      communicator);
            TestHelper.Assert(p1.ToString()!.Equals("test -t -p ice1 -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000"));

            if (!(communicator.GetPropertyAsBool("Ice.IPv6") ?? false))
            {
                // Two legal TCP endpoints expressed as opaque endpoints
                p1 = IObjectPrx.Parse("test -e 1.1 -p ice1:" + "" +
                    "opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:" +
                    "opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==", communicator);
                TestHelper.Assert(p1.ToString() ==
                    "test -t -p ice1 -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

                // Test that an SSL endpoint and a nonsense endpoint get written back out as an opaque endpoint.
                p1 = IObjectPrx.Parse("test -e 1.1 -p ice1:opaque -e 1.1 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -e 1.1 -t 99 -v abch",
                                      communicator);
                TestHelper.Assert(p1.ToString() ==
                    "test -t -p ice1 -e 1.1:ssl -h 127.0.0.1 -p 10001 -t infinite:opaque -t 99 -e 1.1 -v abch");
            }

            output.WriteLine("ok");

            output.Write("testing communicator shutdown/destroy... ");
            output.Flush();
            {
                Communicator com = new Communicator();
                com.ShutdownAsync();
                com.WaitForShutdownAsync();
                com.Dispose();
                com.ShutdownAsync();
                com.WaitForShutdownAsync();
                com.Dispose();
            }
            output.WriteLine("ok");

            return cl;
        }
    }
}
