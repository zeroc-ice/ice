// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.background;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.io.PrintWriter;

import com.zeroc.Ice.InvocationFuture;
import com.zeroc.Ice.Util;

import test.Ice.background.Test.BackgroundControllerPrx;
import test.Ice.background.Test.BackgroundPrx;

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

    private static class OpAMICallback
    {
        public void response()
        {
            _response.called();
        }

        public void exception(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

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

    static class OpThread extends Thread
    {
        OpThread(BackgroundPrx background)
        {
            _destroyed = false;
            _background = background.ice_oneway();
            start();
        }

        @Override
        public void run()
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
                catch(com.zeroc.Ice.LocalException ex)
                {
                }
            }
        }

        public synchronized void _destroy()                  // Thread.destroy is deprecated
        {
            _destroyed = true;
        }

        private boolean _destroyed;
        private BackgroundPrx _background = null;
    }

    public static BackgroundPrx allTests(Configuration configuration, test.TestHelper helper)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "background:" + helper.getTestEndpoint(0);
        com.zeroc.Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundPrx background = BackgroundPrx.uncheckedCast(obj);

        sref = "backgroundController:" + helper.getTestEndpoint(1, "tcp");
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundControllerPrx backgroundController = BackgroundControllerPrx.uncheckedCast(obj);

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
            com.zeroc.Ice.LocatorPrx locator;
            obj = communicator.stringToProxy("locator:" + helper.getTestEndpoint(0)).ice_invocationTimeout(250);
            locator = com.zeroc.Ice.LocatorPrx.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_locator(locator).ice_oneway();

            backgroundController.pauseCall("findAdapterById");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
            }
            backgroundController.resumeCall("findAdapterById");

            obj = communicator.stringToProxy("locator:" + helper.getTestEndpoint(0));
            locator = com.zeroc.Ice.LocatorPrx.uncheckedCast(obj);
            obj = obj.ice_locator(locator);
            obj.ice_ping();

            obj = communicator.stringToProxy("background@Test").ice_locator(locator);
            BackgroundPrx bg = BackgroundPrx.uncheckedCast(obj);

            backgroundController.pauseCall("findAdapterById");
            CompletableFuture<Void> r1 = bg.opAsync();
            CompletableFuture<Void> r2 = bg.opAsync();
            test(!r1.isDone());
            test(!r2.isDone());
            backgroundController.resumeCall("findAdapterById");
            r1.join();
            r2.join();
            test(r1.isDone());
            test(r2.isDone());
        }
        out.println("ok");

        out.print("testing router... ");
        out.flush();
        {
            com.zeroc.Ice.RouterPrx router;

            obj = communicator.stringToProxy("router:" + helper.getTestEndpoint(0)).ice_invocationTimeout(250);
            router = com.zeroc.Ice.RouterPrx.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router).ice_oneway();

            backgroundController.pauseCall("getClientProxy");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch(com.zeroc.Ice.TimeoutException ex)
            {
            }
            backgroundController.resumeCall("getClientProxy");

            obj = communicator.stringToProxy("router:" + helper.getTestEndpoint(0));
            router = com.zeroc.Ice.RouterPrx.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router);
            BackgroundPrx bg = BackgroundPrx.uncheckedCast(obj);
            test(bg.ice_getRouter() != null);

            backgroundController.pauseCall("getClientProxy");
            CompletableFuture<Void> r1 = bg.opAsync();
            CompletableFuture<Void> r2 = bg.opAsync();
            test(!r1.isDone());
            test(!r2.isDone());
            backgroundController.resumeCall("getClientProxy");
            r1.join();
            r2.join();
            test(r1.isDone());
            test(r2.isDone());
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
            background.opAsync();
            background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);
            background.opAsync();

            java.util.List<CompletableFuture<Void>> results = new java.util.ArrayList<>();
            for(int i = 0; i < 10000; ++i)
            {
                CompletableFuture<Void> r = background.opAsync();
                results.add(r);
                if(i % 50 == 0)
                {
                    backgroundController.holdAdapter();
                    backgroundController.resumeAdapter();
                }
                if(i % 100 == 0)
                {
                    r.join();
                }
            }
            for(CompletableFuture<Void> r : results)
            {
                r.join();
            }

            out.println("ok");
        }

        return background;
    }

    private static void connectTests(Configuration configuration, BackgroundPrx background)
    {
        try
        {
            background.op();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        for(int i = 0; i < 4; ++i)
        {
            if(i == 0 || i == 2)
            {
                configuration.connectorsException(new com.zeroc.Ice.DNSException());
            }
            else
            {
                configuration.connectException(new com.zeroc.Ice.SocketException());
            }
            BackgroundPrx prx = (i == 1 || i == 3) ? background : background.ice_oneway();

            try
            {
                prx.op();
                test(false);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
            }

            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try
            {
                r.join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.LocalException);
            }
            test(r.isDone());

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
            catch(com.zeroc.Ice.LocalException ex)
            {
                test(false);
            }

            configuration.connectException(new com.zeroc.Ice.SocketException());
            background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);
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
            catch(com.zeroc.Ice.LocalException ex)
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

    private static void initializeTests(Configuration configuration, BackgroundPrx background,
                                        BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        for(int i = 0; i < 4; i++)
        {
            if(i == 0 || i == 2)
            {
                configuration.initializeException(new com.zeroc.Ice.SocketException());
            }
            else
            {
                configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Write);
                configuration.initializeException(new com.zeroc.Ice.SocketException());
            }
            BackgroundPrx prx = (i == 1 || i == 3) ? background : background.ice_oneway();

            try
            {
                prx.op();
                test(false);
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
            }

            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try
            {
                r.join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.LocalException);
            }
            test(r.isDone());

            if(i == 0 || i == 2)
            {
                configuration.initializeException(null);
            }
            else
            {
                configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
                configuration.initializeException(null);
            }
        }

        try
        {
            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Connect);
            background.op();
            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        try
        {
            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Write);
            background.op();
            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        try
        {
            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Write);
            configuration.initializeException(new com.zeroc.Ice.SocketException());
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.SocketException ex)
        {
            configuration.initializeException(null);
            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
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
        catch(com.zeroc.Ice.ConnectionLostException ex)
        {
            ctl.initializeException(false);
        }
        catch(com.zeroc.Ice.SecurityException ex)
        {
            ctl.initializeException(false);
        }

        try
        {
            ctl.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Write);
            background.op();
            ctl.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
        background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        try
        {
            ctl.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Write);
            ctl.initializeException(true);
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.ConnectionLostException ex)
        {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
        }
        catch(com.zeroc.Ice.SecurityException ex)
        {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for(int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false);
            }

            configuration.initializeException(new com.zeroc.Ice.SocketException());
            background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);
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
            catch(com.zeroc.Ice.LocalException ex)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false);
            }

            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Write);
            background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);

            try
            {
                background.ice_ping();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false); // Something's wrong with retries.
            }

            configuration.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);

            ctl.initializeException(true);
            background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);
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
            catch(com.zeroc.Ice.LocalException ex)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false);
            }

            try
            {
                ctl.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.Write);
                background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);
                background.op();
                ctl.initializeSocketStatus(com.zeroc.IceInternal.SocketOperation.None);
            }
            catch(com.zeroc.Ice.LocalException ex)
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

    private static void validationTests(Configuration configuration, BackgroundPrx background,
                                        BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        try
        {
            // Get the read() of connection validation to throw right away.
            configuration.readException(new com.zeroc.Ice.SocketException());
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.SocketException ex)
        {
            configuration.readException(null);
        }

        for(int i = 0; i < 2; i++)
        {
            configuration.readException(new com.zeroc.Ice.SocketException());
            BackgroundPrx prx = i == 0 ? background : background.ice_oneway();
            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try
            {
                r.join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.SocketException);
            }
            test(r.isDone());
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
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false);
            }
            background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

            try
            {
                // Get the read() of the connection validation to return "would block" and then throw.
                configuration.readReady(false);
                configuration.readException(new com.zeroc.Ice.SocketException());
                background.op();
                test(false);
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
                configuration.readException(null);
                configuration.readReady(true);
            }

            for(int i = 0; i < 2; i++)
            {
                configuration.readReady(false);
                configuration.readException(new com.zeroc.Ice.SocketException());
                CompletableFuture<Void> r = background.opAsync();
                InvocationFuture<Void> f = Util.getInvocationFuture(r);
                test(!f.sentSynchronously());
                try
                {
                    r.join();
                    test(false);
                }
                catch(CompletionException ex)
                {
                    test(ex.getCause() instanceof com.zeroc.Ice.SocketException);
                }
                test(r.isDone());
                configuration.readException(null);
                configuration.readReady(true);
            }
        }

        ctl.holdAdapter(); // Hold to block in connection validation
        CompletableFuture<Void> r = background.opAsync();
        CompletableFuture<Void> r2 = background.opAsync();
        InvocationFuture<Void> f = Util.getInvocationFuture(r);
        InvocationFuture<Void> f2 = Util.getInvocationFuture(r2);
        test(!f.sentSynchronously() && !f2.sentSynchronously());
        test(!r.isDone() && !r2.isDone());
        ctl.resumeAdapter();
        r.join();
        r2.join();
        test(r.isDone() && r2.isDone());

        try
        {
            // Get the write() of connection validation to throw right away.
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.ConnectionLostException ex)
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
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        try
        {
            // Get the write() of the connection validation to return "would block" and then throw.
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.ConnectionLostException ex)
        {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

        byte[] seq = new byte[512 * 1024];

        BackgroundPrx backgroundBatchOneway = background.ice_batchOneway();

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
        backgroundBatchOneway.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

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
        backgroundBatchOneway.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        //
        // Then try the same thing with async flush.
        //
        ctl.holdAdapter();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        ctl.resumeAdapter();
        backgroundBatchOneway.ice_flushBatchRequestsAsync();
        backgroundBatchOneway.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);

        ctl.holdAdapter();
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        ctl.resumeAdapter();
        r = backgroundBatchOneway.ice_flushBatchRequestsAsync();
        r.join();
        backgroundBatchOneway.ice_getConnection().close(com.zeroc.Ice.ConnectionClose.GracefullyWithWait);
    }

    private static void readWriteTests(Configuration configuration, BackgroundPrx background,
                                       BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        for(int i = 0; i < 2; i++)
        {
            BackgroundPrx prx = i == 0 ? background : background.ice_oneway();

            try
            {
                background.ice_ping();
                configuration.writeException(new com.zeroc.Ice.SocketException());
                prx.op();
                test(false);
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
                configuration.writeException(null);
            }

            background.ice_ping();
            configuration.writeException(new com.zeroc.Ice.SocketException());
            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try
            {
                r.join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.SocketException);
            }
            test(r.isDone());
            configuration.writeException(null);
        }

        try
        {
            background.ice_ping();
            configuration.readException(new com.zeroc.Ice.SocketException());
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.SocketException ex)
        {
            configuration.readException(null);
        }

        background.ice_ping();
        configuration.readReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
        configuration.readException(new com.zeroc.Ice.SocketException());
        CompletableFuture<Void> r = background.opAsync();
        try
        {
            r.join();
            test(false);
        }
        catch(CompletionException ex)
        {
            test(ex.getCause() instanceof com.zeroc.Ice.SocketException);
        }
        test(r.isDone());
        configuration.readException(null);
        configuration.readReady(true);

        try
        {
            background.ice_ping();
            configuration.writeReady(false);
            background.op();
            configuration.writeReady(true);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            background.ice_ping();
            configuration.readReady(false);
            background.op();
            configuration.readReady(true);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            background.ice_ping();
            configuration.writeReady(false);
            configuration.writeException(new com.zeroc.Ice.SocketException());
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.SocketException ex)
        {
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        for(int i = 0; i < 2; ++i)
        {
            BackgroundPrx prx = i == 0 ? background : background.ice_oneway();

            background.ice_ping();
            configuration.writeReady(false);
            configuration.writeException(new com.zeroc.Ice.SocketException());
            r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try
            {
                r.join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.SocketException);
            }
            test(r.isDone());
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        try
        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new com.zeroc.Ice.SocketException());
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.SocketException ex)
        {
            configuration.readException(null);
            configuration.readReady(true);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new com.zeroc.Ice.SocketException());
            r = background.opAsync();
            try
            {
                r.join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.SocketException);
            }
            test(r.isDone());
            configuration.readReady(true);
            configuration.readException(null);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.writeReady(false);
            configuration.readException(new com.zeroc.Ice.SocketException());
            r = background.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            // The read exception might propagate before the message send is seen as completed on IOCP.
            f.waitForSent();
            try
            {
                r.join();
                test(false);
            }
            catch(CompletionException ex)
            {
                test(ex.getCause() instanceof com.zeroc.Ice.SocketException);
            }
            test(r.isDone());
            configuration.writeReady(true);
            configuration.readReady(true);
            configuration.readException(null);
        }

        background.ice_ping(); // Establish the connection

        BackgroundPrx backgroundOneway = background.ice_oneway();
        test(backgroundOneway.ice_getConnection() == background.ice_getConnection());

        ctl.holdAdapter(); // Hold to block in request send.

        byte[] seq = new byte[10024];
        new java.util.Random().nextBytes(seq); // Make sure the request doesn't compress too well.

        // Fill up the receive and send buffers
        for(int i = 0; i < 200; ++i) // 2MB
        {
            backgroundOneway.opWithPayloadAsync(seq).whenComplete((result, ex) -> test(false));
        }

        OpAMICallback cb = new OpAMICallback();
        CompletableFuture<Void> r1 = background.opAsync();
        r1.whenComplete((result, ex) ->
            {
                if(ex != null)
                {
                    cb.exception((com.zeroc.Ice.LocalException)ex);
                }
                else
                {
                    cb.response();
                }
            });
        InvocationFuture<Void> f1 = Util.getInvocationFuture(r1);
        test(!f1.sentSynchronously() && !f1.isSent());
        f1.whenSent((sentSynchronously, ex) ->
            {
                if(ex != null)
                {
                    cb.exception((com.zeroc.Ice.LocalException)ex);
                }
                else
                {
                    cb.sent(sentSynchronously);
                }
            });

        OpAMICallback cb2 = new OpAMICallback();
        CompletableFuture<Void> r2 = background.opAsync();
        r2.whenComplete((result, ex) ->
            {
                if(ex != null)
                {
                    cb2.exception((com.zeroc.Ice.LocalException)ex);
                }
                else
                {
                    cb2.response();
                }
            });
        InvocationFuture<Void> f2 = Util.getInvocationFuture(r2);
        test(!f2.sentSynchronously() && !f2.isSent());
        f2.whenSent((sentSynchronously, ex) ->
            {
                if(ex != null)
                {
                    cb2.exception((com.zeroc.Ice.LocalException)ex);
                }
                else
                {
                    cb2.sent(sentSynchronously);
                }
            });

        test(!Util.getInvocationFuture(backgroundOneway.opWithPayloadAsync(seq)).sentSynchronously());
        test(!Util.getInvocationFuture(backgroundOneway.opWithPayloadAsync(seq)).sentSynchronously());

        test(!cb.response(false));
        test(!cb2.response(false));
        ctl.resumeAdapter();
        cb.responseAndSent();
        cb2.responseAndSent();
        test(f1.isSent() && r1.isDone());
        test(f2.isSent() && r2.isDone());

        try
        {
            background.ice_ping();
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch(com.zeroc.Ice.ConnectionLostException ex)
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
        catch(com.zeroc.Ice.ConnectionLostException ex)
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
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
            test(false);
        }

        try
        {
            background.ice_ping();
            ctl.readReady(false);
            background.op();
            ctl.readReady(true);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            ex.printStackTrace();
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
        catch(com.zeroc.Ice.ConnectionLostException ex)
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
        catch(com.zeroc.Ice.ConnectionLostException ex)
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
            catch(com.zeroc.Ice.LocalException ex)
            {
                ex.printStackTrace();
                test(false);
            }

            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }
            configuration.writeException(new com.zeroc.Ice.SocketException());
            try
            {
                background.op();
            }
            catch(com.zeroc.Ice.LocalException ex)
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
            background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);
            try
            {
                Thread.sleep(10);
            }
            catch(java.lang.InterruptedException ex)
            {
            }

            background.ice_getCachedConnection().close(com.zeroc.Ice.ConnectionClose.Forcefully);
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
