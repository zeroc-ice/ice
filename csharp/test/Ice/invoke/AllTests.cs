// Copyright (c) ZeroC, Inc. All rights reserved.

using Test;

namespace ZeroC.Ice.Test.Invoke
{
    public static class AllTests
    {
        private const string TestString = "This is a test string";

        public static IMyClassPrx Run(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator;
            TestHelper.Assert(communicator != null);
            bool ice1 = helper.Protocol == Protocol.Ice1;
            var cl = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            IMyClassPrx oneway = cl.Clone(oneway: true);

            System.IO.TextWriter output = helper.Output;
            output.Write("testing InvokeAsync... ");
            output.Flush();

            {
                var request = OutgoingRequestFrame.WithEmptyArgs(oneway, "opOneway", idempotent: false);
                IncomingResponseFrame response;
                try
                {
                    response = oneway.InvokeAsync(request, oneway: true).Result;
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                request = OutgoingRequestFrame.WithArgs(cl,
                                                        "opString",
                                                        idempotent: false,
                                                        compress: false,
                                                        format: default,
                                                        context: null,
                                                        TestString,
                                                        OutputStream.IceWriterFromString);

                response = cl.InvokeAsync(request).Result;
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
                var request = OutgoingRequestFrame.WithEmptyArgs(cl, "opException", idempotent: false);
                IncomingResponseFrame response = cl.InvokeAsync(request).Result;

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
