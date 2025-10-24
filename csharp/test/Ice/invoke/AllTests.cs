// Copyright (c) ZeroC, Inc.

namespace Ice.invoke;

public class AllTests : global::Test.AllTests
{
    private const string testString = "This is a test string";

    public static Test.MyClassPrx allTests(global::Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Ice.ObjectPrx baseProxy = communicator.stringToProxy("test:" + helper.getTestEndpoint(0));
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        Test.MyClassPrx oneway = Test.MyClassPrxHelper.uncheckedCast(cl.ice_oneway());
        Test.MyClassPrx batchOneway = Test.MyClassPrxHelper.uncheckedCast(cl.ice_batchOneway());

        TextWriter output = helper.getWriter();
        output.Write("testing ice_invoke... ");
        output.Flush();
        {
            byte[] inEncaps;
            if (!oneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out _))
            {
                test(false);
            }

            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out _));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out _));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out _));
            test(batchOneway.ice_invoke("opOneway", Ice.OperationMode.Normal, null, out _));
            batchOneway.ice_flushBatchRequests();

            var outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            inEncaps = outS.finished();
            if (cl.ice_invoke("opString", Ice.OperationMode.Normal, inEncaps, out byte[] outEncaps))
            {
                var inS = new Ice.InputStream(communicator, outEncaps);
                inS.startEncapsulation();
                string s = inS.readString();
                test(s == testString);
                s = inS.readString();
                inS.endEncapsulation();
                test(s == testString);
            }
            else
            {
                test(false);
            }
        }

        for (int i = 0; i < 2; ++i)
        {
            Dictionary<string, string> ctx = null;
            if (i == 1)
            {
                ctx = new Dictionary<string, string>
                {
                    ["raise"] = ""
                };
            }

            if (cl.ice_invoke("opException", Ice.OperationMode.Normal, null, out byte[] outEncaps, ctx))
            {
                test(false);
            }
            else
            {
                var inS = new Ice.InputStream(communicator, outEncaps);
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
                oneway.ice_invokeAsync("opOneway", Ice.OperationMode.Normal, null).Wait();
            }
            catch (Exception)
            {
                test(false);
            }

            var outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeString(testString);
            outS.endEncapsulation();
            byte[] inEncaps = outS.finished();

            // begin_ice_invoke with no callback
            Object_Ice_invokeResult result = cl.ice_invokeAsync("opString", Ice.OperationMode.Normal, inEncaps).Result;
            if (result.returnValue)
            {
                var inS = new Ice.InputStream(communicator, result.outEncaps);
                inS.startEncapsulation();
                string s = inS.readString();
                test(s == testString);
                s = inS.readString();
                inS.endEncapsulation();
                test(s == testString);
            }
            else
            {
                test(false);
            }
        }

        {
            Object_Ice_invokeResult result = cl.ice_invokeAsync("opException", Ice.OperationMode.Normal, null).Result;
            if (result.returnValue)
            {
                test(false);
            }
            else
            {
                var inS = new Ice.InputStream(communicator, result.outEncaps);
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
