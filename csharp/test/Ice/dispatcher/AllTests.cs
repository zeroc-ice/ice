//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

public class AllTests : Test.AllTests
{
    public class Progress : IProgress<bool>
    {
        public Progress()
        {
        }

        public bool getResult()
        {
            return _sentSynchronously;
        }

        public void Report(bool sentSynchronously)
        {
            _sentSynchronously = sentSynchronously;
        }

        bool _sentSynchronously = false;
    }

    public static void allTests(TestHelper helper)
    {
        var output = helper.getWriter();
        Ice.Communicator communicator = helper.communicator();
        string sref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfPrx p = Test.TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfControllerPrx testController = Test.TestIntfControllerPrxHelper.uncheckedCast(obj);

        output.Write("testing dispatcher with async/await... ");
        output.Flush();
        {
            TaskCompletionSource<object> t = new TaskCompletionSource<object>();
            p.opAsync().ContinueWith(async previous => // Execute the code below from the Ice client thread pool
            {
                try
                {
                    await p.opAsync();
                    test(Dispatcher.isDispatcherThread());

                    try
                    {
                        TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
                        await i.opAsync();
                        test(false);
                    }
                    catch(Exception)
                    {
                        test(Dispatcher.isDispatcherThread());
                    }

                    Test.TestIntfPrx to = Test.TestIntfPrxHelper.uncheckedCast(p.ice_invocationTimeout(10));
                    try
                    {
                        await to.sleepAsync(500);
                        test(false);
                    }
                    catch(Ice.InvocationTimeoutException)
                    {
                        test(Dispatcher.isDispatcherThread());
                    }
                    t.SetResult(null);
                }
                catch(Exception ex)
                {
                    t.SetException(ex);
                }
            }, p.ice_scheduler());

            t.Task.Wait();
        }
        output.WriteLine("ok");

        p.shutdown();
    }
}
