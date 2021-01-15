// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Proxy
{
    public static class AllTests
    {
        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            bool ice1 = helper.Protocol == Protocol.Ice1;
            System.IO.TextWriter output = helper.Output;
            output.Write("testing proxy parsing... ");
            output.Flush();

            // ice1 proxies
            string[] ice1ProxyArray =
            {
                "ice -t:tcp -h localhost -p 10000",
                "ice+tcp:ssl -h localhost -p 10000"
            };

            // ice2 proxies
            string[] ice2ProxyArray =
            {
                "ice+tcp://host.zeroc.com/identity#facet",
                "ice+tcp://host.zeroc.com:1000/category/name",
                "ice+tcp://host.zeroc.com:1000/loc0/loc1/category/name",
                "ice+tcp://host.zeroc.com/category/name%20with%20space",
                "ice+ws://host.zeroc.com//identity",
                "ice+ws://host.zeroc.com//identity?invocation-timeout=100ms",
                "ice+ws://host.zeroc.com//identity?invocation-timeout=1s",
                "ice+ws://host.zeroc.com//identity?alt-endpoint=host2.zeroc.com",
                "ice+ws://host.zeroc.com//identity?alt-endpoint=host2.zeroc.com:10000",
                "ice+tcp://[::1]:10000/identity?alt-endpoint=host1:10000,host2,host3,host4",
                "ice+tcp://[::1]:10000/identity?alt-endpoint=host1:10000&alt-endpoint=host2,host3&alt-endpoint=[::2]",
                "ice:location//identity#facet",
                "ice:location//identity?relative=true#facet",
                "ice+tcp://host.zeroc.com//identity",
                "ice+tcp://host.zeroc.com:/identity", // another syntax for empty port
                "ice+universal://com.zeroc.ice/identity?transport=iaps&option=a,b%2Cb,c&option=d",
                "ice+universal://host.zeroc.com/identity?transport=100",
                "ice+universal://[::ab:cd:ef:00]/identity?transport=bt", // leading :: to make the address IPv6-like
                "ice+ws://host.zeroc.com/identity?resource=/foo%2Fbar?/xyz",
                "ice+universal://host.zeroc.com:10000/identity?transport=tcp",
                "ice+universal://host.zeroc.com/identity?transport=ws&option=/foo%2520/bar",
                "ice:tcp -p 10000", // a valid URI
            };

            // ice3 proxies
            string[] ice3ProxyArray =
            {
                "ice+universal://host.zeroc.com/identity?transport=ws&option=/foo%2520/bar&protocol=3"
            };

            foreach (string str in ice1ProxyArray)
            {
                var prx = IObjectPrx.Parse(str, communicator);
                TestHelper.Assert(prx.Protocol == Protocol.Ice1);
                // output.WriteLine($"{str} = {prx}");
                var prx2 = IObjectPrx.Parse(prx.ToString()!, communicator);
                TestHelper.Assert(prx.Equals(prx2)); // round-trip works

                try
                {
                    // Cannot use Ice2 endpoint with Ice1 proxy
                    prx2.Clone(endpoints: IObjectPrx.Parse(ice2ProxyArray[0], communicator).Endpoints);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }
            }

            foreach (string str in ice2ProxyArray)
            {
                var prx = IObjectPrx.Parse(str, communicator);
                TestHelper.Assert(prx.Protocol == Protocol.Ice2);
                // output.WriteLine($"{str} = {prx}");
                var prx2 = IObjectPrx.Parse(prx.ToString()!, communicator);
                TestHelper.Assert(prx.Equals(prx2)); // round-trip works

                try
                {
                    // Cannot use Ice1 endpoint with Ice2 proxy
                    prx2.Clone(endpoints: IObjectPrx.Parse(ice1ProxyArray[0], communicator).Endpoints);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }
            }

            foreach (string str in ice3ProxyArray)
            {
                var prx = IObjectPrx.Parse(str, communicator);
                TestHelper.Assert(prx.Protocol == (Protocol)3);
                // output.WriteLine($"{str} = {prx}");
                var prx2 = IObjectPrx.Parse(prx.ToString()!, communicator);
                TestHelper.Assert(prx.Equals(prx2)); // round-trip works
            }

            string[] badProxyArray =
            {
                "ice+tcp://host.zeroc.com:foo",     // missing host
                "ice+tcp:identity?protocol=invalid", // invalid protocol
                "ice+universal://host.zeroc.com", // missing transport
                "ice+universal://host.zeroc.com?transport=100&protocol=ice1", // invalid protocol
                "ice://host:1000/identity", // host not allowed
                "ice+universal:/identity", // missing host
                "ice+tcp://host.zeroc.com/identity?protocol=3", // unknown protocol (must use universal)
                "ice+ws://host.zeroc.com//identity?protocol=ice1", // invalid protocol
                "ice+tcp://host.zeroc.com/identity?alt-endpoint=host2?protocol=ice2", // protocol option in alt-endpoint
                "ice+tcp://host.zeroc.com/identity?foo=bar", // unknown option
                "ice+tcp://host.zeroc.com/identity?invocation-timeout=0s", // 0 is not a valid invocation timeout
                "ice:foo?relative=bad", // bad value for relative
                "ice:foo?fixed=true", // cannot create fixed proxy from URI

                "",
                "\"\"",
                "\"\" test", // invalid trailing characters
                "test:", // missing endpoint
                "id@adapter test",
                "id -f \"facet x",
                "id -f \'facet x",
                "test -f facet@test @test",
                "test -p 2.0",
                "test:tcp@location",
                "test: :tcp",
                "id:opaque -t 99 -v abcd -x abc", // invalid x option
                "id:opaque", // missing -t and -v
                "id:opaque -t 1 -t 1 -v abcd", // repeated -t
                "id:opaque -t 1 -v abcd -v abcd",
                "id:opaque -v abcd",
                "id:opaque -t 1",
                "id:opaque -t -v abcd",
                "id:opaque -t 1 -v",
                "id:opaque -t x -v abcd",
                "id:opaque -t -1 -v abcd", // -t must be >= 0
                "id:opaque -t 99 -v x?c", // invalid char in v
                "id:opaque -t 99 -v xc", // invalid length for base64 input
                "ice+tcp://0.0.0.0/identity#facet", // Invalid Any IPv4 address in proxy endpoint
                "ice+tcp://[::0]/identity#facet", // Invalid Any IPv6 address in proxy endpoint
                "identity:tcp -h 0.0.0.0", // Invalid Any IPv4 address in proxy endpoint
                "identity:tcp -h [::0]", // Invalid Any IPv6 address in proxy endpoint
            };

            foreach (string str in badProxyArray)
            {
                try
                {
                    _ = IObjectPrx.Parse(str, communicator);
                    TestHelper.Assert(false);
                }
                catch (FormatException)
                {
                    // expected
                }
            }

            string rf = helper.GetTestProxy("test");
            var baseProxy = IObjectPrx.Parse(rf, communicator);
            TestHelper.Assert(baseProxy != null);

            var b1 = IObjectPrx.Parse("ice:test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                              b1.Location.Count == 0 && b1.Facet.Length == 0);

            b1 = IObjectPrx.Parse("ice:test ", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);

            b1 = IObjectPrx.Parse(" ice:test ", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                              b1.Facet.Length == 0);

            b1 = IObjectPrx.Parse(" ice:test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);

            b1 = IObjectPrx.Parse("test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                              b1.Location.Count == 0 && b1.Facet.Length == 0);

            b1 = IObjectPrx.Parse("test ", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);

            b1 = IObjectPrx.Parse(" test ", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                              b1.Facet.Length == 0);

            b1 = IObjectPrx.Parse(" test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);

            // The following tests are only relevant to the ice1 format
            b1 = IObjectPrx.Parse("'test -f facet'", communicator);
            TestHelper.Assert(b1.Identity.Name == "test -f facet" && b1.Identity.Category.Length == 0 &&
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
            TestHelper.Assert(b1.Identity.Name == "test -f facet" && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("\"test -f facet@test\"", communicator);
            TestHelper.Assert(b1.Identity.Name == "test -f facet@test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet.Length == 0);
            b1 = IObjectPrx.Parse("\"test -f facet@test @test\"", communicator);
            TestHelper.Assert(b1.Identity.Name == "test -f facet@test @test" && b1.Identity.Category.Length == 0 &&
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

            TestHelper.Assert(b1.Identity.Name == "test test" && b1.Identity.Category.Length == 0);
            try
            {
                b1 = IObjectPrx.Parse("test\\777", communicator);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }
            b1 = IObjectPrx.Parse("test\\40test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test test");

            // Test some octal and hex corner cases.
            b1 = IObjectPrx.Parse("test\\4test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test\u0004test");
            b1 = IObjectPrx.Parse("test\\04test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test\u0004test");
            b1 = IObjectPrx.Parse("test\\004test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test\u0004test");
            b1 = IObjectPrx.Parse("test\\1114test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test\u00494test");

            b1 = IObjectPrx.Parse("test\\b\\f\\n\\r\\t\\'\\\"\\\\test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test\b\f\n\r\t\'\"\\test" && b1.Identity.Category.Length == 0);

            // End of ice1 format-only tests

            b1 = IObjectPrx.Parse("ice:category/test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location.Count == 0);

            b1 = IObjectPrx.Parse("ice:loc0/loc1/category/test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location.Count == 2 && b1.Location[0] == "loc0" && b1.Location[1] == "loc1");

            b1 = IObjectPrx.Parse("ice+tcp://host:10000/test?source-address=::1", communicator);
            TestHelper.Assert(b1.Equals(IObjectPrx.Parse(b1.ToString()!, communicator)));

            b1 = IObjectPrx.Parse("ice+tcp://host:10000/loc0/loc1//test?source-address=::1", communicator);
            TestHelper.Assert(b1.Equals(IObjectPrx.Parse(b1.ToString()!, communicator)));
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                b1.Location.Count == 2 && b1.Location[0] == "loc0" && b1.Location[1] == "loc1");

            b1 = IObjectPrx.Parse("ice:adapter//test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Location[0] == "adapter");

            b1 = IObjectPrx.Parse("ice:adapter/category/test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location[0] == "adapter");

            b1 = IObjectPrx.Parse("ice:adapter:tcp/category/test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location[0] == "adapter:tcp");

            // preferred syntax with escape:
            TestHelper.Assert(b1.Equals(IObjectPrx.Parse("ice:adapter%3Atcp/category/test", communicator)));

            b1 = IObjectPrx.Parse("category/test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location.Count == 0);

            b1 = IObjectPrx.Parse("test:tcp -h host -p 10000 --sourceAddress \"::1\"", communicator);
            TestHelper.Assert(b1.Equals(IObjectPrx.Parse(b1.ToString()!, communicator)));

            b1 = IObjectPrx.Parse(
                "test:udp -h host -p 10000 --sourceAddress \"::1\" --interface \"0:0:0:0:0:0:0:1%lo\"",
                communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Location.Count == 0);

            b1 = IObjectPrx.Parse("test:tcp -h localhost -p 10000 -t infinite", communicator);
            TestHelper.Assert(b1.ToString() == "test -t -e 1.1:tcp -h localhost -p 10000 -t -1");

            b1 = IObjectPrx.Parse("test@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Location[0] == "adapter");

            b1 = IObjectPrx.Parse("category/test@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location[0] == "adapter");

            b1 = IObjectPrx.Parse("category/test@adapter:tcp", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location[0] == "adapter:tcp");

            // The following tests are only for the ice1 format:
            b1 = IObjectPrx.Parse("'category 1/test'@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category 1" &&
                    b1.Location[0] == "adapter");
            b1 = IObjectPrx.Parse("'category/test 1'@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name == "test 1" && b1.Identity.Category == "category" &&
                    b1.Location[0] == "adapter");
            b1 = IObjectPrx.Parse("'category/test'@'adapter 1'", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category" &&
                    b1.Location[0] == "adapter 1");
            b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@adapter", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category /test@foo" &&
                    b1.Location[0] == "adapter");
            b1 = IObjectPrx.Parse("\"category \\/test@foo/test\"@\"adapter:tcp\"", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category == "category /test@foo" &&
                    b1.Location[0] == "adapter:tcp");
            // End of ice1 format-only tests.

            b1 = IObjectPrx.Parse("ice:id#facet", communicator);
            TestHelper.Assert(b1.Identity.Name == "id" && b1.Identity.Category.Length == 0 && b1.Facet == "facet");

            b1 = IObjectPrx.Parse("ice:id#facet%20x", communicator);
            TestHelper.Assert(b1.Identity.Name == "id" && b1.Identity.Category.Length == 0 && b1.Facet == "facet x");

            b1 = IObjectPrx.Parse("id -f facet", communicator);
            TestHelper.Assert(b1.Identity.Name == "id" && b1.Identity.Category.Length == 0 && b1.Facet == "facet");

            b1 = IObjectPrx.Parse("id -f 'facet x'", communicator);
            TestHelper.Assert(b1.Identity.Name == "id" && b1.Identity.Category.Length == 0 && b1.Facet == "facet x");

            // The following tests are only for the ice1 format:
            b1 = IObjectPrx.Parse("id -f \"facet x\"", communicator);
            TestHelper.Assert(b1.Identity.Name == "id" && b1.Identity.Category.Length == 0 && b1.Facet == "facet x");
            b1 = IObjectPrx.Parse("test -f facet:tcp -h localhost", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet == "facet" && b1.Location.Count == 0);
            b1 = IObjectPrx.Parse("test -f \"facet:tcp\"", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet == "facet:tcp" && b1.Location.Count == 0);
            b1 = IObjectPrx.Parse("test -f facet@test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet == "facet" && b1.Location[0] == "test");
            b1 = IObjectPrx.Parse("test -f 'facet@test'", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet == "facet@test" && b1.Location.Count == 0);
            b1 = IObjectPrx.Parse("test -f 'facet@test'@test", communicator);
            TestHelper.Assert(b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                    b1.Facet == "facet@test" && b1.Location[0] == "test");
            // End of ice1 format-only tests.

            b1 = IObjectPrx.Parse("ice:test", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.Twoway);
            TestHelper.Assert(!b1.IsOneway);

            b1 = IObjectPrx.Parse("test", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.Twoway);
            TestHelper.Assert(!b1.IsOneway);

            b1 = IObjectPrx.Parse("test -t", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.Twoway);

            b1 = IObjectPrx.Parse("test -o", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.Oneway);

            b1 = IObjectPrx.Parse("test -O", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.BatchOneway);

            b1 = IObjectPrx.Parse("test -d", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.Datagram);

            b1 = IObjectPrx.Parse("test -D", communicator);
            TestHelper.Assert(b1.InvocationMode == InvocationMode.BatchDatagram);

            b1 = IObjectPrx.Parse("ice:test", communicator);
            TestHelper.Assert(b1.Protocol == Protocol.Ice2 && b1.Encoding == Encoding.V20);
            b1 = IObjectPrx.Parse("test", communicator);
            TestHelper.Assert(b1.Protocol == Protocol.Ice1 && b1.Encoding == Encoding.V11);

            b1 = IObjectPrx.Parse("ice:test?encoding=6.5", communicator);
            TestHelper.Assert(b1.Encoding.Major == 6 && b1.Encoding.Minor == 5);
            b1 = IObjectPrx.Parse("test -e 6.5", communicator);
            TestHelper.Assert(b1.Encoding.Major == 6 && b1.Encoding.Minor == 5);

            b1 = IObjectPrx.Parse("ice:test?encoding=2.1&protocol=6", communicator);
            TestHelper.Assert(b1.Protocol == (Protocol)6 && b1.Encoding.Major == 2 && b1.Encoding.Minor == 1);

            // Test invalid endpoint syntax
            // TODO: why are we testing this here?
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

            // Test for bug ICE-5543: escaped escapes in Identity.Parse
            var id = new Identity("test", ",X2QNUAzSBcJ_e$AV;E\\");
            var id2 = Identity.Parse(id.ToString(communicator.ToStringMode), uriFormat: false);
            var id3 = Identity.Parse(id.ToString()); // new URI style
            TestHelper.Assert(id == id2);
            TestHelper.Assert(id == id3);

            id = new Identity("test", ",X2QNUAz\\SB\\/cJ_e$AV;E\\\\");
            id2 = Identity.Parse(id.ToString(communicator.ToStringMode), uriFormat: false);
            id3 = Identity.Parse(id.ToString());
            TestHelper.Assert(id == id2);
            TestHelper.Assert(id == id3);

            id = new Identity("/test", "cat/");
            string idStr = id.ToString(communicator.ToStringMode);
            TestHelper.Assert(idStr == "cat\\//\\/test");
            id2 = Identity.Parse(idStr, uriFormat: false);
            id3 = Identity.Parse(id.ToString());
            TestHelper.Assert(id == id2);
            TestHelper.Assert(id == id3);

            // Input string in ice1 format with various pitfalls
            idStr = "\\342\\x82\\254\\60\\x9\\60\\";
            id = Identity.Parse(idStr, uriFormat: false);
            TestHelper.Assert(id.Name == "€0\t0\\" && id.Category.Length == 0);

            try
            {
                // Illegal character < 32
                _ = Identity.Parse("xx\01FooBar", uriFormat: false);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            try
            {
                // Illegal surrogate
                _ = Identity.Parse("xx\\ud911", uriFormat: false);
                TestHelper.Assert(false);
            }
            catch (FormatException)
            {
            }

            // Testing bytes 127(\x7F, \177) and €
            // € is encoded as 0x20AC (UTF-16) and 0xE2 0x82 0xAC (UTF-8)
            id = new Identity("test", "\x7f€");

            idStr = id.ToString();
            TestHelper.Assert(idStr == "%7F%E2%82%AC/test");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id == id2);

            idStr = id.ToString(ToStringMode.Unicode);
            TestHelper.Assert(idStr == "\\u007f€/test");
            id2 = Identity.Parse(idStr, uriFormat: false);
            TestHelper.Assert(id == id2);

            idStr = id.ToString(ToStringMode.ASCII);
            TestHelper.Assert(idStr == "\\u007f\\u20ac/test");
            id2 = Identity.Parse(idStr, uriFormat: false);
            TestHelper.Assert(id == id2);

            idStr = id.ToString(ToStringMode.Compat);
            TestHelper.Assert(idStr == "\\177\\342\\202\\254/test");
            id2 = Identity.Parse(idStr, uriFormat: false);
            TestHelper.Assert(id == id2);

            // More unicode character
            id = new Identity("banana \x0E-\ud83c\udf4c\u20ac\u00a2\u0024", "greek \ud800\udd6a");
            idStr = id.ToString();
            TestHelper.Assert(idStr == "greek%20%F0%90%85%AA/banana%20%0E-%F0%9F%8D%8C%E2%82%AC%C2%A2%24");
            id2 = Identity.Parse(idStr);
            TestHelper.Assert(id == id2);

            idStr = id.ToString(ToStringMode.Unicode);
            TestHelper.Assert(idStr == "greek \ud800\udd6a/banana \\u000e-\ud83c\udf4c\u20ac\u00a2$");
            id2 = Identity.Parse(idStr, uriFormat: false);
            TestHelper.Assert(id == id2);

            idStr = id.ToString(ToStringMode.ASCII);
            TestHelper.Assert(idStr == "greek \\U0001016a/banana \\u000e-\\U0001f34c\\u20ac\\u00a2$");
            id2 = Identity.Parse(idStr, uriFormat: false);
            TestHelper.Assert(id == id2);

            idStr = id.ToString(ToStringMode.Compat);
            id2 = Identity.Parse(idStr, uriFormat: false);
            TestHelper.Assert(idStr == "greek \\360\\220\\205\\252/banana \\016-\\360\\237\\215\\214\\342\\202\\254\\302\\242$");
            TestHelper.Assert(id == id2);

            output.WriteLine("ok");

            output.Write("testing fixed proxies... ");
            output.Flush();
            b1 = IObjectPrx.Parse(rf, communicator);

            Connection connection = await b1.GetConnectionAsync();
            IObjectPrx b2 = connection.CreateProxy(Identity.Parse("fixed"), facet: "", IObjectPrx.Factory);
            if (connection.Protocol == Protocol.Ice1)
            {
                TestHelper.Assert(b2.ToString() == "fixed -t -e 1.1");
            }
            else
            {
                TestHelper.Assert(b2.ToString() == "ice:fixed?fixed=true");
            }
            output.WriteLine("ok");

            output.Write("testing proxy options and properties... ");
            output.Flush();

            string propertyPrefix = "Foo.Proxy";
            string proxyString = helper.GetTestProxy("test", 0);

            // For ice1, we parse the property tree while for ice2 we usually just parse the string-value since it's
            // equivalent.

            communicator.SetProperty(propertyPrefix, proxyString);
            b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
            TestHelper.Assert(
                b1.Identity.Name == "test" && b1.Identity.Category.Length == 0 &&
                b1.Location.Count == 0 && b1.Facet.Length == 0);

            TestHelper.Assert(b1.CacheConnection);
            if (ice1)
            {
                string property = propertyPrefix + ".CacheConnection";
                communicator.SetProperty(property, "0");
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                communicator.RemoveProperty(property);
            }
            else
            {
                b1 = IObjectPrx.Parse($"{proxyString}?cache-connection=false", communicator);
            }
            TestHelper.Assert(!b1.CacheConnection);

            if (ice1)
            {
                string property = propertyPrefix + ".Context.c1";
                TestHelper.Assert(!b1.Context.ContainsKey("c1"));
                communicator.SetProperty(property, "TEST1");
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                TestHelper.Assert(b1.Context["c1"] == "TEST1");

                property = propertyPrefix + ".Context.c2";
                TestHelper.Assert(!b1.Context.ContainsKey("c2"));
                communicator.SetProperty(property, "TEST2");
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                TestHelper.Assert(b1.Context["c2"] == "TEST2");

                communicator.SetProperty(propertyPrefix + ".Context.c1", "");
                communicator.SetProperty(propertyPrefix + ".Context.c2", "");
            }
            else
            {
                b1 = IObjectPrx.Parse(
                    $"{proxyString}?context=c1=TEST1,c2=TEST&context=c2=TEST2,d%204=TEST%204,c3=TEST3",
                    communicator);

                TestHelper.Assert(b1.Context.Count == 4);
                TestHelper.Assert(b1.Context["c1"] == "TEST1");
                TestHelper.Assert(b1.Context["c2"] == "TEST2");
                TestHelper.Assert(b1.Context["c3"] == "TEST3");
                TestHelper.Assert(b1.Context["d 4"] == "TEST 4");

                // This works because Context is a sorted dictionary
                TestHelper.Assert(b1.ToString() ==
                    $"{proxyString}?context=c1=TEST1,c2=TEST2,c3=TEST3,d%204=TEST%204");
            }

            TestHelper.Assert(b1.InvocationTimeout == TimeSpan.FromSeconds(60));
            if (ice1)
            {
                string property = propertyPrefix + ".InvocationTimeout";
                communicator.SetProperty(property, "1s");
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                communicator.SetProperty(property, "");
            }
            else
            {
                b1 = IObjectPrx.Parse($"{proxyString}?invocation-timeout=1s", communicator);
            }
            TestHelper.Assert(b1.InvocationTimeout == TimeSpan.FromSeconds(1));

            if (ice1)
            {
                string property = propertyPrefix + ".Locator";
                TestHelper.Assert(b1.Locator == null);
                communicator.SetProperty(property, "ice+tcp://host:10000/locator");
                try
                {
                    b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                    TestHelper.Assert(false);
                }
                catch (InvalidConfigurationException)
                {
                    // expected, can't have a locator on a direct proxy
                }
                communicator.SetProperty(property, "");

                // Now retest with an indirect proxy.
                communicator.SetProperty(propertyPrefix, "test");
                property = propertyPrefix + ".Locator";
                communicator.SetProperty(property, "ice+tcp://host:10000/locator"); // locator can be ice2 proxy
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                TestHelper.Assert(b1.Locator != null && b1.Locator.Identity.Name == "locator");
            }
            // else, no proxy option equivalent with ice2

            TestHelper.Assert(b1.LocatorCacheTimeout == Timeout.InfiniteTimeSpan);
            if (ice1)
            {
                string property = propertyPrefix + ".LocatorCacheTimeout";
                communicator.SetProperty(property, "1s");
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                communicator.SetProperty(property, "");
                communicator.SetProperty(propertyPrefix + ".Locator", "");
            }
            else
            {
                communicator.DefaultLocator = ILocatorPrx.Parse("ice+tcp://host:10000/locator", communicator);
                b1 = IObjectPrx.Parse("ice:test?locator-cache-timeout=1s", communicator);
                communicator.DefaultLocator = null;
            }
            TestHelper.Assert(b1.LocatorCacheTimeout == TimeSpan.FromSeconds(1));

            TestHelper.Assert(b1.PreferNonSecure == communicator.DefaultPreferNonSecure);
            if (ice1)
            {
                string property = propertyPrefix + ".PreferNonSecure";
                communicator.SetProperty(property, "SameHost");
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                communicator.RemoveProperty(property);
            }
            else
            {
                b1 = IObjectPrx.Parse($"{proxyString}?prefer-non-secure=SameHost",
                                      communicator);
            }
            TestHelper.Assert(b1.PreferNonSecure != communicator.DefaultPreferNonSecure);

            TestHelper.Assert(!b1.IsRelative);
            if (ice1)
            {
                communicator.SetProperty(propertyPrefix, "test");
                string property = propertyPrefix + ".Relative";
                communicator.SetProperty(property, "true");
                b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                communicator.RemoveProperty(property);
                communicator.SetProperty(propertyPrefix, proxyString);
            }
            else
            {
                try
                {
                    b1 = IObjectPrx.Parse($"{proxyString}?relative=true", communicator);
                }
                catch (FormatException)
                {
                    // expected
                }
                b1 = IObjectPrx.Parse("ice:test?relative=true", communicator);
            }
            TestHelper.Assert(b1.IsRelative);

            if (ice1)
            {
                string property = propertyPrefix + ".Router";
                TestHelper.Assert(b1.Router == null);
                communicator.SetProperty(property, "ice+tcp://host:10000/router");
                try
                {
                    b1 = communicator.GetPropertyAsProxy(propertyPrefix, IObjectPrx.Factory)!;
                    TestHelper.Assert(false);
                }
                catch (InvalidConfigurationException)
                {
                    // expected: a router must be an ice1 proxy
                }
                communicator.RemoveProperty(property);
            }
            else
            {
                string complicated = $"{proxyString}?invocation-timeout=10s&context=c%201=some%20value" +
                    "&alt-endpoint=ice+ws://localhost?resource=/x/y$source-address=[::1]&context=c5=v5";
                b1 = IObjectPrx.Parse(complicated, communicator);

                TestHelper.Assert(b1.Endpoints.Count == 2);
                TestHelper.Assert(b1.Endpoints[1].Transport == Transport.WS);
                TestHelper.Assert(b1.Endpoints[1]["resource"] == "/x/y");
                TestHelper.Assert(b1.Endpoints[1]["source-address"] == "::1");
                TestHelper.Assert(b1.Context.Count == 2);
                TestHelper.Assert(b1.Context["c 1"] == "some value");
                TestHelper.Assert(b1.Context["c5"] == "v5");
            }

            output.WriteLine("ok");

            output.Write("testing IObjectPrx.ToProperty... ");
            output.Flush();

            b1 = IObjectPrx.Parse(
                ice1 ? "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 1000" : "ice+tcp://127.0.0.1/test",
                communicator).Clone(cacheConnection: true,
                                    preferExistingConnection: true,
                                    preferNonSecure: NonSecure.Never,
                                    invocationTimeout: TimeSpan.FromSeconds(10));

            Dictionary<string, string> proxyProps = b1.ToProperty("Test");
            // InvocationTimeout is a property with ice1 and an URI option with ice2 so the extra property with ice1.
            // Also no router properties with ice2.
            TestHelper.Assert(proxyProps.Count == (ice1 ? 4 : 1));

            TestHelper.Assert(proxyProps["Test"] ==
                (ice1 ? "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 1000" :
                        "ice+tcp://127.0.0.1/test?invocation-timeout=10s&prefer-existing-connection=true&prefer-non-secure=never"));

            if (ice1)
            {
                TestHelper.Assert(proxyProps["Test.InvocationTimeout"] == "10s");
                TestHelper.Assert(proxyProps["Test.PreferNonSecure"] == "Never");
            }

            IRouterPrx? router = null;

            if (ice1)
            {
                router = IRouterPrx.Parse("router", communicator).Clone(
                    cacheConnection: true,
                    preferExistingConnection: true,
                    preferNonSecure: NonSecure.Always);
            }

            ILocatorPrx? locator = ILocatorPrx.Parse(ice1 ? "locator" : "ice:locator", communicator).Clone(
                cacheConnection: false,
                preferExistingConnection: false,
                preferNonSecure: NonSecure.Always,
                router: router);

            b1 = b1.Clone(endpoints: ImmutableArray<Endpoint>.Empty,
                          locatorCacheTimeout: TimeSpan.FromSeconds(100),
                          locator: locator);

            proxyProps = b1.ToProperty("Test");

            TestHelper.Assert(proxyProps.Count == (ice1 ? 12 : 1), $"count: {proxyProps.Count}");
            TestHelper.Assert(proxyProps["Test"] == (ice1 ? "test -t -e 1.1" :
                "ice:test?invocation-timeout=10s&locator-cache-timeout=100s&prefer-existing-connection=true&prefer-non-secure=never"));

            if (ice1)
            {
                TestHelper.Assert(proxyProps["Test.InvocationTimeout"] == "10s");
                TestHelper.Assert(proxyProps["Test.PreferNonSecure"] == "Never");

                TestHelper.Assert(proxyProps["Test.Locator"] == "locator -t -e 1.1");
                TestHelper.Assert(proxyProps["Test.Locator.CacheConnection"] == "false");
                TestHelper.Assert(proxyProps["Test.LocatorCacheTimeout"] == "100s");
                TestHelper.Assert(proxyProps["Test.Locator.PreferExistingConnection"] == "false");
                TestHelper.Assert(proxyProps["Test.Locator.PreferNonSecure"] == "Always");
                TestHelper.Assert(proxyProps["Test.Locator.Router"] == "router -t -e 1.1");
                TestHelper.Assert(proxyProps["Test.Locator.Router.PreferNonSecure"] == "Always");
            }

            output.WriteLine("ok");

            output.Write("testing IObjectPrx.Communicator... ");
            output.Flush();
            TestHelper.Assert(baseProxy.Communicator == communicator);
            output.WriteLine("ok");

            output.Write("testing proxy Clone... ");

            TestHelper.Assert(baseProxy.Clone(IObjectPrx.Factory, facet: "facet").Facet == "facet");
            TestHelper.Assert(baseProxy.Clone(location: ImmutableArray.Create("id")).Location[0] == "id");

            TestHelper.Assert(!baseProxy.Clone(oneway: false).IsOneway);
            TestHelper.Assert(baseProxy.Clone(oneway: true).IsOneway);

            if (ice1)
            {
                TestHelper.Assert(!baseProxy.Clone(invocationMode: InvocationMode.Twoway).IsOneway);
                TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.Oneway).IsOneway);
            }

            IObjectPrx other = baseProxy.Clone(IObjectPrx.Factory, identityAndFacet: "test#facet");
            TestHelper.Assert(other.Facet == "facet");
            TestHelper.Assert(other.Identity.Name == "test");
            TestHelper.Assert(other.Identity.Category.Length == 0);

            other = other.Clone(IObjectPrx.Factory, identityAndFacet: "category/test");
            TestHelper.Assert(other.Facet.Length == 0);
            TestHelper.Assert(other.Identity.Name == "test");
            TestHelper.Assert(other.Identity.Category == "category");

            other = baseProxy.Clone(IObjectPrx.Factory, identityAndFacet: "foo#facet1");
            TestHelper.Assert(other.Facet == "facet1");
            TestHelper.Assert(other.Identity.Name == "foo");
            TestHelper.Assert(other.Identity.Category.Length == 0);

            if (ice1)
            {
                TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.Datagram).IsOneway);
                TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.BatchOneway).InvocationMode ==
                    InvocationMode.BatchOneway);
                TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.Datagram).InvocationMode ==
                    InvocationMode.Datagram);
                TestHelper.Assert(baseProxy.Clone(invocationMode: InvocationMode.BatchDatagram).InvocationMode ==
                    InvocationMode.BatchDatagram);
            }
            TestHelper.Assert(baseProxy.Clone(preferNonSecure: NonSecure.Always).PreferNonSecure == NonSecure.Always);
            TestHelper.Assert(baseProxy.Clone(preferNonSecure: NonSecure.Never).PreferNonSecure == NonSecure.Never);

            try
            {
                baseProxy.Clone(endpoints: ImmutableArray<Endpoint>.Empty,
                                locator: locator,
                                locatorCacheTimeout: TimeSpan.Zero);
            }
            catch (ArgumentException)
            {
                TestHelper.Assert(false);
            }

            try
            {
                baseProxy.Clone(endpoints: ImmutableArray<Endpoint>.Empty,
                                locator: locator,
                                locatorCacheTimeout: Timeout.InfiniteTimeSpan);
            }
            catch (ArgumentException)
            {
                TestHelper.Assert(false);
            }

            try
            {
                baseProxy.Clone(endpoints: ImmutableArray<Endpoint>.Empty,
                                locator: locator,
                                locatorCacheTimeout: TimeSpan.FromSeconds(-2));
                TestHelper.Assert(false);
            }
            catch (ArgumentException)
            {
            }

            output.WriteLine("ok");

            output.Write("testing proxy comparison... ");
            output.Flush();

            TestHelper.Assert(IObjectPrx.Parse("ice:foo", communicator).Equals(IObjectPrx.Parse("ice:foo", communicator)));
            TestHelper.Assert(!IObjectPrx.Parse("ice:foo", communicator).Equals(IObjectPrx.Parse("ice:foo2", communicator)));

            var compObj = IObjectPrx.Parse(ice1 ? "foo" : "ice:foo", communicator);

            TestHelper.Assert(compObj.Clone(IObjectPrx.Factory, facet: "facet").Equals(
                              compObj.Clone(IObjectPrx.Factory, facet: "facet")));
            TestHelper.Assert(!compObj.Clone(IObjectPrx.Factory, facet: "facet").Equals(
                              compObj.Clone(IObjectPrx.Factory, facet: "facet1")));

            TestHelper.Assert(compObj.Clone(oneway: true).Equals(compObj.Clone(oneway: true)));
            TestHelper.Assert(!compObj.Clone(oneway: true).Equals(compObj.Clone(oneway: false)));

            TestHelper.Assert(compObj.Clone(cacheConnection: true).Equals(compObj.Clone(cacheConnection: true)));
            TestHelper.Assert(!compObj.Clone(cacheConnection: false).Equals(compObj.Clone(cacheConnection: true)));

            TestHelper.Assert(compObj.Clone(label: "id2").Equals(compObj.Clone(label: "id2")));
            TestHelper.Assert(!compObj.Clone(label: "id1").Equals(compObj.Clone(label: "id2")));
            TestHelper.Assert(Equals(compObj.Clone(label: "id1").Label, "id1"));
            TestHelper.Assert(Equals(compObj.Clone(label: "id2").Label, "id2"));

            var loc1 = ILocatorPrx.Parse("ice+tcp://host:10000/loc1", communicator);
            var loc2 = ILocatorPrx.Parse("ice+tcp://host:10000/loc2", communicator);
            TestHelper.Assert(compObj.Clone(clearLocator: true).Equals(compObj.Clone(clearLocator: true)));
            TestHelper.Assert(compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc1)));
            TestHelper.Assert(!compObj.Clone(locator: loc1).Equals(compObj.Clone(clearLocator: true)));
            TestHelper.Assert(!compObj.Clone(clearLocator: true).Equals(compObj.Clone(locator: loc2)));
            TestHelper.Assert(!compObj.Clone(locator: loc1).Equals(compObj.Clone(locator: loc2)));

            if (ice1)
            {
                var rtr1 = IRouterPrx.Parse("rtr1:tcp -h host -p 10000", communicator);
                var rtr2 = IRouterPrx.Parse("rtr2:tcp -h host -p 10000", communicator);
                TestHelper.Assert(compObj.Clone(clearRouter: true).Equals(compObj.Clone(clearRouter: true)));
                TestHelper.Assert(compObj.Clone(router: rtr1).Equals(compObj.Clone(router: rtr1)));
                TestHelper.Assert(!compObj.Clone(router: rtr1).Equals(compObj.Clone(clearRouter: true)));
                TestHelper.Assert(!compObj.Clone(clearRouter: true).Equals(compObj.Clone(router: rtr2)));
                TestHelper.Assert(!compObj.Clone(router: rtr1).Equals(compObj.Clone(router: rtr2)));
            }

            var ctx1 = new Dictionary<string, string>
            {
                ["ctx1"] = "v1"
            };
            var ctx2 = new Dictionary<string, string>
            {
                ["ctx2"] = "v2"
            };
            TestHelper.Assert(compObj.Clone(context: new Dictionary<string, string>()).Equals(
                              compObj.Clone(context: new Dictionary<string, string>())));
            TestHelper.Assert(compObj.Clone(context: ctx1).Equals(compObj.Clone(context: ctx1)));
            TestHelper.Assert(!compObj.Clone(context: ctx1).Equals(
                              compObj.Clone(context: new Dictionary<string, string>())));
            TestHelper.Assert(!compObj.Clone(context: new Dictionary<string, string>()).Equals(
                              compObj.Clone(context: ctx2)));
            TestHelper.Assert(!compObj.Clone(context: ctx1).Equals(compObj.Clone(context: ctx2)));

            TestHelper.Assert(compObj.Clone(preferNonSecure: NonSecure.Always).Equals(
                              compObj.Clone(preferNonSecure: NonSecure.Always)));
            TestHelper.Assert(!compObj.Clone(preferNonSecure: NonSecure.Always).Equals(
                              compObj.Clone(preferNonSecure: NonSecure.Never)));

            var compObj1 = IObjectPrx.Parse("ice+tcp://127.0.0.1:10000/foo", communicator);
            var compObj2 = IObjectPrx.Parse("ice+tcp://127.0.0.1:10001/foo", communicator);
            TestHelper.Assert(!compObj1.Equals(compObj2));

            compObj1 = IObjectPrx.Parse("ice:MyAdapter1//foo", communicator);
            compObj2 = IObjectPrx.Parse("ice:MyAdapter2//foo", communicator);
            TestHelper.Assert(!compObj1.Equals(compObj2));

            TestHelper.Assert(compObj1.Clone(locator: locator, locatorCacheTimeout: TimeSpan.FromSeconds(20))
                .Equals(compObj1.Clone(locator: locator, locatorCacheTimeout: TimeSpan.FromSeconds(20))));
            TestHelper.Assert(!compObj1.Clone(locator: locator, locatorCacheTimeout: TimeSpan.FromSeconds(10))
                .Equals(compObj1.Clone(locator: locator, locatorCacheTimeout: TimeSpan.FromSeconds(20))));

            compObj1 = IObjectPrx.Parse("ice+tcp://127.0.0.1:10000/foo", communicator);
            compObj2 = IObjectPrx.Parse("ice:MyAdapter1//foo", communicator);
            TestHelper.Assert(!compObj1.Equals(compObj2));

            IReadOnlyList<Endpoint> endpts1 =
                IObjectPrx.Parse("ice+tcp://127.0.0.1:10000/foo", communicator).Endpoints;
            IReadOnlyList<Endpoint> endpts2 =
                IObjectPrx.Parse("ice+tcp://127.0.0.1:10001/foo", communicator).Endpoints;
            TestHelper.Assert(!endpts1[0].Equals(endpts2[0]));
            TestHelper.Assert(endpts1[0].Equals(
                IObjectPrx.Parse("ice+tcp://127.0.0.1:10000/foo", communicator).Endpoints[0]));

            if (await baseProxy.GetConnectionAsync() is IPConnection baseConnection)
            {
                Connection baseConnection2 = await baseProxy.Clone(label: "base2").GetConnectionAsync();
                compObj1 = compObj1.Clone(fixedConnection: baseConnection);
                compObj2 = compObj2.Clone(fixedConnection: baseConnection2);
                TestHelper.Assert(!compObj1.Equals(compObj2));
            }

            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            var cl = baseProxy.CheckedCast(IMyClassPrx.Factory);
            TestHelper.Assert(cl != null);
            var derived = cl.CheckedCast(IMyDerivedClassPrx.Factory);
            TestHelper.Assert(derived != null);
            TestHelper.Assert(cl.Equals(baseProxy));
            TestHelper.Assert(derived.Equals(baseProxy));
            TestHelper.Assert(cl.Equals(derived));
            try
            {
                cl.Clone(IMyDerivedClassPrx.Factory, facet: "facet").IcePing();
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
            }
            output.WriteLine("ok");

            output.Write("testing checked cast with context... ");
            output.Flush();

            SortedDictionary<string, string> c = cl.GetContext();
            TestHelper.Assert(c == null || c.Count == 0);

            c = new SortedDictionary<string, string>
            {
                ["one"] = "hello",
                ["two"] = "world"
            };
            cl = baseProxy.CheckedCast(IMyClassPrx.Factory, c);
            SortedDictionary<string, string> c2 = cl!.GetContext();
            TestHelper.Assert(c.DictionaryEqual(c2));
            output.WriteLine("ok");

            output.Write("testing location... ");
            var shortLocation = ImmutableArray.Create("loc0");
            var longLocation = ImmutableArray.Create("locA", "locB", "locC", "locD");
            if (ice1)
            {
                var prx = baseProxy.Clone(factory: IMyDerivedClassPrx.Factory);

                prx = prx.Clone(location: shortLocation);
                TestHelper.Assert(prx.Endpoints.Count == 0); // and can't call it

                try
                {
                    prx = prx.Clone(location: longLocation);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                    // too many segments
                }
            }
            else
            {
                var prx = baseProxy.Clone(factory: IMyDerivedClassPrx.Factory, location: shortLocation);
                TestHelper.Assert(prx.GetLocation().SequenceEqual(shortLocation));

                TestHelper.Assert(prx.Clone(location: longLocation).GetLocation().SequenceEqual(longLocation));
            }
            output.WriteLine("ok");

            if (ice1)
            {
                output.Write("testing ice2 proxy in 1.1 encapsulation... ");
                output.Flush();
                var ice2Prx = IObjectPrx.Parse(
                    "ice+tcp://localhost:10000/foo?alt-endpoint=ice+ws://localhost:10000", communicator);
                var prx = baseProxy.Clone(IMyDerivedClassPrx.Factory).Echo(ice2Prx);
                TestHelper.Assert(ice2Prx.Equals(prx));

                // With a location dropped by the 1.1 encoding:
                ice2Prx = IObjectPrx.Parse("ice+tcp://localhost:10000/location//foo", communicator);
                prx = baseProxy.Clone(IMyDerivedClassPrx.Factory).Echo(ice2Prx);
                TestHelper.Assert(ice2Prx.Clone(location: ImmutableArray<string>.Empty).Equals(prx));

                // With a multi-segment location without endpoints (only keep first segment)
                ice2Prx = IObjectPrx.Parse("ice:loc0/loc1//foo", communicator);
                prx = baseProxy.Clone(IMyDerivedClassPrx.Factory).Echo(ice2Prx);
                TestHelper.Assert(ice2Prx.Clone(location: ImmutableArray.Create("loc0")).Equals(prx));

                output.WriteLine("ok");
            }
            else
            {
                output.Write("testing ice1 proxy in 2.0 encapsulation... ");
                output.Flush();
                var ice1Prx = IObjectPrx.Parse(
                    "foo:tcp -h localhost -p 10000:udp -h localhost -p 10000", communicator);
                var prx = baseProxy.Clone(IMyDerivedClassPrx.Factory).Echo(ice1Prx);
                TestHelper.Assert(ice1Prx.Equals(prx));
                output.WriteLine("ok");
            }

            output.Write("testing relative proxies... ");
            {
                // The Clone(encoding: Encoding.V20) are only for ice1; with ice2, it's the default encoding. We need
                // to marshal all relative proxies with the 2.0 encoding.

                await using ObjectAdapter oa = communicator.CreateObjectAdapter(protocol: helper.Protocol);
                (await cl.GetConnectionAsync()).Adapter = oa;
                ICallbackPrx callback = oa.AddWithUUID(
                    new Callback((relativeTest, current, cancel) =>
                                 {
                                     TestHelper.Assert(relativeTest.IsFixed);
                                     return relativeTest.DoIt(cancel: cancel);
                                 }),
                    ICallbackPrx.Factory).Clone(encoding: Encoding.V20, relative: true);
                TestHelper.Assert(callback.IsRelative);
                callback.IcePing(); // colocated call

                IRelativeTestPrx relativeTest = cl.Clone(encoding: Encoding.V20).OpRelative(callback);
                TestHelper.Assert(relativeTest.Endpoints == cl.Endpoints); // reference equality
                TestHelper.Assert(!relativeTest.IsRelative);
                TestHelper.Assert(relativeTest.DoIt() == 2);
            }
            output.WriteLine("ok");

            output.Write("testing ice_fixed... ");
            output.Flush();
            {
                if (await cl.GetConnectionAsync() is Connection connection2)
                {
                    TestHelper.Assert(!cl.IsFixed);
                    IMyClassPrx prx = cl.Clone(fixedConnection: connection2);
                    TestHelper.Assert(prx.IsFixed);
                    prx.IcePing();
                    TestHelper.Assert(cl.Clone(IObjectPrx.Factory,
                                               facet: "facet",
                                               fixedConnection: connection2).Facet == "facet");
                    TestHelper.Assert(cl.Clone(oneway: true, fixedConnection: connection2).IsOneway);
                    var ctx = new Dictionary<string, string>
                    {
                        ["one"] = "hello",
                        ["two"] = "world"
                    };
                    TestHelper.Assert(cl.Clone(fixedConnection: connection2).Context.Count == 0);
                    TestHelper.Assert(cl.Clone(context: ctx, fixedConnection: connection2).Context.Count == 2);
                    TestHelper.Assert(await cl.Clone(fixedConnection: connection2).GetConnectionAsync() == connection2);
                    TestHelper.Assert(await cl.Clone(fixedConnection: connection2).Clone(fixedConnection: connection2).GetConnectionAsync() == connection2);
                    Connection? fixedConnection = await cl.Clone(label: "ice_fixed").GetConnectionAsync();
                    TestHelper.Assert(await cl.Clone(fixedConnection: connection2).Clone(fixedConnection: fixedConnection).GetConnectionAsync() == fixedConnection);
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
            string ref13 = helper.GetTestProxy("test", 0);
            IMyClassPrx cl13 = IMyClassPrx.Parse(ref13, communicator).Clone(encoding: new Encoding(1, 3));
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

            if (helper.Protocol == Protocol.Ice2)
            {
                output.Write("testing protocol versioning... ");
                output.Flush();
                string ref3 = helper.GetTestProxy("test", 0);
                ref3 += "?protocol=3";

                string transport = helper.Transport;
                ref3 = ref3.Replace($"ice+{transport}", "ice+universal");
                ref3 += $"&transport={transport}";
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
            }

            output.Write("testing ice2 universal endpoints... ");
            output.Flush();

            var p1 = IObjectPrx.Parse("ice+universal://127.0.0.1:4062/test?transport=tcp", communicator);
            TestHelper.Assert(p1.ToString() == "ice+tcp://127.0.0.1/test"); // uses default port

            p1 = IObjectPrx.Parse(
                "ice+universal://127.0.0.1:4062/test?transport=tcp&alt-endpoint=host2:10000?transport=tcp",
                communicator);
            TestHelper.Assert(p1.ToString() == "ice+tcp://127.0.0.1/test?alt-endpoint=host2:10000");

            p1 = IObjectPrx.Parse(
                "ice+universal://127.0.0.1:4062/test?transport=tcp&alt-endpoint=host2:10000?transport=99$option=a",
                communicator);

            TestHelper.Assert(p1.ToString() ==
                "ice+tcp://127.0.0.1/test?alt-endpoint=ice+universal://host2:10000?transport=99$option=a");

            output.WriteLine("ok");

            output.Write("testing ice1 opaque endpoints... ");
            output.Flush();

            // Legal TCP endpoint expressed as opaque endpoint
            p1 = IObjectPrx.Parse("test:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==",
                                      communicator);
            TestHelper.Assert(p1.ToString() == "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000");

            // Two legal TCP endpoints expressed as opaque endpoints
            p1 = IObjectPrx.Parse(@"test:opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMeouAAAQJwAAAA==:
                opaque -e 1.1 -t 1 -v CTEyNy4wLjAuMusuAAAQJwAAAA==", communicator);
            TestHelper.Assert(p1.ToString() ==
                "test -t -e 1.1:tcp -h 127.0.0.1 -p 12010 -t 10000:tcp -h 127.0.0.2 -p 12011 -t 10000");

            // Test that an SSL endpoint and an unknown-transport endpoint get written back out as an opaque
            // endpoint.
            p1 = IObjectPrx.Parse(
                "test:opaque -e 1.1 -t 2 -v CTEyNy4wLjAuMREnAAD/////AA==:opaque -e 1.1 -t 99 -v abch",
                communicator);
            TestHelper.Assert(p1.ToString() ==
                "test -t -e 1.1:ssl -h 127.0.0.1 -p 10001 -t -1:opaque -t 99 -e 1.1 -v abch");

            output.WriteLine("ok");

            // TODO test communicator destroy in its own test
            output.Write("testing communicator shutdown/destroy... ");
            output.Flush();
            {
                var com = new Communicator();
                await com.ShutdownAsync();
                await com.DisposeAsync();
                await com.ShutdownAsync();
                await com.DisposeAsync();
            }
            output.WriteLine("ok");

            output.Write("testing communicator default source address... ");
            output.Flush();
            {
                await using var comm1 = new Communicator(new Dictionary<string, string>()
                    {
                        { "Ice.Default.SourceAddress", "192.168.1.40" }
                    });

                await using var comm2 = new Communicator();

                string[] proxyArray =
                    {
                        "ice+tcp://host.zeroc.com/identity#facet",
                        "ice -t:tcp -h localhost -p 10000",
                    };

                foreach (string s in proxyArray)
                {
                    var prx = IObjectPrx.Parse(s, comm1);
                    TestHelper.Assert(prx.Endpoints[0]["source-address"] == "192.168.1.40");
                    prx = IObjectPrx.Parse(s, comm2);
                    TestHelper.Assert(prx.Endpoints[0]["source-address"] == null);
                }
            }
            output.WriteLine("ok");

            output.Write("testing communicator default invocation timeout... ");
            output.Flush();
            {
                await using var comm1 = new Communicator(new Dictionary<string, string>()
                    {
                        { "Ice.Default.InvocationTimeout", "120s" }
                    });

                await using var comm2 = new Communicator();

                TestHelper.Assert(IObjectPrx.Parse("ice+tcp://localhost/identity", comm1).InvocationTimeout ==
                                  TimeSpan.FromSeconds(120));

                TestHelper.Assert(IObjectPrx.Parse("ice+tcp://localhost/identity", comm2).InvocationTimeout ==
                                  TimeSpan.FromSeconds(60));

                TestHelper.Assert(IObjectPrx.Parse("ice+tcp://localhost/identity?invocation-timeout=10s",
                                                   comm1).InvocationTimeout == TimeSpan.FromSeconds(10));

                TestHelper.Assert(IObjectPrx.Parse("ice+tcp://localhost/identity?invocation-timeout=10s",
                                                   comm2).InvocationTimeout == TimeSpan.FromSeconds(10));

                TestHelper.Assert(IObjectPrx.Parse("identity -t:tcp -h localhost", comm1).InvocationTimeout ==
                                 TimeSpan.FromSeconds(120));

                TestHelper.Assert(IObjectPrx.Parse("identity -t:tcp -h localhost", comm2).InvocationTimeout ==
                                  TimeSpan.FromSeconds(60));
            }
            output.WriteLine("ok");

            output.Write("testing invalid invocation timeout... ");
            output.Flush();
            {
                try
                {
                    await using var comm1 = new Communicator(new Dictionary<string, string>()
                    {
                        { "Ice.Default.InvocationTimeout", "0s" }
                    });
                    TestHelper.Assert(false);
                }
                catch (InvalidConfigurationException)
                {
                }

                try
                {
                    IObjectPrx.Parse("ice+tcp://localhost/identity", communicator).Clone(
                        invocationTimeout: TimeSpan.Zero);
                    TestHelper.Assert(false);
                }
                catch (ArgumentException)
                {
                }
            }
            output.WriteLine("ok");

            await cl.ShutdownAsync();
        }
    }

    internal delegate int CallbackDelegate(IRelativeTestPrx relativeTest, Current current, CancellationToken cancel);

    internal sealed class Callback : ICallback
    {
        private CallbackDelegate _delegate;

        public int Op(IRelativeTestPrx relativeTest, Current current, CancellationToken cancel) =>
            _delegate(relativeTest, current, cancel);

        internal Callback(CallbackDelegate @delegate) => _delegate = @delegate;
    }
}
