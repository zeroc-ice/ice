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
                ObjectPrx baseProxy = communicator.stringToProxy("test:" + helper.getTestEndpoint(0));
                var cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
                var oneway = Test.MyClassPrxHelper.uncheckedCast(cl.ice_oneway());
                var batchOneway = Test.MyClassPrxHelper.uncheckedCast(cl.ice_batchOneway());

                var output = helper.getWriter();
                output.Write("testing ice_invoke... ");
                output.Flush();

                {
                    byte[] inEncaps, outEncaps;
                    if (!oneway.ice_invoke("opOneway", OperationMode.Normal, null, out outEncaps))
                    {
                        test(false);
                    }

                    test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null, out outEncaps));
                    test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null, out outEncaps));
                    test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null, out outEncaps));
                    test(batchOneway.ice_invoke("opOneway", OperationMode.Normal, null, out outEncaps));
                    batchOneway.ice_flushBatchRequests();

                    OutputStream outS = new OutputStream(communicator);
                    outS.startEncapsulation();
                    outS.writeString(testString);
                    outS.endEncapsulation();
                    inEncaps = outS.finished();

                    if (cl.ice_invoke("opString", OperationMode.Normal, inEncaps, out outEncaps))
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

                    if (cl.ice_invoke("opException", OperationMode.Normal, null, out outEncaps, ctx))
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
                        oneway.ice_invokeAsync("opOneway", OperationMode.Normal, null).Wait();
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
                    var result = cl.ice_invokeAsync("opString", OperationMode.Normal, inEncaps).Result;
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
                    var result = cl.ice_invokeAsync("opException", OperationMode.Normal, null).Result;
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
