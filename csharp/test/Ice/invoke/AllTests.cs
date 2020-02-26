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
            output.Write("testing ice_invoke... ");
            output.Flush();

            {
                byte[] inEncaps, outEncaps;
                if (!oneway.Invoke("opOneway", idempotent: false, null, out outEncaps))
                {
                    test(false);
                }

                OutputStream outS = new OutputStream(communicator);
                outS.StartEncapsulation();
                outS.WriteString(testString);
                outS.EndEncapsulation();
                inEncaps = outS.ToArray();

                if (cl.Invoke("opString", idempotent: false, inEncaps, out outEncaps))
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

                if (cl.Invoke("opException", idempotent: false, null, out outEncaps, ctx))
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

            output.Write("testing asynchronous ice_invoke with Async Task API... ");
            output.Flush();

            {
                try
                {
                    oneway.InvokeAsync("opOneway", idempotent: false, null).Wait();
                }
                catch (Exception)
                {
                    test(false);
                }

                OutputStream outS = new OutputStream(communicator);
                outS.StartEncapsulation();
                outS.WriteString(testString);
                outS.EndEncapsulation();
                byte[] inEncaps = outS.ToArray();

                // begin_ice_invoke with no callback
                var result = cl.InvokeAsync("opString", idempotent: false, inEncaps).Result;
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
                var result = cl.InvokeAsync("opException", idempotent: false, null).Result;
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
