// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

import java.io.PrintStream;
import java.io.PrintWriter;

import test.Ice.background.Test.AMI_Background_op;
import test.Ice.background.Test.AMI_Background_opWithPayload;
import test.Ice.background.Test.BackgroundControllerPrx;
import test.Ice.background.Test.BackgroundControllerPrxHelper;
import test.Ice.background.Test.BackgroundPrx;
import test.Ice.background.Test.BackgroundPrxHelper;

public class AllTests
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

        public synchronized void
        check()
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

        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        public synchronized boolean
        isCalled()
        {
            return _called;
        }

        private boolean _called;
    }

    static class OpThread extends  Thread
    {
        OpThread(BackgroundPrx background)
        {
            _background = BackgroundPrxHelper.uncheckedCast(background.ice_oneway());
            start();
        }

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

        private boolean _destroyed = false;
        private BackgroundPrx _background = null;
    }

    private static class OpAMICallback extends AMI_Background_op implements Ice.AMISentCallback
    {
        public void
        ice_response()
        {
            response.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        public void
        ice_sent()
        {
            sent.called();
        }

        public boolean
        response(boolean wait)
        {
            if(wait)
            {
                response.check();
                return true;
            }
            else
            {
                return response.isCalled();
            }
        }

        public void
        responseAndSent()
        {
            sent.check();
            response.check();
        }

        private Callback response = new Callback();
        private Callback sent = new Callback();
    }

    private static class OpExAMICallback extends AMI_Background_op implements Ice.AMISentCallback
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            exception.called();
        }

        public void
        ice_sent()
        {
            sent.called();
        }

        public void
        exception()
        {
            exception.check();
        }

        public boolean 
        sent(boolean wait)
        {
            if(wait)
            {
                sent.check();
                return true;
            }
            else
            {
                return sent.isCalled();
            }
        }

        private Callback exception = new Callback();
        private Callback sent = new Callback();
    }

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
    };

    public static BackgroundPrx
    allTests(Configuration configuration, Ice.Communicator communicator, PrintWriter out)
    {
        String sref = "background:default -p 12010";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundPrx background = BackgroundPrxHelper.uncheckedCast(obj);

        sref = "backgroundController:tcp -p 12011";
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
            obj = communicator.stringToProxy("locator:default -p 12010 -t 500");
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
            test(cb.response(true));
            test(cb2.response(true));
        }
        out.println("ok");

        out.print("testing router... ");
        out.flush();
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
            catch(Ice.TimeoutException ex)
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
            test(cb.response(true));
            test(cb2.response(true));
        }
        out.println("ok");

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
        background.ice_getConnection().close(false);

        OpExAMICallback cbEx = new OpExAMICallback();

        try
        {
            configuration.connectorsException(new Ice.DNSException());
            background.op();
            test(false);
        }
        catch(Ice.DNSException ex)
        {
            configuration.connectorsException(null);
        }

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
        catch(Ice.SocketException ex)
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
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            
            configuration.connectException(new Ice.SocketException());
            background.ice_getCachedConnection().close(true);
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
        background.ice_getConnection().close(false);

        try
        {
            configuration.initializeException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException ex)
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
        background.ice_getConnection().close(false);
        
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
        background.ice_getConnection().close(false);

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

        configuration.initializeSocketStatus(IceInternal.SocketOperation.Write);
        configuration.initializeException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        configuration.initializeException(null);
        configuration.initializeSocketStatus(IceInternal.SocketOperation.None);

        configuration.initializeSocketStatus(IceInternal.SocketOperation.Write);
        configuration.initializeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        configuration.initializeException(null);
        configuration.initializeSocketStatus(IceInternal.SocketOperation.None);

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
        background.ice_getConnection().close(false);

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
            background.ice_getCachedConnection().close(true);
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
            background.ice_getCachedConnection().close(true);
            background.ice_ping();
            configuration.initializeSocketStatus(IceInternal.SocketOperation.None);

            ctl.initializeException(true);
            background.ice_getCachedConnection().close(true);
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
                background.ice_getCachedConnection().close(true);
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
        background.ice_getConnection().close(false);

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

        OpExAMICallback cbEx = new OpExAMICallback();

        configuration.readException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        cbEx.exception();
        configuration.readException(null);

        configuration.readException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        cbEx.exception();
        configuration.readException(null);

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
            background.ice_getConnection().close(false);

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
        background.ice_getConnection().close(false);

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
        backgroundBatchOneway.ice_ping();
        backgroundBatchOneway.ice_getConnection().close(false);
        try
        {
            backgroundBatchOneway.ice_ping();
            test(false);
        }
        catch(Ice.CloseConnectionException ex)
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
        catch(Ice.CloseConnectionException ex)
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
        catch(Ice.CloseConnectionException ex)
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
        catch(Ice.CloseConnectionException ex)
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

        try
        {
            background.ice_ping();
            configuration.writeException(new Ice.SocketException()); 
            background.op();
            test(false);
        }
        catch(Ice.SocketException ex)
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
        catch(Ice.SocketException ex)
        {
            configuration.readException(null);
        }

        background.ice_ping();
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
        catch(Ice.SocketException ex)
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
        new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.
        while(backgroundOneway.opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq))
        {
        }
        OpAMICallback cb = new OpAMICallback();
        test(!background.op_async(cb));
        OpAMICallback cb2 = new OpAMICallback();
        test(!background.op_async(cb2));
        test(!backgroundOneway.opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq));
        test(!backgroundOneway.opWithPayload_async(new OpWithPayloadOnewayAMICallback(), seq));
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
            background.ice_getCachedConnection().close(true);
            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            background.ice_getCachedConnection().close(true);
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
