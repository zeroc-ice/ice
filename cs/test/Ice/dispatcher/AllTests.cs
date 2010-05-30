// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public void check()
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

        public void response()
        {
            test(Dispatcher.isDispatcherThread());
            called();
        }

        public void exception(Ice.Exception ex)
        {
            test(ex is Ice.NoEndpointException);
            test(Dispatcher.isDispatcherThread());
            called();
        }

        public void payload()
        {
            test(Dispatcher.isDispatcherThread());
        }

        public void ignoreEx(Ice.Exception ex)
        {
            test(ex is Ice.CommunicatorDestroyedException);
        }

        public void sent(bool sentSynchronously)
        {
            test(sentSynchronously || Dispatcher.isDispatcherThread());
        }

        protected void called()
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

    public static void allTests(Ice.Communicator communicator)
    {
        string sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfPrx p = Test.TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:tcp -p 12011";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfControllerPrx testController = Test.TestIntfControllerPrxHelper.uncheckedCast(obj);

        Console.Out.Write("testing dispatcher... ");
        Console.Out.Flush();
        {
            p.op();

            Callback cb = new Callback();
            p.begin_op().whenCompleted(cb.response, cb.exception);
            cb.check();

            TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
            i.begin_op().whenCompleted(cb.exception);
            cb.check();

            testController.holdAdapter();
            Test.Callback_TestIntf_opWithPayload resp = cb.payload;
            Ice.ExceptionCallback excb = cb.ignoreEx;
            Ice.SentCallback scb = cb.sent;

            byte[] seq = new byte[10 * 1024];
            (new System.Random()).NextBytes(seq);
            Ice.AsyncResult r;
            while((r = p.begin_opWithPayload(seq).whenCompleted(resp, excb).whenSent(scb)).sentSynchronously());
            testController.resumeAdapter();
            r.waitForCompleted();
        }
        Console.Out.WriteLine("ok");

        p.shutdown();
    }
}
