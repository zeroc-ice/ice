// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interrupt;

import java.io.PrintWriter;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.TimeUnit;

import com.zeroc.Ice.Connection;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.Util;
import com.zeroc.Ice.InvocationFuture;
import com.zeroc.Ice.CompressBatch;

import test.Ice.interrupt.Test.CannotInterruptException;
import test.Ice.interrupt.Test.TestIntfControllerPrx;
import test.Ice.interrupt.Test.TestIntfPrx;

public class AllTests
{
    private static class Callback
    {
        Callback()
        {
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

        private boolean _called = false;
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            new Throwable().printStackTrace();
            throw new RuntimeException();
        }
    }

    private static void failIfNotInterrupted()
    {
        if(Thread.currentThread().isInterrupted())
        {
            Thread.interrupted();
        }
        else
        {
            test(false);
        }
    }

    public static void allTests(test.Util.Application app)
        throws InterruptedException
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();
        String sref = "test:" + app.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        final TestIntfPrx p = TestIntfPrx.uncheckedCast(obj);

        sref = "testController:" + app.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrx.uncheckedCast(obj);

        out.print("testing client interrupt... ");
        out.flush();
        {
            final Thread mainThread = Thread.currentThread();
            mainThread.interrupt();
            try
            {
                // Synchronous invocations are interruption points. If the
                // interrupt flag is set at the start of the operation
                // OperationInterruptedException must be thrown.
                p.op();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException ex)
            {
                // Expected
                test(!mainThread.isInterrupted());
            }

            // Same test with the AMI API.
            try
            {
                //
                // We call sleep here to add a small delay. Otherwise there's a chance that the
                // future will be completed before get() is called, in which case the interrupt
                // will be ignored.
                //
                CompletableFuture<Void> r = p.sleepAsync(250);
                mainThread.interrupt();
                r.get();
                test(false);
            }
            catch(java.lang.InterruptedException ex)
            {
                // Expected
                test(!mainThread.isInterrupted());
            }
            catch(java.util.concurrent.ExecutionException ex)
            {
                test(false);
            }

            final Callback cb = new Callback();
            mainThread.interrupt();
            p.opAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            test(Thread.interrupted());
            cb.check();

            ExecutorService executor = java.util.concurrent.Executors.newFixedThreadPool(1);
            executor.submit(() ->
                            {
                                try
                                {
                                    Thread.sleep(500);
                                }
                                catch(InterruptedException e)
                                {
                                    test(false);
                                }
                                mainThread.interrupt();
                            });
            try
            {
                test(!mainThread.isInterrupted());
                p.sleep(2000);
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException ex)
            {
                // Expected
            }
            catch(test.Ice.interrupt.Test.InterruptedException e)
            {
                test(false);
            }

            executor.submit(() ->
                            {
                                try
                                {
                                    Thread.sleep(500);
                                }
                                catch(InterruptedException e)
                                {
                                    test(false);
                                }
                                mainThread.interrupt();
                            });
            try
            {
                test(!mainThread.isInterrupted());
                p.sleepAsync(2000).get();
                test(false);
            }
            catch(ExecutionException ex)
            {
                test(false);
            }
            catch(java.lang.InterruptedException ex)
            {
                // Expected
            }

            executor.submit(() ->
                            {
                                try
                                {
                                    Thread.sleep(500);
                                }
                                catch(InterruptedException e)
                                {
                                    test(false);
                                }
                                mainThread.interrupt();
                            });
            try
            {
                test(!mainThread.isInterrupted());
                p.opIdempotent();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException ex)
            {
                // Expected
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
                test(false);
            }

            // Test waitForSent is an interruption point.
            try
            {
                CompletableFuture<Void> r = p.opAsync();
                try
                {
                    Thread.currentThread().interrupt();
                    InvocationFuture<Void> f = Util.getInvocationFuture(r);
                    f.waitForSent();
                    test(false);
                }
                catch(com.zeroc.Ice.OperationInterruptedException ex)
                {
                    // Expected
                }

                // join should still work.
                r.join();
            }
            catch(CompletionException ex)
            {
                test(false);
            }

            // This section of the test doesn't run when collocated.
            if(p.ice_getConnection() != null)
            {

                testController.holdAdapter();

                //
                // Test interrupt of waitForSent. Here hold the adapter and send a large payload. The
                // thread is interrupted in 500ms which should result in a operation interrupted exception.
                //
                executor.submit(() ->
                                {
                                    try
                                    {
                                        Thread.sleep(500);
                                    }
                                    catch(InterruptedException e)
                                    {
                                        test(false);
                                    }
                                    mainThread.interrupt();
                                });

                CompletableFuture<Void> r = null;
                InvocationFuture<Void> f = null;

                // The sequence needs to be large enough to fill the write/recv buffers
                int bufSize = 2000000;
                byte[] seq = new byte[bufSize];
                r = p.opWithPayloadAsync(seq);
                try
                {
                    f = Util.getInvocationFuture(r);
                    f.waitForSent();
                    test(false);
                }
                catch(com.zeroc.Ice.OperationInterruptedException ex)
                {
                    // Expected
                }
                //
                // Resume the adapter.
                //
                testController.resumeAdapter();
                f.waitForSent();
                f.waitForCompleted();
                r.join();
            }

            //
            // The executor is all done.
            //
            executor.shutdown();
            while(!executor.isTerminated())
            {
                executor.awaitTermination(1000, TimeUnit.SECONDS);
            }
        }
        out.println("ok");

