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
                var request = OutgoingRequestFrame.Empty(oneway, "opOneway", idempotent: false);

                // Whether the proxy is oneway or not does not matter for Invoke's oneway parameter.

                var response = cl.Invoke(request, oneway: true);
                test(response.ReplyStatus == ReplyStatus.OK);

                response = cl.Invoke(request, oneway: false);
                test(response.ReplyStatus == ReplyStatus.UserException);

                response = oneway.Invoke(request, oneway: true);
                test(response.ReplyStatus == ReplyStatus.OK);

                response = oneway.Invoke(request, oneway: false);
                test(response.ReplyStatus == ReplyStatus.UserException);

                request = new OutgoingRequestFrame(cl, "opString", idempotent: false);
                OutputStream ostr = request.WritePayload();
                ostr.WriteString(testString);
                ostr.Save();
                response = cl.Invoke(request);
                var result = response.ReadResult();
                test(result.ResultType == ResultType.Success);
                string s = result.InputStream.ReadString();
                test(s.Equals(testString));
                s = result.InputStream.ReadString();
                result.InputStream.EndEncapsulation();
                test(s.Equals(testString));
            }

            for (int i = 0; i < 2; ++i)
            {
                Dictionary<string, string>? ctx = null;
                if (i == 1)
                {
                    ctx = new Dictionary<string, string>();
                    ctx["raise"] = "";
                }

                var request = OutgoingRequestFrame.Empty(cl, "opException", idempotent: false, context: ctx);
                var response = cl.Invoke(request);
                var result = response.ReadResult();
                test(result.ResultType == ResultType.Failure);
                try
                {
                    result.InputStream.ThrowException();
                }
                catch (Test.MyException)
                {
                    result.InputStream.EndEncapsulation();
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
                var request = OutgoingRequestFrame.Empty(oneway, "opOneway", idempotent: false);
                try
                {
                    oneway.InvokeAsync(request, oneway: true).Wait();
                }
                catch (System.Exception)
                {
                    test(false);
                }

                request = new OutgoingRequestFrame(cl, "opString", idempotent: false);
                OutputStream ostr = request.WritePayload();
                ostr.WriteString(testString);
                ostr.Save();

                var response = cl.InvokeAsync(request).Result;
                var result = response.ReadResult();
                test(result.ResultType == ResultType.Success);
                string s = result.InputStream.ReadString();
                test(s.Equals(testString));
                s = result.InputStream.ReadString();
                result.InputStream.EndEncapsulation();
                test(s.Equals(testString));
            }

            {
                var request = OutgoingRequestFrame.Empty(cl, "opException", idempotent: false);
                var response = cl.InvokeAsync(request).Result;
                var result = response.ReadResult();
                test(result.ResultType == ResultType.Failure);

                try
                {
                    result.InputStream.ThrowException();
                }
                catch (Test.MyException)
                {
                    result.InputStream.EndEncapsulation();
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
