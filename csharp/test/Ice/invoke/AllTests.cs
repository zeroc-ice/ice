//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Invoke
{
    public class AllTests
    {
        private const string TestString = "This is a test string";

        public static IMyClassPrx Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            bool ice1 = communicator.DefaultProtocol == Protocol.Ice1;
            var cl = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            IMyClassPrx oneway = cl.Clone(oneway: true);

            System.IO.TextWriter output = helper.GetWriter();
            output.Write("testing Invoke... ");
            output.Flush();

            {
                var request = OutgoingRequestFrame.WithEmptyParamList(oneway, "opOneway", idempotent: false);

                // Whether the proxy is oneway or not does not matter for Invoke's oneway parameter.

                IncomingResponseFrame response = cl.Invoke(request, oneway: true);
                if (ice1)
                {
                    TestHelper.Assert(response.ResultType == ResultType.Success);
                }

                response = cl.Invoke(request, oneway: false);
                if (ice1)
                {
                    TestHelper.Assert(response.ResultType == ResultType.Failure);
                }

                response = oneway.Invoke(request, oneway: true);
                if (ice1)
                {
                    TestHelper.Assert(response.ResultType == ResultType.Success);
                }

                response = oneway.Invoke(request, oneway: false);
                if (ice1)
                {
                    TestHelper.Assert(response.ResultType == ResultType.Failure);
                }

                request = OutgoingRequestFrame.WithParamList(cl,
                                                             "opString",
                                                             idempotent: false,
                                                             compress: false,
                                                             format: default,
                                                             context: null,
                                                             TestString,
                                                             OutputStream.IceWriterFromString);
                response = cl.Invoke(request);
                (string s1, string s2) = response.ReadReturnValue(communicator, istr =>
                    {
                        string s1 = istr.ReadString();
                        string s2 = istr.ReadString();
                        return (s1, s2);
                    });
                TestHelper.Assert(s1.Equals(TestString) && s2.Equals(TestString));
            }

            for (int i = 0; i < 2; ++i)
            {
                Dictionary<string, string>? ctx = null;
                if (i == 1)
                {
                    ctx = new Dictionary<string, string>
                    {
                        ["raise"] = ""
                    };
                }

                var request = OutgoingRequestFrame.WithEmptyParamList(cl, "opException", idempotent: false, context: ctx);
                IncomingResponseFrame response = cl.Invoke(request);
                try
                {
                    response.ReadVoidReturnValue(communicator);
                }
                catch (MyException)
                {
                    // expected
                }
                catch
                {
                    TestHelper.Assert(false);
                }
            }

            output.WriteLine("ok");

            output.Write("testing InvokeAsync... ");
            output.Flush();

            {
                var request = OutgoingRequestFrame.WithEmptyParamList(oneway, "opOneway", idempotent: false);
                IncomingResponseFrame response;
                try
                {
                    response = oneway.InvokeAsync(request, oneway: true).AsTask().Result;
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                request = OutgoingRequestFrame.WithParamList(cl,
                                                             "opString",
                                                             idempotent: false,
                                                             compress: false,
                                                             format: default,
                                                             context: null,
                                                             TestString,
                                                             OutputStream.IceWriterFromString);

                response = cl.InvokeAsync(request).AsTask().Result;
                (string s1, string s2) = response.ReadReturnValue(communicator, istr =>
                    {
                        string s1 = istr.ReadString();
                        string s2 = istr.ReadString();
                        return (s1, s2);
                    });
                TestHelper.Assert(s1.Equals(TestString));
                TestHelper.Assert(s2.Equals(TestString));
            }

            {
                var request = OutgoingRequestFrame.WithEmptyParamList(cl, "opException", idempotent: false);
                IncomingResponseFrame response = cl.InvokeAsync(request).AsTask().Result;

                try
                {
                    response.ReadVoidReturnValue(communicator);
                    TestHelper.Assert(false);
                }
                catch (MyException)
                {
                }
                catch
                {
                    TestHelper.Assert(false);
                }
            }

            output.WriteLine("ok");
            return cl;
        }
    }
}
