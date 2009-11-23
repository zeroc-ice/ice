// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.Callback_MyClass_opVoid;
import test.Ice.operations.Test.MyClass;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.StateChangerPrx;
import test.Ice.operations.Test.StateChangerPrxHelper;

class OnewaysNewAMI
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

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

    private static class NoEndpointCallbackUnsafe extends Ice.AsyncCallback
    {
        @Override
        public void completed(Ice.AsyncResult r)
        {
            try
            {
                MyClassPrx p = MyClassPrxHelper.uncheckedCast(r.getProxy());
                p.end_opVoid(r);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
                callback.called();
            }
        }

        @Override
        public void sent(Ice.AsyncResult r)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class NoEndpointCallbackSafe extends Callback_MyClass_opVoid
    {
        @Override
        public void response()
        {
            test(false);
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.NoEndpointException);
            callback.called();
        }

        @Override
        public void sent()
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    enum ThrowType { LocalException, OtherException };

    private static void throwEx(ThrowType t)
    {
        switch(t)
        {
        case LocalException:
            throw new Ice.ObjectNotExistException();

        case OtherException:
            throw new NullPointerException();
        }
    }

    private static class ThrowerUnsafeEx extends Ice.AsyncCallback
    {
        ThrowerUnsafeEx(ThrowType t)
        {
            _t = t;
        }

        @Override
        public void completed(Ice.AsyncResult r)
        {
            MyClassPrx p = (MyClassPrx)r.getProxy();
            try
            {
                p.end_opVoid(r);
                test(false);
            }
            catch(Ice.LocalException ex)
            {
                callback.called();
                throwEx(_t);
            }
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private ThrowType _t;
    }

    private static class ThrowerSafeEx extends Callback_MyClass_opVoid
    {
        ThrowerSafeEx(ThrowType t)
        {
            _t = t;
        }

        @Override
        public void response()
        {
            test(false);
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            callback.called();
            throwEx(_t);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private ThrowType _t;
    }

    private static class PingCallback extends Ice.AsyncCallback
    {
        @Override
        public void completed(Ice.AsyncResult r)
        {
            test(false);
        }
    }

    private static class SentCounter extends Ice.AsyncCallback
    {
        SentCounter()
        {
            _queuedCount = 0;
        }

        @Override
        public void completed(Ice.AsyncResult r)
        {
            test(false);
        }

        @Override
        public void sent(Ice.AsyncResult r)
        {
            synchronized(this)
            {
                ++_queuedCount;
            }
        }

        synchronized int queuedCount()
        {
            return _queuedCount;
        }

        synchronized void check(int size)
        {
            while(_queuedCount != size)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
        }

        private int _queuedCount;
    }

    static void
    onewaysNewAMI(test.Util.Application app, MyClassPrx proxy)
    {
        Ice.Communicator communicator = app.communicator();
        MyClassPrx p = (MyClassPrx)proxy.ice_oneway();

        {
            //
            // Check that a call to a void operation raises NoEndpointException
            // in the end_ method instead of at the point of call.
            //
            MyClassPrx indirect = MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy").ice_oneway());
            Ice.AsyncResult r;

            r = indirect.begin_opVoid();
            try
            {
                indirect.end_opVoid(r);
                test(false);
            }
            catch(Ice.NoEndpointException ex)
            {
            }

            //
            // Check that a second call to the end_ method throws IllegalArgumentException.
            //
            try
            {
                indirect.end_opVoid(r);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }

            //
            // Check that a call to a void operation raises NoEndpointException in the callback.
            // Use type-unsafe and type-safe variations of the callback.
            // Also test that the sent callback is not called in this case.
            //
            NoEndpointCallbackUnsafe cb1 = new NoEndpointCallbackUnsafe();
            indirect.begin_opVoid(cb1);
            cb1.check();

            NoEndpointCallbackSafe cb2 = new NoEndpointCallbackSafe();
            indirect.begin_opVoid(cb2);
            cb2.check();

            //
            // Check that calling the end_ method with a different proxy or for a different operation than the begin_
            // method throws IllegalArgumentException. If the test throws as expected, we never call the end_ method,
            // so this also tests that it is safe to throw the AsyncResult away without calling the end_ method.
            //
            MyClassPrx indirect1 = MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy").ice_oneway());
            MyClassPrx indirect2 = MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy2").ice_oneway());

            Ice.AsyncResult r1 = indirect1.begin_opVoid();
            Ice.AsyncResult r2 = indirect2.begin_opVoid();

            try
            {
                indirect1.end_opVoid(r2); // Wrong proxy
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }

            try
            {
                indirect1.end_shutdown(r1); // Wrong operation
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }
        }

        {
            //
            // Check that calling the end_ method with a null result throws IllegalArgumentException.
            //
            try
            {
                p.end_opVoid(null);
                test(false);
            }
            catch(IllegalArgumentException ex)
            {
            }
        }

        {
            //
            // Check that throwing an exception from the exception callback doesn't cause problems.
            //
            MyClassPrx indirect = MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy").ice_oneway());

            {
                ThrowerSafeEx cb = new ThrowerSafeEx(ThrowType.LocalException);
                indirect.begin_opVoid(cb);
                cb.check();
            }

            {
                ThrowerSafeEx cb = new ThrowerSafeEx(ThrowType.OtherException);
                indirect.begin_opVoid(cb);
                cb.check();
            }

            {
                ThrowerUnsafeEx cb = new ThrowerUnsafeEx(ThrowType.LocalException);
                indirect.begin_opVoid(cb);
                cb.check();
            }

            {
                ThrowerUnsafeEx cb = new ThrowerUnsafeEx(ThrowType.OtherException);
                indirect.begin_opVoid(cb);
                cb.check();
            }
        }

        {
            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            Ice.Communicator ic = app.initialize(initData);
            Ice.ObjectPrx obj = ic.stringToProxy(p.toString()).ice_oneway();
            MyClassPrx p2 = MyClassPrxHelper.uncheckedCast(obj);

            ic.destroy();

            try
            {
                p2.begin_opVoid();
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
                // Expected.
            }
        }

        {
            //
            // Check that we can call operations on Object asynchronously.
            //
            {
                Ice.AsyncResult r = p.begin_ice_ping();
                try
                {
                    p.end_ice_ping(r);
                }
                catch(RuntimeException ex)
                {
                    test(false);
                }
            }

            {
                PingCallback cb = new PingCallback();
                p.begin_ice_ping(cb);
            }
        }

        //
        // Test that marshaling works as expected, and that the delegates for each type of callback work.
        //

        {
            Ice.AsyncResult r = p.begin_opVoid();
            p.end_opVoid(r);
        }

        //
        // Test that calling a twoway operation with a oneway proxy raises TwowayOnlyException.
        //
        {
            Ice.AsyncResult r = p.begin_opByte((byte)0xff, (byte)0x0f);
            try
            {
                p.end_opByte(null, r);
                test(false);
            }
            catch(Ice.TwowayOnlyException ex)
            {
            }
        }

        //
        // Test that queuing indication works.
        //
        {
            Ice.AsyncResult r = p.begin_delay(100);
            r.waitForSent();
            test(r.isCompleted() && r.isSent());
            p.end_delay(r);
            test(r.isCompleted());
        }

        //
        // Put the server's adapter into the holding state and pump out requests until one is queued.
        // Then activate the adapter again and pump out more until one isn't queued again.
        // Check that all the callbacks arrive after calling the end_ method for each request.
        // We fill a context with a few kB of data to make sure we don't queue up too many requests.
        //
        {
            final int contextSize = 10; // Kilobytes
            StringBuilder s = new StringBuilder();
            for(int i = 0; i < 1024; ++i)
            {
                s.append('a');
            }

            java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
            for(int i = 0; i < contextSize; ++i)
            {
                ctx.put("i" + i, s.toString());
            }

            //
            // Keep all the AsyncResults we get from the begin_ calls, so we can call end_ for each of them.
            //
            java.util.ArrayList<Ice.AsyncResult> results = new java.util.ArrayList<Ice.AsyncResult>();

            int queuedCount = 0;

            SentCounter cb = new SentCounter();
            Ice.AsyncResult r;

            StateChangerPrx state =
                StateChangerPrxHelper.checkedCast(communicator.stringToProxy("hold:default -p 12011"));
            state.hold(3);

            do
            {
                r = p.begin_opVoid(ctx, cb);
                results.add(r);
                if(!r.sentSynchronously())
                {
                    ++queuedCount;
                }
            }
            while(r.sentSynchronously());

            int numRequests = results.size();
            test(numRequests > 1); // Something is wrong if we didn't get something out without queueing.

            //
            // Re-enable the adapter.
            //
            state.activate(3);

            //
            // Fire off a bunch more requests until we get one that wasn't queued.
            // We sleep in between calls to allow the queued requests to drain.
            //
            do
            {
                r = p.begin_opVoid(cb);
                results.add(r);
                if(!r.sentSynchronously())
                {
                    ++queuedCount;
                }
                try
                {
                    Thread.sleep(1);
                }
                catch(InterruptedException ex)
                {
                }
            }
            while(!r.sentSynchronously());
            test(results.size() > numRequests); // Something is wrong if we didn't queue additional requests.

            //
            // Now make all the outstanding calls to the end_ method.
            //
            for(Ice.AsyncResult ar : results)
            {
                p.end_opVoid(ar);
            }

            //
            // Check that we got a sent callback for each queued request.
            //
            cb.check(queuedCount);
        }
    }
}
