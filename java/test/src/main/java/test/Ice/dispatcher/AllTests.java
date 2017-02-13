// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

            _called = false;
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.Util.Application app, final Dispatcher dispatcher)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String sref = "test:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int mult = 1;
        if(!communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp").equals("tcp"))
        {
            mult = 4;
        }

        TestIntfPrx p = TestIntfPrx.uncheckedCast(obj);

        sref = "testController:" + app.getTestEndpoint(1, "tcp");
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
                            ex.printStackTrace();
                            test(false);
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
                            test(false);
                        }
                    }, dispatcher);
                cb.check();
            }

            {
                //
                // Expect InvocationTimeoutException.
                //
                TestIntfPrx to = p.ice_invocationTimeout(250);
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
                            test(false);
                        }
                    }, dispatcher);
                com.zeroc.Ice.Util.getInvocationFuture(r).whenSentAsync((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        test(dispatcher.isDispatcherThread());
                    }, dispatcher);
                cb.check();
            }

            testController.holdAdapter();
            byte[] seq = new byte[10 * 1024];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
            CompletableFuture<Void> r = null;
            while(true)
            {
                r = p.opWithPayloadAsync(seq);
                r.whenCompleteAsync((result, ex) ->
                    {
                        if(ex != null)
                        {
                            test(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException);
                        }
                        else
                        {
                            test(dispatcher.isDispatcherThread());
                        }
                    }, dispatcher);
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
