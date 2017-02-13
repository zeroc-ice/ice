// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#if SILVERLIGHT
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
#endif

public class AllTests : TestCommon.TestApp
{
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
                    System.Threading.Monitor.Wait(this);
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

        public void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Warn.AMICallback", "0");
        initData.dispatcher = new Dispatcher().dispatch;
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static void allTests(Ice.Communicator communicator)
#endif
    {
        string sref = "test:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfPrx p = Test.TestIntfPrxHelper.uncheckedCast(obj);

        sref = "testController:tcp -p 12011";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        Test.TestIntfControllerPrx testController = Test.TestIntfControllerPrxHelper.uncheckedCast(obj);

        Write("testing dispatcher... ");
        Flush();
        {
            p.op();

            Callback cb = new Callback();
            p.begin_op().whenCompleted(cb.response, cb.exception);
            cb.check();

            TestIntfPrx i = (TestIntfPrx)p.ice_adapterId("dummy");
            i.begin_op().whenCompleted(cb.exception);
            cb.check();

            //
            // Expect InvocationTimeoutException.
            //
            {
                Test.TestIntfPrx to = Test.TestIntfPrxHelper.uncheckedCast(p.ice_invocationTimeout(250));
                to.begin_sleep(500).whenCompleted(
                    () =>
                    {
                        test(false);
                    },
                    (Ice.Exception ex) => {
                        test(ex is Ice.InvocationTimeoutException);
                        test(Dispatcher.isDispatcherThread());
                        cb.called();
                    });
                cb.check();
            }

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
        WriteLine("ok");

        p.shutdown();
    }
}
