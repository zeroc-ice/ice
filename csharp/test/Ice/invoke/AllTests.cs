//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    namespace invoke
    {
        public class AllTests : global::Test.AllTests
        {
            private static string testString = "This is a test string";

            public static Test.MyClassPrx allTests(global::Test.TestHelper helper)
            {
                Communicator communicator = helper.communicator();
                var cl = Test.MyClassPrx.Parse($"test:{helper.getTestEndpoint(0)}", communicator);
                var oneway = cl.Clone(oneway: true);

                var output = helper.getWriter();
                output.Write("testing ice_invoke... ");
                output.Flush();

                {
                    byte[] inEncaps, outEncaps;
                    if (!oneway.Invoke("opOneway", OperationMode.Normal, null, out outEncaps))
                    {
                        test(false);
                    }

                    OutputStream outS = new OutputStream(communicator);
                    outS.startEncapsulation();
                    outS.writeString(testString);
                    outS.endEncapsulation();
                    inEncaps = outS.finished();

                    if (cl.Invoke("opString", OperationMode.Normal, inEncaps, out outEncaps))
                    {
                        InputStream inS = new InputStream(communicator, outEncaps);
                        inS.startEncapsulation();
                        string s = inS.readString();
                        test(s.Equals(testString));
                        s = inS.readString();
                        inS.endEncapsulation();
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

                    if (cl.Invoke("opException", OperationMode.Normal, null, out outEncaps, ctx))
                    {
                        test(false);
                    }
                    else
                    {
                        InputStream inS = new InputStream(communicator, outEncaps);
                        inS.startEncapsulation();

                        try
                        {
                            inS.throwException();
                        }
                        catch (Test.MyException)
                        {
                            inS.endEncapsulation();
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
                        oneway.InvokeAsync("opOneway", OperationMode.Normal, null).Wait();
                    }
                    catch (Exception)
                    {
                        test(false);
                    }

                    OutputStream outS = new OutputStream(communicator);
                    outS.startEncapsulation();
                    outS.writeString(testString);
                    outS.endEncapsulation();
                    byte[] inEncaps = outS.finished();

                    // begin_ice_invoke with no callback
                    var result = cl.InvokeAsync("opString", OperationMode.Normal, inEncaps).Result;
                    if (result.returnValue)
                    {
                        InputStream inS = new InputStream(communicator, result.outEncaps);
                        inS.startEncapsulation();
                        string s = inS.readString();
                        test(s.Equals(testString));
                        s = inS.readString();
                        inS.endEncapsulation();
                        test(s.Equals(testString));
                    }
                    else
                    {
                        test(false);
                    }
                }

                {
                    var result = cl.InvokeAsync("opException", OperationMode.Normal, null).Result;
                    if (result.returnValue)
                    {
                        test(false);
                    }
                    else
                    {
                        InputStream inS = new InputStream(communicator, result.outEncaps);
                        inS.startEncapsulation();
                        try
                        {
                            inS.throwException();
                        }
                        catch (Test.MyException)
                        {
                            inS.endEncapsulation();
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
}
