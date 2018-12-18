// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.dispatcher;

import java.io.PrintWriter;

import test.Ice.dispatcher.Test.TestIntfPrx;
import test.Ice.dispatcher.Test.TestIntfControllerPrx;

import java.util.concurrent.CompletableFuture;
import com.zeroc.Ice.InvocationFuture;

public class AllTests
{
    private static class Callback
    {
        Callback()
        {
            _called = false;
            _exception = null;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
            if(_exception != null)
            {
                throw _exception;
            }
            _called = false;
        }

        public synchronized void exception(Throwable ex)
        {
            assert(!_called);
            _called = true;
            if(ex instanceof RuntimeException)
            {
                _exception = (RuntimeException)ex;
            }
            else
            {
                _exception = new RuntimeException(ex);
            }
            notify();
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
        private RuntimeException _exception;
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper, final Dispatcher dispatcher)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "test:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int mult = 1;
        if(!communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp").equals("tcp") ||
           helper.isAndroid())
        {
            mult = 4;
        }

        TestIntfPrx p = TestIntfPrx.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1, "tcp");
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrx.uncheckedCast(obj);

        out.print("testing dispatcher... ");
        out.flush();
        {
            p.op();

            {
                final Callback cb = new Callback();
                p.opAsync().whenCompleteAsync((result, ex) ->
                    {
                        if(ex != null)
                        {
                            cb.exception(ex);
                        }
                        else
                        {
                            test(dispatcher.isDispatcherThread());
                            cb.called();
                        }
                    }, dispatcher);
                cb.check();
            }

            {
                final Callback cb = new Callback();
                p.opAsync().whenCompleteAsync((result, ex) ->
                    {
                        if(ex != null)
                        {
                            cb.exception(ex);
                        }
                        else
                        {
                            test(dispatcher.isDispatcherThread());
                            cb.called();
                        }
                    }, p.ice_executor());
                cb.check();
            }

            {
                TestIntfPrx i = p.ice_adapterId("dummy");
                final Callback cb = new Callback();
                i.opAsync().whenCompleteAsync((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.NoEndpointException);
                            test(dispatcher.isDispatcherThread());
                            cb.called();
                        }
                        else
                        {
                            cb.exception(new RuntimeException());
                        }
                    }, dispatcher);
                cb.check();
            }

            {
                TestIntfPrx i = p.ice_adapterId("dummy");
                final Callback cb = new Callback();
                i.opAsync().whenCompleteAsync((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.NoEndpointException);
                            test(dispatcher.isDispatcherThread());
                            cb.called();
                        }
                        else
                        {
                            cb.exception(new RuntimeException());
                        }
                    }, p.ice_executor());
                cb.check();
            }

            {
                //
                // Expect InvocationTimeoutException.
                //
                TestIntfPrx to = p.ice_invocationTimeout(10);
                final Callback cb = new Callback();
                CompletableFuture<Void> r = to.sleepAsync(500 * mult);
                r.whenCompleteAsync((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.InvocationTimeoutException);
                            test(dispatcher.isDispatcherThread());
                            cb.called();
                        }
                        else
                        {
                            cb.exception(new RuntimeException());
                        }
                    }, dispatcher);
                com.zeroc.Ice.Util.getInvocationFuture(r).whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        test(dispatcher.isDispatcherThread());
                    }, dispatcher);
                cb.check();
            }

            {
                //
                // Expect InvocationTimeoutException.
                //
                TestIntfPrx to = p.ice_invocationTimeout(10);
                final Callback cb = new Callback();
                CompletableFuture<Void> r = to.sleepAsync(500 * mult);
                r.whenCompleteAsync((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.InvocationTimeoutException);
                            test(dispatcher.isDispatcherThread());
                            cb.called();
                        }
                        else
                        {
                            cb.exception(new RuntimeException());
                        }
                    }, dispatcher);
                com.zeroc.Ice.Util.getInvocationFuture(r).whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        test(dispatcher.isDispatcherThread());
                    }, p.ice_executor());
                cb.check();
            }

            // Hold adapter to make sure the invocations don't complete synchronously
            // Also disable collocation optimization on p
            //
            testController.holdAdapter();
            p = p.ice_collocationOptimized(false);
            byte[] seq = new byte[10 * 1024];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
            CompletableFuture<Void> r = null;
            while(true)
            {
                r = p.opWithPayloadAsync(seq);
                r.whenComplete((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException);
                        }
                        else
                        {
                            test(dispatcher.isDispatcherThread());
                        }
                    });
                InvocationFuture<Void> f = com.zeroc.Ice.Util.getInvocationFuture(r);
                f.whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        test(dispatcher.isDispatcherThread());
                    }, dispatcher);
                if(!f.sentSynchronously())
                {
                    break;
                }
            }
            testController.resumeAdapter();
            r.join();
        }
        out.println("ok");
        p.shutdown();
    }
}
