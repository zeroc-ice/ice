// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;

public class OnewaysNewAMI
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
        }
    }

    private class CallbackBase
    {
        internal CallbackBase()
        {
            _called = false;
        }

        public virtual void check()
        {
            lock(this)
            {
                while(!_called)
                {
                    Monitor.Wait(this);
                }
                _called = false;
            }
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    private class NoEndpointCallback : CallbackBase
    {
        public void @unsafe(Ice.AsyncResult r)
        {
            try
            {
                Test.MyClassPrx p = (Test.MyClassPrx)r.getProxy();
                p.end_opVoid(r);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
                called();
            }
        }

        public void success()
        {
            test(false);
        }

        public void exCB(Ice.Exception ex)
        {
            test(ex is Ice.NoEndpointException);
            called();
        }

        public void sentCB()
        {
            test(false);
        }
    }

    private class Cookie
    {
        public Cookie(int i)
        {
            val = i;
        }

        public int val;
    }

    private class Callback : CallbackBase
    {
        public Callback()
        {
        }

        public void opVoid()
        {
            called();
        }

        public void opDoubleMarshaling()
        {
            called();
        }

        public void opDerived()
        {
            called();
        }

        public void exCB(Ice.Exception ex)
        {
            test(false);
        }

        public void exCBCookie(Ice.AsyncResult result)
        {
            Cookie cookie = (Cookie)result.AsyncState;
            test(cookie.val == 99);
            called();
        }
    }

    private enum ThrowType { LocalException, UserException, OtherException};

    private class Thrower : CallbackBase
    {
        public Thrower(ThrowType t)
        {
            t_ = t;
        }

        public void exCB(Ice.Exception ex)
        {
            called();
            throwEx();
        }

        private void throwEx()
        {
            switch(t_)
            {
                case ThrowType.LocalException:
                {
                    throw new Ice.ObjectNotExistException();
                }
                case ThrowType.UserException:
                {
                    throw new Test.SomeException();
                }
                case ThrowType.OtherException:
                {
                    throw new System.ArgumentException();
                }
                default:
                {
                    Debug.Assert(false);
                    break;
                }
            }
        }

        private ThrowType t_;
    }

    private class SentCounter
    {
        public SentCounter()
        {
            _queuedCount = 0;
        }

        public void exCB(Ice.Exception e)
        {
            test(false);
        }

        public void sentCB()
        {
            lock(this)
            {
                ++_queuedCount;
            }
        }

        public int queuedCount()
        {
            lock(this)
            {
                return _queuedCount;
            }
        }

        public void check(int size)
        {
            lock(this)
            {
                while(_queuedCount != size)
                {
                    Monitor.Wait(this);
                }
            }
        }

        private int _queuedCount;
    }

    internal static void onewaysNewAMI(Ice.Communicator communicator, Test.MyClassPrx proxy)
    {
        Test.MyClassPrx p = Test.MyClassPrxHelper.uncheckedCast(proxy.ice_oneway());

        {
            //
            // Check that a call to a void operation raises NoEndpointException
            // in the end_ method instead of at the point of call.
            //
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy").ice_oneway());
            Ice.AsyncResult r;

            r = indirect.begin_opVoid();
            try
            {
                indirect.end_opVoid(r);
                test(false);
            }
            catch(Ice.NoEndpointException)
            {
            }

            //
            // Check that a second call to the end_ method throws ArgumentException.
            //
            try
            {
                indirect.end_opVoid(r);
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }

        {
            //
            // Check that a call to a void operation raises NoEndpointException in the callback.
            // Also test that the sent callback is not called in this case.
            //
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy").ice_oneway());

            NoEndpointCallback cb = new NoEndpointCallback();
            indirect.begin_opVoid().whenCompleted(cb.exCB).whenSent(cb.sentCB);
            cb.check();
        }

        {
            //
            // Check that calling the end_ method with a different proxy or for a different operation than the begin_
            // method throws ArgumentException. If the test throws as expected, we never call the end_ method,
            // so this also tests that it is safe to throw the AsyncResult away without calling the end_ method.
            //
            Test.MyClassPrx indirect1 = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy").ice_oneway());
            Test.MyClassPrx indirect2 = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy2").ice_oneway());

            Ice.AsyncResult r1 = indirect1.begin_opVoid();
            Ice.AsyncResult r2 = indirect2.begin_opVoid();

            try
            {
                indirect1.end_opVoid(r2); // Wrong proxy
                test(false);
            }
            catch(System.ArgumentException)
            {
            }

            try
            {
                indirect1.end_shutdown(r1); // Wrong operation
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }

        {
            //
            // Check that calling the end_ method with a null result throws ArgumentException.
            //
            try
            {
                p.end_opVoid(null);
                test(false);
            }
            catch(System.ArgumentException)
            {
            }
        }

        {
            //
            // Check that throwing an exception from the exception callback doesn't cause problems.
            //
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));

            {
                Thrower cb = new Thrower(ThrowType.LocalException);
                indirect.begin_opVoid().whenCompleted(cb.exCB);
                cb.check();
            }

            {
                Thrower cb = new Thrower(ThrowType.UserException);
                indirect.begin_opVoid().whenCompleted(cb.exCB);
                cb.check();
            }

            {
                Thrower cb = new Thrower(ThrowType.OtherException);
                indirect.begin_opVoid().whenCompleted(cb.exCB);
                cb.check();
            }
        }

        {
            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            Ice.Communicator ic = Ice.Util.initialize(initData);
            Ice.ObjectPrx obj = ic.stringToProxy(p.ToString()).ice_oneway();
            Test.MyClassPrx p2 = Test.MyClassPrxHelper.uncheckedCast(obj);

            ic.destroy();

            try
            {
                p2.begin_opVoid();
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException)
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
                catch(Exception)
                {
                    test(false);
                }
            }

            {
                Callback cb = new Callback();
                p.begin_ice_ping().whenCompleted(cb.exCB);
            }
        }

        //
        // Test that marshaling works as expected, and that the delegates for each type of callback work.
        //
        {
            {
                Ice.AsyncResult r = p.begin_opVoid();
                p.end_opVoid(r);
            }
        }

        //
        // Test that calling a twoway operation with a oneway proxy raises TwowayOnlyException.
        //
        {
            Ice.AsyncResult r = p.begin_opByte(0xff, 0x0f);
            try
            {
                byte p3;
                p.end_opByte(out p3, r);
                test(false);
            }
            catch(Ice.TwowayOnlyException)
            {
            }
        }

        {
            double d = 1278312346.0 / 13.0;
            double[] tdsi = new double[] { 5, d };
            Test.DoubleS ds = new Test.DoubleS(tdsi);
            Callback cb = new Callback();
            p.begin_opDoubleMarshaling(d, ds).whenCompleted(cb.exCB);
        }

        {
            Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.uncheckedCast(p);
            test(derived != null);
            Callback cb = new Callback();
            derived.begin_opDerived().whenCompleted(cb.exCB);
        }

        //
        // Test that cookies work.
        //
        {
            Test.MyClassPrx indirect = Test.MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy").ice_oneway());
            Callback cb = new Callback();
            Cookie cookie = new Cookie(99);
            indirect.begin_opVoid(cb.exCBCookie, cookie);
            cb.check();
        }

        //
        // Test that queuing indication works.
        //
        {
            Ice.AsyncResult r = p.begin_delay(100);
            r.waitForSent();
            test(r.IsCompleted && r.isSent());
            p.end_delay(r);
            test(r.IsCompleted);
        }

        //
        // Put the server's adapter into the holding state and pump out requests until one is queued.
        // Then activate the adapter again and pump out more until one isn't queued again.
        // Check that all the callbacks arrive after calling the end_ method for each request.
        // We fill a context with a few kB of data to make sure we don't queue up too many requests.
        //
        {
            const int contextSize = 10; // Kilobytes
            string s = new string('a', 1024);

            Dictionary<string, string> ctx = new Dictionary<string, string>();
            for(int i = 0; i < contextSize; ++i)
            {
                string label = "i" + i;
                ctx[label] = s;
            }

            //
            // Keep all the AsyncResults we get from the begin_ calls, so we can call end_ for each of them.
            //
//             List<Ice.AsyncResult> results = new List<Ice.AsyncResult>();

//             int queuedCount = 0;

//             SentCounter cb = new SentCounter();
//             Ice.AsyncResult r;

//             Test.StateChangerPrx state = Test.StateChangerPrxHelper.checkedCast(
//                                             communicator.stringToProxy("hold:default -p 12011"));
//             state.hold(3);

//             do
//             {
//                 r = p.begin_opVoid(ctx).whenCompleted(cb.exCB).whenSent(cb.sentCB);
//                 results.Add(r);
//                 if(!r.SentSynchronously)
//                 {
//                     ++queuedCount;
//                 }
//             }
//             while(r.SentSynchronously);

//             int numRequests = results.Count;
//             test(numRequests > 1); // Something is wrong if we didn't get something out without queueing.

//             //
//             // Re-enable the adapter.
//             //
//             state.activate(3);

//             //
//             // Fire off a bunch more requests until we get one that wasn't queued.
//             // We sleep in between calls to allow the queued requests to drain.
//             //
//             do
//             {
//                 r = p.begin_opVoid().whenCompleted(cb.exCB).whenSent(cb.sentCB);
//                 results.Add(r);
//                 if(!r.SentSynchronously)
//                 {
//                     ++queuedCount;
//                 }
//                 Thread.Sleep(1);
//             }
//             while(!r.SentSynchronously);
//             test(results.Count > numRequests); // Something is wrong if we didn't queue additional requests.

//             //
//             // Now make all the outstanding calls to the end_ method.
//             //
//             foreach(Ice.AsyncResult ar in results)
//             {
//                 p.end_opVoid(ar);
//             }

//             //
//             // Check that we got a sent callback for each queued request.
//             //
//             cb.check(queuedCount);
        }
    }
}
