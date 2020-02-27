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
                var requestFrame = OutgoingRequestFrame.Empty(oneway, "opOneway", idempotent: false);
                var responseFrame = oneway.Invoke(requestFrame);
                test(responseFrame.ReplyStatus == ReplyStatus.OK);

                requestFrame = OutgoingRequestFrame.Start(cl, "opString", idempotent: false);
                requestFrame.WriteString(testString);
                requestFrame.EndPayload();

                responseFrame = cl.Invoke(requestFrame);
                test(responseFrame.ReplyStatus == ReplyStatus.OK);
                responseFrame.InputStream.StartEncapsulation();
                string s = responseFrame.InputStream.ReadString();
                test(s.Equals(testString));
                s = responseFrame.InputStream.ReadString();
                responseFrame.InputStream.EndEncapsulation();
                test(s.Equals(testString));
            }

            for (int i = 0; i < 2; ++i)
            {
                Dictionary<string, string> ctx = null;
                if (i == 1)
                {
                    ctx = new Dictionary<string, string>();
                    ctx["raise"] = "";
                }

                var requestFrame = OutgoingRequestFrame.Empty(cl, "opException", idempotent: false, context: ctx);

                var responseFrame = cl.Invoke(requestFrame);
                test(responseFrame.ReplyStatus == ReplyStatus.UserException);
                responseFrame.InputStream.StartEncapsulation();
                try
                {
                    responseFrame.InputStream.ThrowException();
                }
                catch (Test.MyException)
                {
                    responseFrame.InputStream.EndEncapsulation();
                }
                catch (Exception)
                {
                    test(false);
                }
            }

            output.WriteLine("ok");

            output.Write("testing InvokeAsync... ");
            output.Flush();

            {
                var requestFrame = OutgoingRequestFrame.Empty(oneway, "opOneway", idempotent: false);
                try
                {
                    oneway.InvokeAsync(requestFrame).Wait();
                }
                catch (Exception)
                {
                    test(false);
                }

                requestFrame = OutgoingRequestFrame.Start(cl, "opString", idempotent: false);
                requestFrame.WriteString(testString);
                requestFrame.EndPayload();

                var responseFrame = cl.InvokeAsync(requestFrame).Result;
                test(responseFrame.ReplyStatus == 0);
                responseFrame.InputStream.StartEncapsulation();
                string s = responseFrame.InputStream.ReadString();
                test(s.Equals(testString));
                s = responseFrame.InputStream.ReadString();
                responseFrame.InputStream.EndEncapsulation();
                test(s.Equals(testString));
            }

            {
                var requestFrame = OutgoingRequestFrame.Empty(cl, "opException", idempotent: false);
                var responseFrame = cl.InvokeAsync(requestFrame).Result;
                test(responseFrame.ReplyStatus == ReplyStatus.UserException);

                responseFrame.InputStream.StartEncapsulation();
                try
                {
                    responseFrame.InputStream.ThrowException();
                }
                catch (Test.MyException)
                {
                    responseFrame.InputStream.EndEncapsulation();
                }
                catch (Exception)
                {
                    test(false);
                }
            }

            output.WriteLine("ok");
            return cl;
        }
    }
}
