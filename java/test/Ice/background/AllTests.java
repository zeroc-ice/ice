// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

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

        public synchronized boolean
        check(boolean wait)
        {
            if(wait)
            {
                while(!_called)
                {
                    try
                    {
                        wait(30000);
                    }
                    catch(InterruptedException ex)
                    {
                        continue;
                    }

                    if(!_called)
                    {
                        test(false);
                        return false; // Must be timeout.
                    }
                }

                _called = false;
                return true;
            }
            return _called;
        }

        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
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

    private static class OpAMICallback extends Test.AMI_Background_op implements Ice.AMISentCallback
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
            return response.check(wait);
        }

        public boolean
        responseAndSent(boolean wait)
        {
            if(sent.check(wait))
            {               
                return response.check(wait);
            }
            else
            {
                return false;
            }
        }

        private Callback response = new Callback();
        private Callback sent = new Callback();
    }

    private static class OpExAMICallback extends Test.AMI_Background_op implements Ice.AMISentCallback
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

        public boolean
        exception(boolean wait)
        {
            return exception.check(wait);
        }

        public boolean 
        sent(boolean wait)
        {
            return sent.check(wait);
        }

        private Callback exception = new Callback();
        private Callback sent = new Callback();
    }

    private static class OpWithPayloadOnewayAMICallback extends Test.AMI_Background_opWithPayload
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

    public static Test.BackgroundPrx
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
        String sref = "background:default -p 12010 -t 20000";
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundPrx background = BackgroundPrxHelper.uncheckedCast(obj);

        sref = "backgroundController:tcp -p 12011 -t 20000";
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundControllerPrx backgroundController = BackgroundControllerPrxHelper.uncheckedCast(obj);

        Configuration configuration = Configuration.getInstance();

        System.out.print("testing connect... ");
        System.out.flush();
        {
            connectTests(configuration, background);
        }
        System.out.println("ok");

        System.out.print("testing initialization... ");
        System.out.flush();
        {
            initializeTests(configuration, background, backgroundController);
        }
        System.out.println("ok");

        System.out.print("testing connection validation... ");
        System.out.flush();
        {
            validationTests(configuration, background, backgroundController);
        }
        System.out.println("ok");

        System.out.print("testing read/write... ");
        System.out.flush();
        {
            readWriteTests(configuration, background, backgroundController);
        }
        System.out.println("ok");

        System.out.print("testing locator... ");
        System.out.flush();
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

            obj = communicator.stringToProxy("locator:default -p 12010 -t 10000");
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
        System.out.println("ok");

        System.out.print("testing router... ");
        System.out.flush();
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

            obj = communicator.stringToProxy("router:default -p 12010 -t 10000");
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
        System.out.println("ok");

        return background;
    }

    private static void
    connectTests(Configuration configuration, Test.BackgroundPrx background)
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
        test(cbEx.exception(true));
        configuration.connectorsException(null);

        configuration.connectorsException(new Ice.DNSException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        test(cbEx.exception(true));
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
        test(cbEx.exception(true));
        configuration.connectException(null);

        configuration.connectException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        test(cbEx.exception(true));
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
    initializeTests(Configuration configuration, Test.BackgroundPrx background, Test.BackgroundControllerPrx ctl)
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
        test(cbEx.exception(true));
        configuration.initializeException(null);

        configuration.initializeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        test(cbEx.exception(true));
        configuration.initializeException(null);

        try
        {
            configuration.initializeSocketStatus(IceInternal.SocketStatus.NeedConnect);
            background.op();
            configuration.initializeSocketStatus(IceInternal.SocketStatus.Finished);
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(false);
        
        try
        {
            configuration.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite);
            background.op();
            configuration.initializeSocketStatus(IceInternal.SocketStatus.Finished);
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(false);

        try
        {
            configuration.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite);
            configuration.initializeException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch(Ice.SocketException ex)
        {
            configuration.initializeException(null);
            configuration.initializeSocketStatus(IceInternal.SocketStatus.Finished);
        }

        configuration.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite);
        configuration.initializeException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        test(cbEx.exception(true));
        configuration.initializeException(null);
        configuration.initializeSocketStatus(IceInternal.SocketStatus.Finished);

        configuration.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite);
        configuration.initializeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        test(cbEx.exception(true));
        configuration.initializeException(null);
        configuration.initializeSocketStatus(IceInternal.SocketStatus.Finished);

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
            ctl.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite.value());
            background.op();
            ctl.initializeSocketStatus(IceInternal.SocketStatus.Finished.value());
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(false);

        try
        {
            ctl.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite.value());
            ctl.initializeException(true);
            background.op();
            test(false);
        }
        catch(Ice.ConnectionLostException ex)
        {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(IceInternal.SocketStatus.Finished.value());
        }
        catch(Ice.SecurityException ex)
        {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(IceInternal.SocketStatus.Finished.value());
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

            configuration.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite);
            background.ice_getCachedConnection().close(true);
            background.ice_ping();
            configuration.initializeSocketStatus(IceInternal.SocketStatus.Finished);

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
                ctl.initializeSocketStatus(IceInternal.SocketStatus.NeedWrite.value());
                background.ice_getCachedConnection().close(true);
                background.op();
                ctl.initializeSocketStatus(IceInternal.SocketStatus.Finished.value());
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
    validationTests(Configuration configuration, Test.BackgroundPrx background, Test.BackgroundControllerPrx ctl)
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
        test(cbEx.exception(true));
        configuration.readException(null);

        configuration.readException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        test(cbEx.exception(true));
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
            test(cbEx.exception(true));
            configuration.readException(null);
            configuration.readReady(true);

            configuration.readReady(false);
            configuration.readException(new Ice.SocketException());
            test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
            test(cbEx.exception(true));
            configuration.readException(null);
            configuration.readReady(true);
        }

        ctl.holdAdapter(); // Hold to block in connection validation
        OpAMICallback cb = new OpAMICallback();
        test(!background.op_async(cb));
        OpAMICallback cb2 = new OpAMICallback();
        test(!background.op_async(cb2));
        test(!cb.responseAndSent(false));
        test(!cb2.responseAndSent(false));
        ctl.resumeAdapter();
        test(cb.responseAndSent(true));
        test(cb2.responseAndSent(true));

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
    readWriteTests(Configuration configuration, Test.BackgroundPrx background, Test.BackgroundControllerPrx ctl)
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
        test(cbEx.exception(true) && !cbEx.sent(false));
        configuration.writeException(null);

        configuration.writeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        test(cbEx.exception(true) && !cbEx.sent(false));
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
        test(cbEx.exception(true));
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
        test(cbEx.exception(true) && !cbEx.sent(false));
        configuration.writeException(null);
        configuration.writeReady(true);

        background.ice_ping();
        configuration.writeReady(false);
        configuration.writeException(new Ice.SocketException());
        test(!((BackgroundPrx)background.ice_oneway()).op_async(cbEx));
        test(cbEx.exception(true) && !cbEx.sent(false));
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
        test(cbEx.exception(true));
        configuration.readException(null);
        configuration.readReady(true);

        background.ice_ping();
        configuration.readReady(false);
        configuration.writeReady(false);
        configuration.readException(new Ice.SocketException());
        test(!background.op_async(cbEx));
        test(cbEx.sent(true));
        test(cbEx.exception(true));
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
        test(cb.responseAndSent(true));
        test(cb2.responseAndSent(true));

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
