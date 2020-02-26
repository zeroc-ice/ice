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
                byte[] outEncaps;
                if (!oneway.Invoke(requestFrame, out outEncaps))
                {
                    test(false);
                }

                requestFrame = new OutgoingRequestFrame(cl, "opString", idempotent: false);
                requestFrame.WriteString(testString);
                requestFrame.EndParameters();

                if (cl.Invoke(requestFrame, out outEncaps))
                {
                    InputStream inS = new InputStream(communicator, outEncaps);
                    inS.StartEncapsulation();
                    string s = inS.ReadString();
                    test(s.Equals(testString));
                    s = inS.ReadString();
                    inS.EndEncapsulation();
                    test(s.Equals(testString));
                }
                else
                {
                    test(false);
                }
            }

            for (int i = 0; i < 2; ++i)
            {
                byte[] outEncaps;
                Dictionary<string, string> ctx = null;
                if (i == 1)
                {
                    ctx = new Dictionary<string, string>();
                    ctx["raise"] = "";
                }

                var requestFrame = OutgoingRequestFrame.Empty(cl, "opException", idempotent: false, context: ctx);

                if (cl.Invoke(requestFrame, out outEncaps))
                {
                    test(false);
                }
                else
                {
                    InputStream inS = new InputStream(communicator, outEncaps);
                    inS.StartEncapsulation();
                    try
                    {
                        inS.ThrowException();
                    }
                    catch (Test.MyException)
                    {
                        inS.EndEncapsulation();
                    }
                    catch (Exception)
                    {
                        test(false);
                    }
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

                requestFrame = new OutgoingRequestFrame(cl, "opString", idempotent: false);
                requestFrame.WriteString(testString);
                requestFrame.EndParameters();

                var result = cl.InvokeAsync(requestFrame).Result;
                if (result.ReturnValue)
                {
                    InputStream inS = new InputStream(communicator, result.OutEncaps);
                    inS.StartEncapsulation();
                    string s = inS.ReadString();
                    test(s.Equals(testString));
                    s = inS.ReadString();
                    inS.EndEncapsulation();
                    test(s.Equals(testString));
                }
                else
                {
                    test(false);
                }
            }

            {
                var requestFrame = OutgoingRequestFrame.Empty(cl, "opException", idempotent: false);
                var result = cl.InvokeAsync(requestFrame).Result;
                if (result.ReturnValue)
                {
                    test(false);
                }
                else
                {
                    InputStream inS = new InputStream(communicator, result.OutEncaps);
                    inS.StartEncapsulation();
                    try
                    {
                        inS.ThrowException();
                    }
                    catch (Test.MyException)
                    {
                        inS.EndEncapsulation();
                    }
                    catch (Exception)
                    {
                        test(false);
                    }
                }
            }

            output.WriteLine("ok");
            return cl;
        }
    }

}
