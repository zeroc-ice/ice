// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.dispatcher;

import java.io.PrintWriter;

import test.Ice.dispatcher.Test.TestIntfPrx;
import test.Ice.dispatcher.Test.TestIntfPrxHelper;
import test.Ice.dispatcher.Test.TestIntfControllerPrx;
import test.Ice.dispatcher.Test.TestIntfControllerPrxHelper;
import test.Ice.dispatcher.Test.Callback_TestIntf_op;
import test.Ice.dispatcher.Test.Callback_TestIntf_opWithPayload;
import test.Ice.dispatcher.Test.Callback_TestIntf_sleep;

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

        public synchronized void exception(Exception ex)
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

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper, final Dispatcher dispatcher)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        int mult = 1;
        if(!communicator.getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp").equals("tcp") ||
           helper.isAndroid())
        {
            mult = 4;
        }

        TestIntfPrx p = TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1, "tcp");
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrxHelper.uncheckedCast(obj);

        out.print("testing dispatcher... ");
        out.flush();
        {
            p.op();

            final Callback cb1 = new Callback();
            p.begin_op(new Callback_TestIntf_op() {
                @Override
                public void
                response()
                {
                    test(dispatcher.isDispatcherThread());
                    cb1.called();
                }

                @Override
                public void
                exception(Ice.LocalException ex)
                {
                    cb1.exception(ex);
                }
            });
            cb1.check();

            final Callback cb2 = new Callback();
            TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
            i.begin_op(new Callback_TestIntf_op() {
                @Override
                public void
                response()
                {
                    cb2.exception(new RuntimeException());
                }

                @Override
                public void
                exception(Ice.LocalException ex)
                {
                    test(ex instanceof Ice.NoEndpointException);
                    test(dispatcher.isDispatcherThread());
                    cb2.called();
                }
            });
            cb2.check();

            {
                //
                // Expect InvocationTimeoutException.
                //
                TestIntfPrx to = TestIntfPrxHelper.uncheckedCast(p.ice_invocationTimeout(10));
                final Callback cb3 = new Callback();
                to.begin_sleep(500 * mult, new Callback_TestIntf_sleep()
                {
                    @Override
                    public void
                    response()
                    {
                        cb3.exception(new RuntimeException());
                    }

                    @Override
                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(ex instanceof Ice.InvocationTimeoutException);
                        test(dispatcher.isDispatcherThread());
                        cb3.called();
                    }

                    @Override
                    public void
                    sent(boolean sentSynchronously)
                    {
                        test(sentSynchronously || dispatcher.isDispatcherThread());
                    }
                });
                cb3.check();
            }

            testController.holdAdapter();
            Callback_TestIntf_opWithPayload callback = new Callback_TestIntf_opWithPayload()
                {
                    @Override
                    public void
                    response()
                    {
                        test(dispatcher.isDispatcherThread());
                    }

                    @Override
                    public void
                    exception(Ice.LocalException ex)
                    {
                        test(ex instanceof Ice.CommunicatorDestroyedException);
                    }

                    @Override
                    public void
                    sent(boolean sentSynchronously)
                    {
                        test(sentSynchronously || dispatcher.isDispatcherThread());
                    }
                };

            byte[] seq = new byte[10 * 1024];
            new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
            Ice.AsyncResult r;
            while((r = p.begin_opWithPayload(seq, callback)).sentSynchronously());
            testController.resumeAdapter();
            r.waitForCompleted();
        }
        out.println("ok");

        p.shutdown();
    }
}
