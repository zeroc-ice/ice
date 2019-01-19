//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interrupt;

import java.io.PrintWriter;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.TimeUnit;

import test.Ice.interrupt.Test.Callback_TestIntf_op;
import test.Ice.interrupt.Test.Callback_TestIntf_sleep;
import test.Ice.interrupt.Test.CannotInterruptException;
import test.Ice.interrupt.Test.TestIntfControllerPrx;
import test.Ice.interrupt.Test.TestIntfControllerPrxHelper;
import test.Ice.interrupt.Test.TestIntfPrx;
import test.Ice.interrupt.Test.TestIntfPrxHelper;
import Ice.AsyncResult;
import Ice.Callback_Communicator_flushBatchRequests;
import Ice.Callback_Connection_flushBatchRequests;
import Ice.Callback_Object_ice_flushBatchRequests;
import Ice.Callback_Object_ice_getConnection;
import Ice.Connection;
import Ice.LocalException;

public class AllTests
{
    private static class CallbackBase
    {
        CallbackBase()
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

    private static void
    test(boolean b)
    {
        if(!b)
        {
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

    public static void
    allTests(test.TestHelper helper)
        throws InterruptedException
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        String sref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        final TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrxHelper.uncheckedCast(obj);

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
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
                test(!mainThread.isInterrupted());
            }

            // Same test with the AMI API.
            try
            {
                Ice.AsyncResult r = p.begin_op();
                mainThread.interrupt();
                p.end_op(r);
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
                test(!mainThread.isInterrupted());
            }

            final CallbackBase cb = new CallbackBase();
            mainThread.interrupt();
            p.begin_op(new Callback_TestIntf_op()
            {
                @Override
                public void response()
                {
                    cb.called();
                }

                @Override
                public void exception(Ice.LocalException ex)
                {
                    test(false);
                }
            });
            test(Thread.interrupted());
            cb.check();

            p.begin_op(new Ice.Callback()
            {
                @Override
                public void completed(AsyncResult r)
                {
                    try
                    {
                        Thread.currentThread().interrupt();
                        p.end_op(r);
                        test(false);
                    }
                    catch(Ice.OperationInterruptedException ex)
                    {
                        // Expected
                        test(!Thread.currentThread().isInterrupted());
                    }
                }
            });

            ExecutorService executor = java.util.concurrent.Executors.newFixedThreadPool(1);
            executor.submit(new Runnable() {
                @Override
                public void run()
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
                }
            });

            try
            {
                test(!mainThread.isInterrupted());
                p.sleep(2000);
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }
            catch(test.Ice.interrupt.Test.InterruptedException e)
            {
                test(false);
            }

            executor.submit(new Runnable() {
                @Override
                public void run()
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
                }
            });

            try
            {
                test(!mainThread.isInterrupted());
                Ice.AsyncResult r = p.begin_sleep(2000);
                p.end_sleep(r);
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }
            catch(test.Ice.interrupt.Test.InterruptedException e)
            {
                test(false);
            }

