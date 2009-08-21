// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Diagnostics;
using System.Threading;

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

        public virtual bool isCalled()
        {
            lock(this)
            {
                return _called;
            }
        }

        private bool _called;
    }

    private class OpThread
    {
        internal OpThread(BackgroundPrx background)
        {
            _background = BackgroundPrxHelper.uncheckedCast(background.ice_oneway());
            Start();
        }

        public void Join()
        {
            _thread.Join();
        }

        public void Start()
        {
            _thread = new Thread(new ThreadStart(Run));
            _thread.Start();
        }

        public void Run()
        {
            int count = 0;
            while(true)
            {
                lock(this)
                {
                    if(_destroyed)
                    {
                        return;
                    }
                }

                try
                {
                    if(++count == 10) // Don't blast the connection with only oneway's
                    {
                        count = 0;
                        _background.ice_twoway().ice_ping();
                    }
                    _background.op_async(new OpExAMICallback());
                    Thread.Sleep(1);
                }
                catch(Ice.LocalException)
                {
                }
            }
        }

        public void destroy()
        {
            lock(this)
            {
                _destroyed = true;
            }
        }

        private bool _destroyed = false;
        private BackgroundPrx _background = null;
        private Thread _thread;
    }

    private class OpAMICallback : Test.AMI_Background_op, Ice.AMISentCallback
    {
        public override void ice_response()
        {
            resp.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }

        public void ice_sent()
        {
            sent.called();
        }

        public bool response(bool wait)
        {
            if(wait)
            {
                resp.check();
                return true;
            }
            else
            {
                return resp.isCalled();
            }
        }

        public void responseAndSent()
        {
            sent.check();
            resp.check();
        }

        private Callback resp = new Callback();
        private Callback sent = new Callback();
    }

    private class OpExAMICallback : Test.AMI_Background_op, Ice.AMISentCallback
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            _ex.called();
        }

        public void ice_sent()
        {
            _sent.called();
        }

        public void exception()
        {
            _ex.check();
        }

        public bool sent(bool wait)
        {
            if(wait)
            {
                _sent.check();
                return true;
            }
            else
            {
                return _sent.isCalled();
            }
        }
        
        private Callback _ex = new Callback();
        private Callback _sent = new Callback();
    }

    private class OpWithPayloadOnewayAMICallback : Test.AMI_Background_opWithPayload
    {
        public override void ice_response()
        {
            test(false);
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }
    }

    private class FlushBatchRequestsCallback : Ice.AMI_Object_ice_flushBatchRequests
    {
        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }
    }

    public static Test.BackgroundPrx allTests(Ice.Communicator communicator)
    {
        string sref = "background:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundPrx background = BackgroundPrxHelper.uncheckedCast(obj);

        sref = "backgroundController:tcp -p 12011";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundControllerPrx backgroundController = BackgroundControllerPrxHelper.uncheckedCast(obj);

        Configuration configuration = Configuration.getInstance();

        Console.Write("testing connect... ");
        Console.Out.Flush();
        {
            connectTests(configuration, background);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing initialization... ");
        Console.Out.Flush();
        {
            initializeTests(configuration, background, backgroundController);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing connection validation... ");
        Console.Out.Flush();
        {
            validationTests(configuration, background, backgroundController);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing read/write... ");
        Console.Out.Flush();
        {
            readWriteTests(configuration, background, backgroundController);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing locator... ");
        Console.Out.Flush();
        {
            Ice.LocatorPrx locator;
            obj = communicator.stringToProxy("locator:default -p 12010 -t 500");
            locator = Ice.LocatorPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_locator(locator).ice_oneway();

            backgroundController.pauseCall("findAdapterById");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch(Ice.TimeoutException)
            {
            }
            backgroundController.resumeCall("findAdapterById");

            obj = communicator.stringToProxy("locator:default -p 12010");
            locator = Ice.LocatorPrxHelper.uncheckedCast(obj);
            obj = obj.ice_locator(locator);
            obj.ice_ping();

            obj = communicator.stringToProxy("background@Test").ice_locator(locator);
            BackgroundPrx bg = BackgroundPrxHelper.uncheckedCast(obj);

            backgroundController.pauseCall("findAdapterById");
            OpAMICallback cb = new OpAMICallback();
            bg.op_async(cb);
            OpAMICallback cb2 = new OpAMICallback();
            bg.op_async(cb2);
            test(!cb.response(false));
            test(!cb2.response(false));
            backgroundController.resumeCall("findAdapterById");
            cb.response(true);
            cb2.response(true);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing router... ");
        Console.Out.Flush();
        {
            Ice.RouterPrx router;

            obj = communicator.stringToProxy("router:default -p 12010 -t 500");
            router = Ice.RouterPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router).ice_oneway();

            backgroundController.pauseCall("getClientProxy");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch(Ice.TimeoutException)
            {
            }
            backgroundController.resumeCall("getClientProxy");

            obj = communicator.stringToProxy("router:default -p 12010");
            router = Ice.RouterPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router);
            BackgroundPrx bg = BackgroundPrxHelper.uncheckedCast(obj);
            test(bg.ice_getRouter() != null);

            backgroundController.pauseCall("getClientProxy");
            OpAMICallback cb = new OpAMICallback();
            bg.op_async(cb);
            OpAMICallback cb2 = new OpAMICallback();
            bg.op_async(cb2);
            test(!cb.response(false));
            test(!cb2.response(false));
            backgroundController.resumeCall("getClientProxy");
            cb.response(true);
            cb2.response(true);
        }
        Console.Out.WriteLine("ok");

        return background;
    }

    private static void connectTests(Configuration configuration, Test.BackgroundPrx background)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException ex)
        {
            System.Console.Out.WriteLine(ex);
            test(false);
        }
        background.ice_getConnection().close(false);

        try
        {
            configuration.connectorsException(new Ice.DNSException());
            background.op();
            test(false);
        }
        catch(Ice.DNSException)
        {
            configuration.connectorsException(null);
        }

        OpExAMICallback cbEx = new OpExAMICallback();

        configuration.connectorsException(new Ice.DNSException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        configuration.connectorsException(null);

        configuration.connectorsException(new Ice.DNSException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        configuration.connectorsException(null);

        try
        {
            configuration.connectException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException)
        {
            configuration.connectException(null);
        }

        configuration.connectException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        configuration.connectException(null);

        configuration.connectException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        configuration.connectException(null);

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for(int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
                test(false);
            }

            configuration.connectException(new Ice.SocketException());
            background.ice_getCachedConnection().close(true);
            Thread.Sleep(10);
            configuration.connectException(null);
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
            }
        }

        thread1.destroy();
        thread2.destroy();

        thread1.Join();
        thread2.Join();
    }

    private static void initializeTests(Configuration configuration, Test.BackgroundPrx background,
                                        Test.BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        background.ice_getConnection().close(false);

        try
        {
            configuration.initializeException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException)
        {
            configuration.initializeException(null);
        }

        OpExAMICallback cbEx = new OpExAMICallback();

        configuration.initializeException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        configuration.initializeException(null);

        configuration.initializeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        configuration.initializeException(null);

        //
        // Now run the same tests with the server side.
        //

        try
        {
            ctl.initializeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            ctl.initializeException(false);
        }
        catch(Ice.SecurityException)
        {
            ctl.initializeException(false);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for(int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
                test(false);
            }

            configuration.initializeException(new Ice.SocketException());
            background.ice_getCachedConnection().close(true);
            Thread.Sleep(10);
            configuration.initializeException(null);
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
                test(false);
            }

            background.ice_getCachedConnection().close(true);
            background.ice_ping();

            ctl.initializeException(true);
            background.ice_getCachedConnection().close(true);
            Thread.Sleep(10);
            ctl.initializeException(false);
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
                test(false);
            }

            try
            {
                background.ice_getCachedConnection().close(true);
                background.op();
            }
            catch(Ice.LocalException)
            {
                test(false);
            }
        }

        thread1.destroy();
        thread2.destroy();

        thread1.Join();
        thread2.Join();
    }

    private static void validationTests(Configuration configuration, Test.BackgroundPrx background,
                                        Test.BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException)
        {
            test(false);
        }
        background.ice_getConnection().close(false);

        try
        {
            // Get the read() of connection validation to throw right away.
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException)
        {
            configuration.readException(null);
        }

        OpExAMICallback cbEx = new OpExAMICallback();

        configuration.readException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        configuration.readException(null);

        configuration.readException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        configuration.readException(null);

        if(!background.ice_getCommunicator().getProperties().getProperty("Ice.Default.Protocol").Equals("test-ssl"))
        {
            try
            {
                // Get the read() of the connection validation to return "would block"
                configuration.readReady(false);
                background.op();
                configuration.readReady(true);
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                test(false);
            }
            background.ice_getConnection().close(false);

            try
            {
                // Get the read() of the connection validation to return "would block" and then throw.
                configuration.readReady(false);
                configuration.readException(new Ice.SocketException());
                background.op();
                test(false);
            }
            catch(Ice.SocketException)
            {
                configuration.readException(null);
                configuration.readReady(true);
            }

            configuration.readReady(false);
            configuration.readException(new Ice.SocketException());
            test(!background.op_async(cbEx));
            cbEx.exception();
            configuration.readException(null);
            configuration.readReady(true);

            configuration.readReady(false);
            configuration.readException(new Ice.SocketException());
            test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
            cbEx.exception();
            configuration.readException(null);
            configuration.readReady(true);
        }

        ctl.holdAdapter(); // Hold to block in connection validation
        OpAMICallback cb = new OpAMICallback();
        test(!background.op_async(cb));
        OpAMICallback cb2 = new OpAMICallback();
        test(!background.op_async(cb2));
        test(!cb.response(false));
        test(!cb2.response(false));
        ctl.resumeAdapter();
        cb.responseAndSent();
        cb2.responseAndSent();

        try
        {
            // Get the write() of connection validation to throw right away.
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            ctl.writeException(false);
        }

        try
        {
            // Get the write() of the connection validation to return "would block"
            ctl.writeReady(false);
            background.op();
            ctl.writeReady(true);
        }
        catch(Ice.LocalException ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }
        background.ice_getConnection().close(false);

        try
        {
            // Get the write() of the connection validation to return "would block" and then throw.
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

        byte[] seq = new byte[512 * 1024];

        BackgroundPrx backgroundBatchOneway = BackgroundPrxHelper.uncheckedCast(background.ice_batchOneway());

        //
        // First send small requests to test without auto-flushing.
        //
        backgroundBatchOneway.ice_ping();
        backgroundBatchOneway.ice_getConnection().close(false);
        try
        {
            backgroundBatchOneway.ice_ping();
            test(false);
        }
        catch(Ice.CloseConnectionException)
        {
        }
        ctl.holdAdapter();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        ctl.resumeAdapter();
        backgroundBatchOneway.ice_flushBatchRequests();

        //
        // Send bigger requests to test with auto-flushing.
        //
        backgroundBatchOneway.ice_ping();
        backgroundBatchOneway.ice_getConnection().close(false);
        try
        {
            backgroundBatchOneway.ice_ping();
            test(false);
        }
        catch(Ice.CloseConnectionException)
        {
        }
        ctl.holdAdapter();
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        ctl.resumeAdapter();
        backgroundBatchOneway.ice_flushBatchRequests();

        //
        // Then try the same thing with async flush.
        //

        backgroundBatchOneway.ice_ping();
        backgroundBatchOneway.ice_getConnection().close(false);
        try
        {
            backgroundBatchOneway.ice_ping();
            test(false);
        }
        catch(Ice.CloseConnectionException)
        {
        }
        ctl.holdAdapter();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        ctl.resumeAdapter();
        backgroundBatchOneway.ice_flushBatchRequests_async(new FlushBatchRequestsCallback());
        backgroundBatchOneway.ice_getConnection().close(false);

        backgroundBatchOneway.ice_ping();
        backgroundBatchOneway.ice_getConnection().close(false);
        try
        {
            backgroundBatchOneway.ice_ping();
            test(false);
        }
        catch(Ice.CloseConnectionException)
        {
        }
        ctl.holdAdapter();
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        ctl.resumeAdapter();
        FlushBatchRequestsCallback fcb = new FlushBatchRequestsCallback();
        backgroundBatchOneway.ice_flushBatchRequests_async(fcb);
        //
        // We can't close the connection before ensuring all the batches have been sent since
        // with auto-flushing the close connection message might be sent once the first call
        // opWithPayload is sent and before the flushBatchRequests (this would therefore result
        // in the flush to report a CloseConnectionException). Instead we flush a second time
        // with the same callback to wait for the first flush to complete.
        //
        //backgroundBatchOneway.ice_getConnection().close(false);
        backgroundBatchOneway.ice_flushBatchRequests_async(fcb);
        backgroundBatchOneway.ice_getConnection().close(false);
    }

    private static void readWriteTests(Configuration configuration, Test.BackgroundPrx background,
                                       Test.BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }

        try
        {
            background.ice_ping();
            configuration.writeException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException)
        {
            configuration.writeException(null);
        }

        OpExAMICallback cbEx = new OpExAMICallback();

        configuration.writeException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        test(!cbEx.sent(false));
        configuration.writeException(null);

        configuration.writeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        test(!cbEx.sent(false));
        configuration.writeException(null);

        try
        {
            background.ice_ping();
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException)
        {
            configuration.readException(null);
        }

        background.ice_ping();
        configuration.readReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
        configuration.readException(new Ice.SocketException());
        if(!background.op_async(cbEx))
        {
            test(cbEx.sent(true));
        }
        cbEx.exception();
        configuration.readException(null);

        try
        {
            background.ice_ping();
            configuration.writeReady(false);
            background.op();
            configuration.writeReady(true);
        }
        catch(Ice.LocalException)
        {
            test(false);
        }

        try
        {
            background.ice_ping();
            configuration.readReady(false);
            background.op();
            configuration.readReady(true);
        }
        catch(Ice.LocalException)
        {
            test(false);
        }

        try
        {
            background.ice_ping();
            configuration.writeReady(false);
            configuration.writeException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException)
        {
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        background.ice_ping();
        configuration.writeReady(false);
        configuration.writeException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        test(!cbEx.sent(false));
        configuration.writeException(null);
        configuration.writeReady(true);

        background.ice_ping();
        configuration.writeReady(false);
        configuration.writeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        test(!cbEx.sent(false));
        configuration.writeException(null);
        configuration.writeReady(true);

        try
        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException)
        {
            configuration.readException(null);
            configuration.readReady(true);
        }

        background.ice_ping();
        configuration.readReady(false);
        configuration.readException(new Ice.SocketException());
        if(!background.op_async(cbEx))
        {
            test(cbEx.sent(true));
        }
        cbEx.exception();
        configuration.readException(null);
        configuration.readReady(true);

        background.ice_ping();
        configuration.readReady(false);
        configuration.writeReady(false);
        configuration.readException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        test(cbEx.sent(true));
        cbEx.exception();
        configuration.readException(null);
        configuration.writeReady(true);
        configuration.readReady(true);

        background.ice_ping(); // Establish the connection

        BackgroundPrx backgroundOneway = BackgroundPrxHelper.uncheckedCast(background.ice_oneway());
        test(backgroundOneway.ice_getConnection() == background.ice_getConnection());

        ctl.holdAdapter(); // Hold to block in request send.

        byte[] seq = new byte[512 * 1024];
        (new System.Random()).NextBytes(seq);
        while(backgroundOneway.opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq))
        {
        }
        OpAMICallback cb = new OpAMICallback();
        test(!background.op_async(cb));
        OpAMICallback cb2 = new OpAMICallback();
        test(!background.op_async(cb2));
        backgroundOneway.opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq);
        backgroundOneway.opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq);
        test(!cb.response(false));
        test(!cb2.response(false));
        ctl.resumeAdapter();
        cb.responseAndSent();
        cb2.responseAndSent();


        try
        {
            background.ice_ping();
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            ctl.writeException(false);
        }

        try
        {
            background.ice_ping();
            ctl.readException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            ctl.readException(false);
        }

        try
        {
            background.ice_ping();
            ctl.writeReady(false);
            background.op();
            ctl.writeReady(true);
        }
        catch(Ice.LocalException)
        {
            test(false);
        }

        try
        {
            background.ice_ping();
            ctl.readReady(false);
            background.op();
            ctl.readReady(true);
        }
        catch(Ice.LocalException)
        {
            test(false);
        }

        try
        {
            background.ice_ping();
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

        try
        {
            background.ice_ping();
            ctl.readReady(false);
            ctl.readException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException)
        {
            ctl.readException(false);
            ctl.readReady(true);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for(int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException)
            {
                test(false);
            }

            Thread.Sleep(10);
            configuration.writeException(new Ice.SocketException());
            try
            {
                background.op();
            }
            catch(Ice.LocalException)
            {
            }
            configuration.writeException(null);

            Thread.Sleep(10);

            background.ice_ping();
            background.ice_getCachedConnection().close(true);
            Thread.Sleep(10);

            background.ice_getCachedConnection().close(true);
        }

        thread1.destroy();
        thread2.destroy();

        thread1.Join();
        thread2.Join();
    }
}