        if(p.ice_getConnection() != null)
        {
            out.print("testing getConnection interrupt... ");
            out.flush();
            {
                final Thread mainThread = Thread.currentThread();

                p.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

                CompletableFuture<com.zeroc.Ice.Connection> r = p.ice_getConnectionAsync();
                mainThread.interrupt();
                try
                {
                    r.get();
                    //
                    // get() won't raise InterruptedException if connection establishment has already completed.
                    //
                    //test(false);
                    mainThread.interrupted();
                }
                catch(ExecutionException ex)
                {
                    test(false);
                }
                catch(java.lang.InterruptedException ex)
                {
                    // Expected
                }

                p.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

                final Callback cb = new Callback();
                mainThread.interrupt();
                p.ice_getConnectionAsync().whenComplete((result, ex) ->
                    {
                        test(ex == null);
                        cb.called();
                    });
                test(Thread.interrupted());
                cb.check();
            }
            out.println("ok");
        }

        out.print("testing batch proxy flush interrupt... ");
        out.flush();
        {
            final TestIntfPrx p2 = p.ice_batchOneway();
            final Thread mainThread = Thread.currentThread();
            CompletableFuture<Void> r;

            p2.op();
            p2.op();
            p2.op();

            r = p2.ice_flushBatchRequestsAsync();
            mainThread.interrupt();
            try
            {
                r.get();
                //
                // get() won't raise InterruptedException if the operation has already completed.
                //
                //test(false);
                mainThread.interrupted();
            }
            catch(ExecutionException ex)
            {
                test(false);
            }
            catch(java.lang.InterruptedException ex)
            {
                // Expected
            }

            p2.op();
            p2.op();
            p2.op();

            final Callback cb = new Callback();
            mainThread.interrupt();
            r = p2.ice_flushBatchRequestsAsync();
            r.whenComplete((result, ex) -> test(ex == null));
            Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            test(Thread.interrupted());
            cb.check();
        }
        out.println("ok");

