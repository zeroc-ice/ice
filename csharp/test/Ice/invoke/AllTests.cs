// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Invoke
{
    public static class AllTests
    {
        private const string TestString = "This is a test string";

        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            bool ice1 = helper.Protocol == Protocol.Ice1;
            var cl = IMyClassPrx.Parse(helper.GetTestProxy("test", 0), communicator);
            IMyClassPrx oneway = cl.Clone(oneway: true);

            System.IO.TextWriter output = helper.Output;
            output.Write("testing InvokeAsync... ");
            output.Flush();

            {
                using var request = OutgoingRequestFrame.WithEmptyArgs(oneway, "opOneway", idempotent: false);

                try
                {
                    using IncomingResponseFrame response = oneway.InvokeAsync(request, oneway: true).Result;
                }
                catch
                {
                    TestHelper.Assert(false);
                }
            }

            {
                using var request = OutgoingRequestFrame.WithArgs(cl,
                                                                  "opString",
                                                                  idempotent: false,
                                                                  compress: false,
                                                                  format: default,
                                                                  context: null,
                                                                  TestString,
                                                                  OutputStream.IceWriterFromString);

                using IncomingResponseFrame response = cl.InvokeAsync(request).Result;
                (string s1, string s2) = response.ReadReturnValue(cl, istr =>
                    {
                        string s1 = istr.ReadString();
                        string s2 = istr.ReadString();
                        return (s1, s2);
                    });
                TestHelper.Assert(s1.Equals(TestString));
                TestHelper.Assert(s2.Equals(TestString));
            }

            {
                using var request = OutgoingRequestFrame.WithEmptyArgs(cl, "opException", idempotent: false);
                using IncomingResponseFrame response = cl.InvokeAsync(request).Result;

                try
                {
                    response.ReadVoidReturnValue(cl);
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
            await cl.ShutdownAsync();
        }
    }
}
