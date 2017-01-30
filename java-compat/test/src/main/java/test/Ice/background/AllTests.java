// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

import java.io.PrintWriter;

import test.Ice.background.Test.BackgroundControllerPrx;
import test.Ice.background.Test.BackgroundControllerPrxHelper;
import test.Ice.background.Test.BackgroundPrx;
import test.Ice.background.Test.BackgroundPrxHelper;
import test.Ice.background.Test.Callback_Background_op;
import test.Ice.background.Test.Callback_Background_opWithPayload;

public class AllTests
{
    private static void test(boolean b)
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

        public synchronized boolean isCalled()
        {
            return _called;
        }

        private boolean _called;
    }

    private static class OpAMICallback extends Callback_Background_op
    {
        @Override
        public void response()
        {
            _response.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        @Override
        public void sent(boolean ss)
        {
            _sent.called();
        }

        public boolean response(boolean wait)
        {
            if(wait)
            {
                _response.check();
                return true;
            }
            else
            {
                return _response.isCalled();
            }
        }

        public void responseAndSent()
        {
            _sent.check();
            _response.check();
        }

        private Callback _response = new Callback();
        private Callback _sent = new Callback();
    }

    private static class OpAMICallbackEx extends Callback_Background_op
    {
        @Override
        public void response()
        {
            test(false);
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            _response.called();
        }

        @Override
        public void sent(boolean ss)
        {
            _sent.called();
        }

        public boolean exception(boolean wait)
        {
            if(wait)
            {
                _response.check();
                return true;
            }
            else
            {
                return _response.isCalled();
            }
        }

        private Callback _response = new Callback();
        private Callback _sent = new Callback();
    }

    private static class OpAMICallbackNoOp extends Callback_Background_op
    {
        @Override
        public void response()
        {
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        @Override
        public void sent(boolean ss)
        {
        }
    }

    private static class NoResponse extends Callback_Background_opWithPayload
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
    }

    /*
    private static class OpWithPayloadOnewayAMICallback extends AMI_Background_opWithPayload
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }
    }

    private static class FlushBatchRequestsCallback extends Ice.AMI_Object_ice_flushBatchRequests
    {
        public void
        ice_exception(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
    }
    */

    static class OpThread extends Thread
    {
        OpThread(BackgroundPrx background)
        {
            _destroyed = false;
            _background = BackgroundPrxHelper.uncheckedCast(background.ice_oneway());
            start();
        }

        @Override
        public void
        run()
        {
            int count = 0;
            while(true)
            {
                synchronized(this)
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
                    _background.op();
                    try
                    {
                        Thread.sleep(1);
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                catch(Ice.LocalException ex)
                {
                }
            }
        }

        public synchronized void
        _destroy()                  // Thread.destroy is deprecated
        {
            _destroyed = true;
        }

        private boolean _destroyed;
        private BackgroundPrx _background = null;
    }

    public static BackgroundPrx
    allTests(test.Util.Application app, Configuration configuration)
    {
        Ice.Communicator communicator = app.communicator();
        PrintWriter out = app.getWriter();

        String sref = "background:" + app.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundPrx background = BackgroundPrxHelper.uncheckedCast(obj);

        sref = "backgroundController:" + app.getTestEndpoint(1, "tcp");
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundControllerPrx backgroundController = BackgroundControllerPrxHelper.uncheckedCast(obj);

        out.print("testing connect... ");
        out.flush();
        {
            connectTests(configuration, background);
        }
        out.println("ok");

        out.print("testing initialization... ");
        out.flush();
        {
            initializeTests(configuration, background, backgroundController);
        }
        out.println("ok");

        out.print("testing connection validation... ");
        out.flush();
        {
            validationTests(configuration, background, backgroundController);
        }
        out.println("ok");

        out.print("testing read/write... ");
        out.flush();
        {
            readWriteTests(configuration, background, backgroundController);
        }
        out.println("ok");

        out.print("testing locator... ");
        out.flush();
        {
            Ice.LocatorPrx locator;
            obj = communicator.stringToProxy("locator:" + app.getTestEndpoint(0)).ice_invocationTimeout(250);
            locator = Ice.LocatorPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_locator(locator).ice_oneway();

            backgroundController.pauseCall("findAdapterById");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
            }
            backgroundController.resumeCall("findAdapterById");

            obj = communicator.stringToProxy("locator:" + app.getTestEndpoint(0));
            locator = Ice.LocatorPrxHelper.uncheckedCast(obj);
            obj = obj.ice_locator(locator);
            obj.ice_ping();

            obj = communicator.stringToProxy("background@Test").ice_locator(locator);
            BackgroundPrx bg = BackgroundPrxHelper.uncheckedCast(obj);

            backgroundController.pauseCall("findAdapterById");
            Ice.AsyncResult r1 = bg.begin_op();
            Ice.AsyncResult r2 = bg.begin_op();
            test(!r1.isCompleted());
            test(!r2.isCompleted());
            backgroundController.resumeCall("findAdapterById");
            bg.end_op(r1);
            bg.end_op(r2);
            test(r1.isCompleted());
            test(r2.isCompleted());
        }
        out.println("ok");

        out.print("testing router... ");
        out.flush();
        {
            Ice.RouterPrx router;

            obj = communicator.stringToProxy("router:" + app.getTestEndpoint(0)).ice_invocationTimeout(250);
            router = Ice.RouterPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router).ice_oneway();

            backgroundController.pauseCall("getClientProxy");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch(Ice.TimeoutException ex)
            {
            }
            backgroundController.resumeCall("getClientProxy");

            obj = communicator.stringToProxy("router:" + app.getTestEndpoint(0));
            router = Ice.RouterPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router);
            BackgroundPrx bg = BackgroundPrxHelper.uncheckedCast(obj);
            test(bg.ice_getRouter() != null);

            backgroundController.pauseCall("getClientProxy");
            Ice.AsyncResult r1 = bg.begin_op();
            Ice.AsyncResult r2 = bg.begin_op();
            test(!r1.isCompleted());
            test(!r2.isCompleted());
            backgroundController.resumeCall("getClientProxy");
            bg.end_op(r1);
            bg.end_op(r2);
            test(r1.isCompleted());
            test(r2.isCompleted());
        }
        out.println("ok");

        final boolean ws = communicator.getProperties().getProperty("Ice.Default.Protocol").equals("test-ws");
        final boolean wss = communicator.getProperties().getProperty("Ice.Default.Protocol").equals("test-wss");
        if(!ws && !wss)
        {
            out.print("testing buffered transport... ");
            out.flush();

            configuration.buffered(true);
            backgroundController.buffered(true);
            background.begin_op();
            background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);
            background.begin_op();

            OpAMICallbackNoOp cb = new OpAMICallbackNoOp();
            java.util.List<Ice.AsyncResult> results = new java.util.ArrayList<Ice.AsyncResult>();
            for(int i = 0; i < 10000; ++i)
            {
                Ice.AsyncResult r = background.begin_op(cb);
                results.add(r);
                if(i % 50 == 0)
                {
                    backgroundController.holdAdapter();
                    backgroundController.resumeAdapter();
                }
                if(i % 100 == 0)
                {
                    r.waitForCompleted();
                }
            }
            for(Ice.AsyncResult r : results)
            {
                r.waitForCompleted();
            }

            out.println("ok");
        }

        return background;
    }

    private static void
    connectTests(Configuration configuration, BackgroundPrx background)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        for(int i = 0; i < 4; ++i)
        {
            if(i == 0 || i == 2)
            {
                configuration.connectorsException(new Ice.DNSException());
            }
            else
            {
                configuration.connectException(new Ice.SocketException());
            }
            BackgroundPrx prx = (i == 1 || i == 3) ? background : (BackgroundPrx)background.ice_oneway();

            try
            {
                prx.op();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }

            Ice.AsyncResult r = prx.begin_op();
            test(!r.sentSynchronously());
            try
            {
                prx.end_op(r);
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            test(r.isCompleted());

            OpAMICallbackEx cbEx = new OpAMICallbackEx();
            r = prx.begin_op(cbEx);
            test(!r.sentSynchronously());
            cbEx.exception(true);
            test(r.isCompleted());

            if(i == 0 || i == 2)
            {
                configuration.connectorsException(null);
            }
            else
            {
                configuration.connectException(null);
            }
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for(int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }

            configuration.connectException(new Ice.SocketException());
            background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);
            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            configuration.connectException(null);
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
            }
        }

        thread1._destroy();
        thread2._destroy();

        try
        {
            thread1.join();
            thread2.join();
        }
        catch(InterruptedException e)
        {
        }
    }

    private static void
    initializeTests(Configuration configuration, BackgroundPrx background, BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        for(int i = 0; i < 4; i++)
        {
            if(i == 0 || i == 2)
            {
                configuration.initializeException(new Ice.SocketException());
            }
            else
            {
                configuration.initializeSocketStatus(IceInternal.SocketOperation.Write);
                configuration.initializeException(new Ice.SocketException());
            }
            BackgroundPrx prx = (i == 1 || i == 3) ? background : (BackgroundPrx)background.ice_oneway();

            try
            {
                prx.op();
                test(false);
            }
            catch(Ice.SocketException ex)
            {
            }

            Ice.AsyncResult r = prx.begin_op();
            test(!r.sentSynchronously());
            try
            {
                prx.end_op(r);
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            test(r.isCompleted());

            OpAMICallbackEx cbEx = new OpAMICallbackEx();
            r = prx.begin_op(cbEx);
            test(!r.sentSynchronously());
            cbEx.exception(true);
            test(r.isCompleted());

            if(i == 0 || i == 2)
            {
                configuration.initializeException(null);
            }
            else
            {
                configuration.initializeSocketStatus(IceInternal.SocketOperation.None);
                configuration.initializeException(null);
            }
        }

        try
        {
            configuration.initializeSocketStatus(IceInternal.SocketOperation.Connect);
            background.op();
            configuration.initializeSocketStatus(IceInternal.SocketOperation.None);
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        try
        {
            configuration.initializeSocketStatus(IceInternal.SocketOperation.Write);
            background.op();
            configuration.initializeSocketStatus(IceInternal.SocketOperation.None);
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        try
        {
            configuration.initializeSocketStatus(IceInternal.SocketOperation.Write);
            configuration.initializeException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException ex)
        {
            configuration.initializeException(null);
            configuration.initializeSocketStatus(IceInternal.SocketOperation.None);
        }

        //
        // Now run the same tests with the server side.
        //

        try
        {
            ctl.initializeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException ex)
        {
            ctl.initializeException(false);
        }
        catch(Ice.SecurityException ex)
        {
            ctl.initializeException(false);
        }

        try
        {
            ctl.initializeSocketStatus(IceInternal.SocketOperation.Write);
            background.op();
            ctl.initializeSocketStatus(IceInternal.SocketOperation.None);
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        try
        {
            ctl.initializeSocketStatus(IceInternal.SocketOperation.Write);
            ctl.initializeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException ex)
        {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(IceInternal.SocketOperation.None);
        }
        catch(Ice.SecurityException ex)
        {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(IceInternal.SocketOperation.None);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for(int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }

            configuration.initializeException(new Ice.SocketException());
            background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);
            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            configuration.initializeException(null);
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }

            configuration.initializeSocketStatus(IceInternal.SocketOperation.Write);
            background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);

            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false); // Something's wrong with retries.
            }

            configuration.initializeSocketStatus(IceInternal.SocketOperation.None);

            ctl.initializeException(true);
            background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);
            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            ctl.initializeException(false);
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }

            try
            {
                ctl.initializeSocketStatus(IceInternal.SocketOperation.Write);
                background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);
                background.op();
                ctl.initializeSocketStatus(IceInternal.SocketOperation.None);
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false);
            }
        }

        thread1._destroy();
        thread2._destroy();

        try
        {
            thread1.join();
            thread2.join();
        }
        catch(InterruptedException e)
        {
        }
    }

    private static void
    validationTests(Configuration configuration, BackgroundPrx background, BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        try
        {
            // Get the read() of connection validation to throw right away.
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException ex)
        {
            configuration.readException(null);
        }

        for(int i = 0; i < 2; i++)
        {
            configuration.readException(new Ice.SocketException());
            BackgroundPrx prx = i == 0 ? background : (BackgroundPrx)background.ice_oneway();
            Ice.AsyncResult r = prx.begin_op();
            test(!r.sentSynchronously());
            try
            {
                prx.end_op(r);
                test(false);
            }
            catch(Ice.SocketException ex)
            {
            }
            test(r.isCompleted());
            configuration.readException(null);
        }

        if(!background.ice_getCommunicator().getProperties().getProperty("Ice.Default.Protocol").equals("test-ssl"))
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
                ex.printStackTrace();
                test(false);
            }
            background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

            try
            {
                // Get the read() of the connection validation to return "would block" and then throw.
                configuration.readReady(false);
                configuration.readException(new Ice.SocketException());
                background.op();
                test(false);
            }
            catch(Ice.SocketException ex)
            {
                configuration.readException(null);
                configuration.readReady(true);
            }

            for(int i = 0; i < 2; i++)
            {
                configuration.readReady(false);
                configuration.readException(new Ice.SocketException());
                Ice.AsyncResult r = background.begin_op();
                test(!r.sentSynchronously());
                try
                {
                    background.end_op(r);
                    test(false);
                }
                catch(Ice.SocketException ex)
                {
                }
                test(r.isCompleted());
                configuration.readException(null);
                configuration.readReady(true);
            }
        }

        ctl.holdAdapter(); // Hold to block in connection validation
        Ice.AsyncResult r = background.begin_op();
        Ice.AsyncResult r2 = background.begin_op();
        test(!r.sentSynchronously() && !r2.sentSynchronously());
        test(!r.isCompleted() && !r2.isCompleted());
        ctl.resumeAdapter();
        background.end_op(r);
        background.end_op(r2);
        test(r.isCompleted() && r2.isCompleted());

        try
        {
            // Get the write() of connection validation to throw right away.
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException ex)
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
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        try
        {
            // Get the write() of the connection validation to return "would block" and then throw.
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException ex)
        {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

        byte[] seq = new byte[512 * 1024];

        BackgroundPrx backgroundBatchOneway = BackgroundPrxHelper.uncheckedCast(background.ice_batchOneway());

        //
        // First send small requests to test without auto-flushing.
        //
        ctl.holdAdapter();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        ctl.resumeAdapter();
        backgroundBatchOneway.ice_flushBatchRequests();
        backgroundBatchOneway.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        //
        // Send bigger requests to test with auto-flushing.
        //
        ctl.holdAdapter();
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        ctl.resumeAdapter();
        backgroundBatchOneway.ice_flushBatchRequests();
        backgroundBatchOneway.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        //
        // Then try the same thing with async flush.
        //
        ctl.holdAdapter();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        ctl.resumeAdapter();
        backgroundBatchOneway.begin_ice_flushBatchRequests();
        backgroundBatchOneway.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);

        ctl.holdAdapter();
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        ctl.resumeAdapter();
        r = backgroundBatchOneway.begin_ice_flushBatchRequests();
        backgroundBatchOneway.end_ice_flushBatchRequests(r);
        backgroundBatchOneway.ice_getConnection().close(Ice.ConnectionClose.CloseGracefullyAndWait);
    }

    private static void
    readWriteTests(Configuration configuration, BackgroundPrx background, BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        for(int i = 0; i < 2; i++)
        {
            BackgroundPrx prx = i == 0 ? background : (BackgroundPrx)background.ice_oneway();

            try
            {
                background.ice_ping();
                configuration.writeException(new Ice.SocketException());
                prx.op();
                test(false);
            }
            catch(Ice.SocketException ex)
            {
                configuration.writeException(null);
            }

            background.ice_ping();
            configuration.writeException(new Ice.SocketException());
            Ice.AsyncResult r = prx.begin_op();
            test(!r.sentSynchronously());
            try
            {
                prx.end_op(r);
                test(false);
            }
            catch(Ice.SocketException ex)
            {
            }
            test(r.isCompleted());
            configuration.writeException(null);
        }

        try
        {
            background.ice_ping();
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException ex)
        {
            configuration.readException(null);
        }

        background.ice_ping();
        configuration.readReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
        configuration.readException(new Ice.SocketException());
        Ice.AsyncResult r = background.begin_op();
        try
        {
            background.end_op(r);
            test(false);
        }
        catch(Ice.SocketException ex)
        {
        }
        test(r.isCompleted());
        configuration.readException(null);
        configuration.readReady(true);

        try
        {
            background.ice_ping();
            configuration.writeReady(false);
            background.op();
            configuration.writeReady(true);
        }
        catch(Ice.LocalException ex)
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
        catch(Ice.LocalException ex)
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
        catch(Ice.SocketException ex)
        {
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        for(int i = 0; i < 2; ++i)
        {
            BackgroundPrx prx = i == 0 ? background : (BackgroundPrx)background.ice_oneway();

            background.ice_ping();
            configuration.writeReady(false);
            configuration.writeException(new Ice.SocketException());
            r = prx.begin_op();
            test(!r.sentSynchronously());
            try
            {
                prx.end_op(r);
                test(false);
            }
            catch(Ice.SocketException ex)
            {
            }
            test(r.isCompleted());
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        try
        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException ex)
        {
            configuration.readException(null);
            configuration.readReady(true);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new Ice.SocketException());
            r = background.begin_op();
            try
            {
                background.end_op(r);
                test(false);
            }
            catch(Ice.SocketException ex)
            {
            }
            test(r.isCompleted());
            configuration.readReady(true);
            configuration.readException(null);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.writeReady(false);
            configuration.readException(new Ice.SocketException());
            r = background.begin_op();
            // The read exception might propagate before the message send is seen as completed on IOCP.
            r.waitForSent();
            try
            {
                background.end_op(r);
                test(false);
            }
            catch(Ice.SocketException ex)
            {
            }
            test(r.isCompleted());
            configuration.writeReady(true);
            configuration.readReady(true);
            configuration.readException(null);
        }

        background.ice_ping(); // Establish the connection

        BackgroundPrx backgroundOneway = BackgroundPrxHelper.uncheckedCast(background.ice_oneway());
        test(backgroundOneway.ice_getConnection() == background.ice_getConnection());

        ctl.holdAdapter(); // Hold to block in request send.

        byte[] seq = new byte[10024];
        new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.

        // Fill up the receive and send buffers
        NoResponse noResponse = new NoResponse();
        for(int i = 0; i < 200; ++i) // 2MB
        {
            backgroundOneway.begin_opWithPayload(seq, noResponse);
        }

        OpAMICallback cb = new OpAMICallback();
        Ice.AsyncResult r1 = background.begin_op(cb);
        test(!r1.sentSynchronously() && !r1.isSent());

        OpAMICallback cb2 = new OpAMICallback();
        Ice.AsyncResult r2 = background.begin_op(cb2);
        test(!r2.sentSynchronously() && !r2.isSent());

        test(!backgroundOneway.begin_opWithPayload(seq, noResponse).sentSynchronously());
        test(!backgroundOneway.begin_opWithPayload(seq, noResponse).sentSynchronously());

        test(!cb.response(false));
        test(!cb2.response(false));
        ctl.resumeAdapter();
        cb.responseAndSent();
        cb2.responseAndSent();
        test(r1.isSent() && r1.isCompleted());
        test(r2.isSent() && r2.isCompleted());

        try
        {
            background.ice_ping();
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException ex)
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
        catch(Ice.ConnectionLostException ex)
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
        catch(Ice.LocalException ex)
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
        catch(Ice.LocalException ex)
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
        catch(Ice.ConnectionLostException ex)
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
        catch(Ice.ConnectionLostException ex)
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
            catch(Ice.LocalException ex)
            {
                test(false);
            }

            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            configuration.writeException(new Ice.SocketException());
            try
            {
                background.op();
            }
            catch(Ice.LocalException ex)
            {
            }
            configuration.writeException(null);

            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            background.ice_ping();
            background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);
            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            background.ice_getCachedConnection().close(Ice.ConnectionClose.CloseForcefully);
        }

        thread1._destroy();
        thread2._destroy();

        try
        {
            thread1.join();
            thread2.join();
        }
        catch(InterruptedException e)
        {
        }
    }
}