        if(p.ice_getConnection() != null)
        {
            out.print("testing batch connection flush interrupt... ");
            out.flush();
            {
                final TestIntfPrx p2 = p.ice_batchOneway();
                final Thread mainThread = Thread.currentThread();
                CompletableFuture<Void> r;

                p2.op();
                p2.op();
                p2.op();

                r = p2.ice_getConnection().flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                mainThread.interrupt();
                try
                {
                    r.get();
                    //
                    // get() won't raise InterruptedException if the operation has already completed.
                    //
                    //test(false);
                    mainThread.interrupted();
                }
                catch(ExecutionException ex)
                {
                    test(false);
                }
                catch(java.lang.InterruptedException ex)
                {
                    // Expected
                }

                p2.op();
                p2.op();
                p2.op();

                final Callback cb = new Callback();
                com.zeroc.Ice.Connection con = p2.ice_getConnection();
                mainThread.interrupt();
                r = con.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                r.whenComplete((result, ex) -> test(ex == null));
                Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        test(ex == null);
                        cb.called();
                    });
                test(Thread.interrupted());
                cb.check();
            }
            out.println("ok");
        }

        out.print("testing batch communicator flush interrupt... ");
        out.flush();
        {
            final TestIntfPrx p2 = p.ice_batchOneway();
            final Thread mainThread = Thread.currentThread();
            CompletableFuture<Void> r;

            p2.op();
            p2.op();
            p2.op();

            r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
            mainThread.interrupt();
            try
            {
                r.get();
                //
                // get() won't raise InterruptedException if the operation has already completed.
                //
                //test(false);
                mainThread.interrupted();
            }
            catch(ExecutionException ex)
            {
                test(false);
            }
            catch(java.lang.InterruptedException ex)
            {
                // Expected
            }

            p2.op();
            p2.op();
            p2.op();

            final Callback cb = new Callback();
            mainThread.interrupt();
            r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
            r.whenComplete((result, ex) -> test(ex == null));
            Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            test(Thread.interrupted());
            cb.check();
        }
        out.println("ok");

        out.print("testing Communicator.destroy interrupt... ");
        out.flush();
        if(p.ice_getConnection() != null)
        {
            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            com.zeroc.Ice.InitializationData initData = app.createInitializationData();
            initData.properties = communicator.getProperties()._clone();
            com.zeroc.Ice.Communicator ic = app.initialize(initData);

            Thread.currentThread().interrupt();
            try
            {
                ic.destroy();
                failIfNotInterrupted();
            }
            catch(com.zeroc.Ice.OperationInterruptedException ex)
            {
                // Expected
            }
            ic.destroy();

            ExecutorService executor = java.util.concurrent.Executors.newFixedThreadPool(2);

            ic = app.initialize(initData);
            com.zeroc.Ice.ObjectPrx o = ic.stringToProxy(p.toString());

            final Thread[] thread = new Thread[1];

            final Callback cb = new Callback();
            final TestIntfPrx p2 = TestIntfPrx.checkedCast(o);
            final CountDownLatch waitSignal = new CountDownLatch(1);
            executor.submit(() ->
                            {
                                try
                                {
                                    waitSignal.await();
                                }
                                catch(InterruptedException e)
                                {
                                    test(false);
                                }
                                thread[0].interrupt();
                            });
            //
            // The whenComplete() action may be executed in the current thread (if the future is
            // already completed). We have to submit the runnable to the executor *before*
            // calling whenComplete() because this thread can block in sleep().
            //
            p2.opAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    try
                    {
                        Thread.sleep(250);
                    }
                    catch(InterruptedException e1)
                    {
                        test(false);
                    }
                    thread[0] = Thread.currentThread();
                    waitSignal.countDown();
                    try
                    {
                        Thread.sleep(10000);
                        test(false);
                    }
                    catch(InterruptedException e)
                    {
                        // Expected
                    }
                    cb.called();
                });

            try
            {
                waitSignal.await();
            }
            catch(InterruptedException e)
            {
                test(false);
            }
            ic.destroy();

            cb.check();

            executor.shutdown();
            while(!executor.isTerminated())
            {
                executor.awaitTermination(1000, TimeUnit.SECONDS);
            }
        }
        out.println("ok");

        out.print("testing server interrupt... ");
        out.flush();
        {
            final Callback cb = new Callback();
            p.sleepAsync(2000).whenComplete((result, ex) ->
                {
                    test(ex != null && ex instanceof test.Ice.interrupt.Test.InterruptedException);
                    cb.called();
                });
            try
            {
                Thread.sleep(250);
            }
            catch(InterruptedException e)
            {
                test(false);
            }
            try
            {
                testController.interrupt();
            }
            catch(CannotInterruptException e)
            {
                test(false);
            }
            cb.check();
        }
        out.println("ok");

        out.print("testing wait methods... ");
        out.flush();
        {
            final Thread mainThread = Thread.currentThread();
            ExecutorService executor = java.util.concurrent.Executors.newFixedThreadPool(1);
            com.zeroc.Ice.InitializationData initData = app.createInitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("ClientTestAdapter.Endpoints", "tcp -h *");
            com.zeroc.Ice.Communicator ic = app.initialize(initData);
            final com.zeroc.Ice.ObjectAdapter adapter = ic.createObjectAdapter("ClientTestAdapter");
            adapter.activate();

            try
            {
                mainThread.interrupt();
                adapter.waitForHold();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException e)
            {
                // Expected.
            }

            try
            {
                mainThread.interrupt();
                adapter.waitForDeactivate();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException e)
            {
                // Expected.
            }

            try
            {
                mainThread.interrupt();
                ic.waitForShutdown();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException e)
            {
                // Expected.
            }

            Runnable interruptMainThread = () ->
                {
                    try
                    {
                        Thread.sleep(250);
                    }
                    catch(InterruptedException e)
                    {
                        test(false);
                    }
                    mainThread.interrupt();
                };

            executor.execute(interruptMainThread);
            try
            {
                adapter.waitForHold();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException e)
            {
                // Expected.
            }

            executor.execute(interruptMainThread);
            try
            {
                adapter.waitForDeactivate();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException e)
            {
                // Expected.
            }

            executor.execute(interruptMainThread);
            try
            {
                ic.waitForShutdown();
                test(false);
            }
            catch(com.zeroc.Ice.OperationInterruptedException e)
            {
                // Expected.
            }

            ic.destroy();

            executor.shutdown();
            while(!executor.isTerminated())
            {
                executor.awaitTermination(1000, TimeUnit.SECONDS);
            }
        }
        out.println("ok");

        p.shutdown();
    }
}
