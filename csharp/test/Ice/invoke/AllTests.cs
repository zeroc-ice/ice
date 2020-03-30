//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.invoke
{
    public class AllTests : global::Test.AllTests
    {
        private static string testString = "This is a test string";

        public static Test.IMyClassPrx allTests(global::Test.TestHelper helper)
        {
            Communicator communicator = helper.communicator();
            var cl = Test.IMyClassPrx.Parse($"test:{helper.getTestEndpoint(0)}", communicator);
            var oneway = cl.Clone(oneway: true);

            var output = helper.getWriter();
            output.Write("testing Invoke... ");
            output.Flush();

            {
                var request = OutgoingRequestFrame.WithNoParameter(oneway, "opOneway", idempotent: false);

                // Whether the proxy is oneway or not does not matter for Invoke's oneway parameter.

                var response = cl.Invoke(request, oneway: true);
                test(response.ReplyStatus == ReplyStatus.OK);

                response = cl.Invoke(request, oneway: false);
                test(response.ReplyStatus == ReplyStatus.UserException);

                response = oneway.Invoke(request, oneway: true);
                test(response.ReplyStatus == ReplyStatus.OK);

                response = oneway.Invoke(request, oneway: false);
                test(response.ReplyStatus == ReplyStatus.UserException);

                request = OutgoingRequestFrame.WithParameters(cl, "opString", idempotent: false,
                    format: null, context: null, testString, OutputStream.IceWriterFromString);
                response = cl.Invoke(request);
                (string s1, string s2) = response.ReadReturnValue(istr =>
                    {
                        string s1 = istr.ReadString();
                        string s2 = istr.ReadString();
                        return (s1, s2);
                    });
                test(s1.Equals(testString) && s2.Equals(testString));
            }

            for (int i = 0; i < 2; ++i)
            {
                Dictionary<string, string>? ctx = null;
                if (i == 1)
                {
                    ctx = new Dictionary<string, string>();
                    ctx["raise"] = "";
                }

                var request = OutgoingRequestFrame.WithNoParameter(cl, "opException", idempotent: false, context: ctx);
                var response = cl.Invoke(request);
                try
                {
                    response.ReadVoidReturnValue();
                }
                catch (Test.MyException)
                {
                    // expected
                }
                catch (System.Exception)
                {
                    test(false);
                }
            }

            output.WriteLine("ok");

            output.Write("testing InvokeAsync... ");
            output.Flush();

            {
                var request = OutgoingRequestFrame.WithNoParameter(oneway, "opOneway", idempotent: false);
                try
                {
                    oneway.InvokeAsync(request, oneway: true).Wait();
                }
                catch (System.Exception)
                {
                    test(false);
                }

                request = OutgoingRequestFrame.WithParameters(cl, "opString", idempotent: false,
                    format: null, context: null, testString, OutputStream.IceWriterFromString);

                IncomingResponseFrame response = cl.InvokeAsync(request).Result;
                (string s1, string s2) = response.ReadReturnValue(istr =>
                    {
                        string s1 = istr.ReadString();
                        string s2 = istr.ReadString();
                        return (s1, s2);
                    });
                test(s1.Equals(testString));
                test(s2.Equals(testString));
            }

            {
                var request = OutgoingRequestFrame.WithNoParameter(cl, "opException", idempotent: false);
                var response = cl.InvokeAsync(request).Result;

                try
                {
                    response.ReadVoidReturnValue();
                    test(false);
                }
                catch (Test.MyException)
                {
                }
                catch (System.Exception)
                {
                    test(false);
                }
            }

            output.WriteLine("ok");
            return cl;
        }
    }
}
