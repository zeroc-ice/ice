// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
import Ice.LocalException;
import Ice.OperationInterruptedException;
import Ice.UserException;

public class AllTests
{
    private static class CallbackBase
    {
        CallbackBase()
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
    
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(test.Util.Application app, Ice.Communicator communicator, PrintWriter out)
        throws InterruptedException
    {
        String sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:tcp -p 12011";
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
                p.op();
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }

            final CallbackBase cb = new CallbackBase();
            mainThread.interrupt();
            p.begin_sleep(500, new Callback_TestIntf_sleep()
            {
                
                @Override
                public void response()
                {
                    test(false);
                }
                
                @Override
                public void exception(LocalException ex)
                {
                    test(ex instanceof OperationInterruptedException);
                    cb.called();
                }

                @Override
                public void exception(UserException ex)
                {
                    test(false);
                }
            });
            cb.check();

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
            

            try
            {
                Ice.AsyncResult r = p.begin_op();
                Thread.currentThread().interrupt();
                r.waitForCompleted();
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }

            try
            {
                Ice.AsyncResult r = p.begin_op();
                Thread.currentThread().interrupt();
                p.end_op(r);
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }

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

            testController.holdAdapter();
            Ice.AsyncResult r = null;
            try
            {
                // The sequence needs to be large enough to fill the write/recv buffers
                byte[] seq = new byte[20000000];
                r = p.begin_opWithPayload(seq);
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
            
            //
            // The executor is all done.
            //
            executor.shutdown();
            executor.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
        }
        out.println("ok");

        out.print("testing Communicator.destroy interrupt... ");
        out.flush();
        {
            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            Ice.Communicator ic = app.initialize(initData);
            
            Thread.currentThread().interrupt();            
            try
            {
                ic.destroy();
                test(false);
            }
            catch(Ice.OperationInterruptedException ex)
            {
                // Expected
            }

            ExecutorService executor = java.util.concurrent.Executors.newFixedThreadPool(2);
            
            ic = app.initialize(initData);
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
                public void exception(LocalException ex)
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
            
            ic.destroy();

            cb.check();
            
            executor.shutdown();
            executor.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
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
                public void exception(LocalException ex)
                {        
                    test(false);
                }

                @Override
                public void exception(UserException ex)
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
            initData.properties.setProperty("ClientTestAdapter.Endpoints", "default -p 12030");
            Ice.Communicator ic = app.initialize(initData);
            final Ice.ObjectAdapter adapter = ic.createObjectAdapter("ClientTestAdapter");
            adapter.activate();
            
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

            ic.destroy();
            
            executor.shutdown();
            executor.awaitTermination(Long.MAX_VALUE, TimeUnit.NANOSECONDS);
        }
        out.println("ok");

        p.shutdown();
    }
}