            executor.submit(new Runnable() {
                @Override
                public void run()
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
                }
            });

            try
            {
                test(!mainThread.isInterrupted());
                p.opIdempotent();
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }
            catch(Ice.ConnectionLostException ex)
            {
                test(false);
            }

            // Test waitForCompleted is an interruption point.
            try
            {
                Ice.AsyncResult r = p.begin_op();
                try
                {
                    Thread.currentThread().interrupt();
                    r.waitForCompleted();
                    test(false);
                }
                catch(Ice.OperationInterruptedException ex)
                {
                    // Expected
                }

                // end_ should still work.
                p.end_op(r);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                test(false);
            }

            // Test waitForSent is an interruption point.
            try
            {
                Ice.AsyncResult r = p.begin_op();
                try
                {
                    Thread.currentThread().interrupt();
                    r.waitForSent();
                    test(false);
                }
                catch(Ice.OperationInterruptedException ex)
                {
                    // Expected
                }

                // end_ should still work.
                p.end_op(r);
            }
            catch(Ice.OperationInterruptedException ex)
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
                executor.submit(new Runnable() {
                    @Override
                    public void run()
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
                    }
                });

                Ice.AsyncResult r = null;

                // The sequence needs to be large enough to fill the write/recv buffers
                int bufSize = 2000000;
                byte[] seq = new byte[bufSize];
                r = p.begin_opWithPayload(seq);
                try
                {
                    r.waitForSent();
                    test(false);
                }
                catch(Ice.OperationInterruptedException ex)
                {
                    // Expected
                }
                //
                // Resume the adapter.
                //
                testController.resumeAdapter();
                r.waitForSent();
                r.waitForCompleted();
                p.end_opWithPayload(r);
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

        if(p.ice_getCachedConnection() != null)
        {
            out.print("testing getConnection interrupt... ");
            out.flush();
            {
                final Thread mainThread = Thread.currentThread();
                p.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

                AsyncResult r = p.begin_ice_getConnection();
                mainThread.interrupt();
                try
                {
                    p.end_ice_getConnection(r);
                    test(false);
                }
                catch(Ice.OperationInterruptedException ex)
                {
                    // Expected
                }

                p.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

                final CallbackBase cb = new CallbackBase();
                mainThread.interrupt();
                p.begin_ice_getConnection(new Callback_Object_ice_getConnection()
                {
                    @Override
                    public void exception(LocalException ex)
                    {
                        test(false);
                    }

                    @Override
                    public void response(Connection con)
                    {
                        cb.called();
                    }
                });
                test(Thread.interrupted());
                cb.check();
            }
            out.println("ok");
        }

        out.print("testing batch proxy flush interrupt... ");
        out.flush();
        {
            final TestIntfPrx p2 = TestIntfPrxHelper.uncheckedCast(p.ice_batchOneway());
            final Thread mainThread = Thread.currentThread();

            p2.op();
            p2.op();
            p2.op();

            AsyncResult r = p2.begin_ice_flushBatchRequests();
            mainThread.interrupt();
            try
            {
                p2.end_ice_flushBatchRequests(r);
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }

            p2.op();
            p2.op();
            p2.op();

            final CallbackBase cb = new CallbackBase();
            mainThread.interrupt();
            p2.begin_ice_flushBatchRequests(new Callback_Object_ice_flushBatchRequests()
            {
                @Override
                public void sent(boolean sentSynchronously)
                {
                    cb.called();
                }

                @Override
                public void exception(LocalException ex)
                {
                    test(false);
                }
            });
            test(Thread.interrupted());
            cb.check();
        }
        out.println("ok");

        if(p.ice_getCachedConnection() != null)
        {
            out.print("testing batch connection flush interrupt... ");
            out.flush();
            {
                final TestIntfPrx p2 = TestIntfPrxHelper.uncheckedCast(p.ice_batchOneway());
                final Thread mainThread = Thread.currentThread();

                p2.op();
                p2.op();
                p2.op();

                AsyncResult r = p2.ice_getConnection().begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy);
                mainThread.interrupt();
                try
                {
                    p2.ice_getCachedConnection().end_flushBatchRequests(r);
                    test(false);
                }
                catch(Ice.OperationInterruptedException ex)
                {
                    // Expected
                }

                p2.op();
                p2.op();
                p2.op();

                final CallbackBase cb = new CallbackBase();
                Ice.Connection con = p2.ice_getConnection();
                mainThread.interrupt();
                con.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy,
                                             new Callback_Connection_flushBatchRequests()
                {
                    @Override
                    public void sent(boolean sentSynchronously)
                    {
                        cb.called();
                    }

                    @Override
                    public void exception(LocalException ex)
                    {
                        test(false);
                    }
                });
                test(Thread.interrupted());
                cb.check();
            }
            out.println("ok");
        }

        out.print("testing batch communicator flush interrupt... ");
        out.flush();
        {
            final TestIntfPrx p2 = TestIntfPrxHelper.uncheckedCast(p.ice_batchOneway());
            final Thread mainThread = Thread.currentThread();

            p2.op();
            p2.op();
            p2.op();

            AsyncResult r = communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy);
            mainThread.interrupt();
            try
            {
                communicator.end_flushBatchRequests(r);
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }

            p2.op();
            p2.op();
            p2.op();

            final CallbackBase cb = new CallbackBase();
            mainThread.interrupt();
            communicator.begin_flushBatchRequests(Ice.CompressBatch.BasedOnProxy,
                                                  new Callback_Communicator_flushBatchRequests()
            {
                @Override
                public void sent(boolean sentSynchronously)
                {
                    cb.called();
                }

                @Override
                public void exception(LocalException ex)
                {
                    test(false);
                }
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
            Ice.Properties properties = communicator.getProperties()._clone();
            Ice.Communicator ic = helper.initialize(properties);

            Thread.currentThread().interrupt();
            try
            {
                ic.destroy();
                failIfNotInterrupted();
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }
            ic.destroy();

            ExecutorService executor = java.util.concurrent.Executors.newFixedThreadPool(2);

            ic = helper.initialize(properties);
            Ice.ObjectPrx o = ic.stringToProxy(p.toString());

            final Thread[] thread = new Thread[1];

            final CallbackBase cb = new CallbackBase();
            final TestIntfPrx p2 = TestIntfPrxHelper.checkedCast(o);
            final CountDownLatch waitSignal = new CountDownLatch(1);
            p2.begin_op(new Callback_TestIntf_op()
            {
                @Override
                public void response()
                {
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
                }

                @Override
                public void exception(Ice.LocalException ex)
                {
                    test(false);

                }
            });

            executor.submit(new Runnable() {
                @Override
                public void run()
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
                }
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
            final CallbackBase cb = new CallbackBase();
            p.begin_sleep(2000, new Callback_TestIntf_sleep()
            {
                @Override
                public void response()
                {
                    test(false);
                }

                @Override
                public void exception(Ice.LocalException ex)
                {
                    test(false);
                }

                @Override
                public void exception(Ice.UserException ex)
                {
                    test(ex instanceof test.Ice.interrupt.Test.InterruptedException);
                    cb.called();
                }
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
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("ClientTestAdapter.Endpoints", "tcp -h *");
            try(Ice.Communicator ic = helper.initialize(initData))
            {
                final Ice.ObjectAdapter adapter = ic.createObjectAdapter("ClientTestAdapter");
                adapter.activate();

                try
                {
                    mainThread.interrupt();
                    adapter.waitForHold();
                    test(false);
                }
                catch(Ice.OperationInterruptedException e)
                {
                    // Expected.
                }

                try
                {
                    mainThread.interrupt();
                    adapter.waitForDeactivate();
                    test(false);
                }
                catch(Ice.OperationInterruptedException e)
                {
                    // Expected.
                }

                try
                {
                    mainThread.interrupt();
                    ic.waitForShutdown();
                    test(false);
                }
                catch(Ice.OperationInterruptedException e)
                {
                    // Expected.
                }

                Runnable interruptMainThread = new Runnable() {
                        @Override
                        public void run()
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
                        }
                    };

                executor.execute(interruptMainThread);
                try
                {
                    adapter.waitForHold();
                    test(false);
                }
                catch(Ice.OperationInterruptedException e)
                {
                    // Expected.
                }

                executor.execute(interruptMainThread);
                try
                {
                    adapter.waitForDeactivate();
                    test(false);
                }
                catch(Ice.OperationInterruptedException e)
                {
                    // Expected.
                }

                executor.execute(interruptMainThread);
                try
                {
                    ic.waitForShutdown();
                    test(false);
                }
                catch(Ice.OperationInterruptedException e)
                {
                    // Expected.
                }
            }

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
