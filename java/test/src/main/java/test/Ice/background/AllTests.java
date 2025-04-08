// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.DNSException;
import com.zeroc.Ice.InvocationFuture;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.LocatorPrx;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.RouterPrx;
import com.zeroc.Ice.SecurityException;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.SocketOperation;
import com.zeroc.Ice.TimeoutException;
import com.zeroc.Ice.Util;

import test.Ice.background.Test.BackgroundControllerPrx;
import test.Ice.background.Test.BackgroundPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static class Callback {
        Callback() {
            _called = false;
        }

        public synchronized void check() {
            while (!_called) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                }
            }
            _called = false;
        }

        public synchronized void called() {
            assert (!_called);
            _called = true;
            notify();
        }

        public synchronized boolean isCalled() {
            return _called;
        }

        private boolean _called;
    }

    private static class OpAMICallback {
        public void response() {
            _response.called();
        }

        public void exception(LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        public void sent(boolean ss) {
            _sent.called();
        }

        public boolean response(boolean wait) {
            if (wait) {
                _response.check();
                return true;
            } else {
                return _response.isCalled();
            }
        }

        public void responseAndSent() {
            _sent.check();
            _response.check();
        }

        private Callback _response = new Callback();
        private Callback _sent = new Callback();
    }

    static class OpThread extends Thread {
        OpThread(BackgroundPrx background) {
            _destroyed = false;
            _background = background.ice_oneway();
            start();
        }

        @Override
        public void run() {
            int count = 0;
            while (true) {
                synchronized (this) {
                    if (_destroyed) {
                        return;
                    }
                }

                try {
                    if (++count == 10) // Don't blast the connection with only oneway's
                        {
                            count = 0;
                            _background.ice_twoway().ice_ping();
                        }
                    _background.op();
                    try {
                        Thread.sleep(1);
                    } catch (InterruptedException ex) {
                    }
                } catch (LocalException ex) {
                }
            }
        }

        public synchronized void _destroy() // Thread.destroy is deprecated
            {
                _destroyed = true;
            }

        private boolean _destroyed;
        private BackgroundPrx _background;
    }

    public static BackgroundPrx allTests(Configuration configuration, TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        String sref = "background:" + helper.getTestEndpoint(0);
        ObjectPrx obj = communicator.stringToProxy(sref);
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
            LocatorPrx locator;
            obj =
                communicator
                    .stringToProxy("locator:" + helper.getTestEndpoint(0))
                    .ice_invocationTimeout(250);
            locator = LocatorPrx.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_locator(locator).ice_oneway();

            backgroundController.pauseCall("findAdapterById");
            try {
                obj.ice_ping();
                test(false);
            } catch (TimeoutException ex) {
            }
            backgroundController.resumeCall("findAdapterById");

            obj = communicator.stringToProxy("locator:" + helper.getTestEndpoint(0));
            locator = LocatorPrx.uncheckedCast(obj);
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
            RouterPrx router;

            obj =
                communicator
                    .stringToProxy("router:" + helper.getTestEndpoint(0))
                    .ice_invocationTimeout(250);
            router = RouterPrx.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router).ice_oneway();

            backgroundController.pauseCall("getClientProxy");
            try {
                obj.ice_ping();
                test(false);
            } catch (TimeoutException ex) {
            }
            backgroundController.resumeCall("getClientProxy");

            obj = communicator.stringToProxy("router:" + helper.getTestEndpoint(0));
            router = RouterPrx.uncheckedCast(obj);
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

        final boolean ws =
            "test-ws"
                .equals(communicator
                    .getProperties()
                    .getIceProperty("Ice.Default.Protocol"));
        final boolean wss =
            "test-wss"
                .equals(communicator
                    .getProperties()
                    .getIceProperty("Ice.Default.Protocol"));
        if (!ws && !wss) {
            out.print("testing buffered transport... ");
            out.flush();

            configuration.buffered(true);
            backgroundController.buffered(true);
            background.opAsync();
            background.ice_getCachedConnection().abort();
            background.opAsync();

            List<CompletableFuture<Void>> results = new ArrayList<>();
            for (int i = 0; i < 10000; i++) {
                CompletableFuture<Void> r = background.opAsync();
                results.add(r);
                if (i % 50 == 0) {
                    backgroundController.holdAdapter();
                    backgroundController.resumeAdapter();
                }
                if (i % 100 == 0) {
                    r.join();
                }
            }
            for (CompletableFuture<Void> r : results) {
                r.join();
            }

            out.println("ok");
        }

        return background;
    }

    private static void connectTests(Configuration configuration, BackgroundPrx background) {
        try {
            background.op();
        } catch (LocalException ex) {
            test(false);
        }
        background.ice_getConnection().close();

        for (int i = 0; i < 4; i++) {
            if (i == 0 || i == 2) {
                configuration.connectorsException(new DNSException("dummy"));
            } else {
                configuration.connectException(new SocketException());
            }
            BackgroundPrx prx = i == 1 || i == 3 ? background : background.ice_oneway();

            try {
                prx.op();
                test(false);
            } catch (LocalException ex) {
            }

            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try {
                r.join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof LocalException);
            }
            test(r.isDone());

            if (i == 0 || i == 2) {
                configuration.connectorsException(null);
            } else {
                configuration.connectException(null);
            }
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for (int i = 0; i < 5; i++) {
            try {
                background.ice_ping();
            } catch (LocalException ex) {
                test(false);
            }

            configuration.connectException(new SocketException());
            background.ice_getCachedConnection().abort();
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
            }
            configuration.connectException(null);
            try {
                background.ice_ping();
            } catch (LocalException ex) {
            }
        }

        thread1._destroy();
        thread2._destroy();

        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
        }
    }

    private static void initializeTests(
            Configuration configuration, BackgroundPrx background, BackgroundControllerPrx ctl) {
        try {
            background.op();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close();

        for (int i = 0; i < 4; i++) {
            if (i == 0 || i == 2) {
                configuration.initializeException(new SocketException());
            } else {
                configuration.initializeSocketStatus(SocketOperation.Write);
                configuration.initializeException(new SocketException());
            }
            BackgroundPrx prx = i == 1 || i == 3 ? background : background.ice_oneway();

            try {
                prx.op();
                test(false);
            } catch (SocketException ex) {
            }

            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try {
                r.join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof LocalException);
            }
            test(r.isDone());

            if (i == 0 || i == 2) {
                configuration.initializeException(null);
            } else {
                configuration.initializeSocketStatus(SocketOperation.None);
                configuration.initializeException(null);
            }
        }

        try {
            configuration.initializeSocketStatus(SocketOperation.Connect);
            background.op();
            configuration.initializeSocketStatus(SocketOperation.None);
        } catch (LocalException ex) {
            test(false);
        }
        background.ice_getConnection().close();

        try {
            configuration.initializeSocketStatus(SocketOperation.Write);
            background.op();
            configuration.initializeSocketStatus(SocketOperation.None);
        } catch (LocalException ex) {
            test(false);
        }
        background.ice_getConnection().close();

        try {
            configuration.initializeSocketStatus(SocketOperation.Write);
            configuration.initializeException(new SocketException());
            background.op();
            test(false);
        } catch (SocketException ex) {
            configuration.initializeException(null);
            configuration.initializeSocketStatus(SocketOperation.None);
        }

        //
        // Now run the same tests with the server side.
        //

        try {
            ctl.initializeException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
            ctl.initializeException(false);
        } catch (SecurityException ex) {
            ctl.initializeException(false);
        }

        try {
            ctl.initializeSocketStatus(SocketOperation.Write);
            background.op();
            ctl.initializeSocketStatus(SocketOperation.None);
        } catch (LocalException ex) {
            test(false);
        }
        background.ice_getConnection().close();

        try {
            ctl.initializeSocketStatus(SocketOperation.Write);
            ctl.initializeException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(SocketOperation.None);
        } catch (SecurityException ex) {
            ctl.initializeException(false);
            ctl.initializeSocketStatus(SocketOperation.None);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for (int i = 0; i < 5; i++) {
            try {
                background.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }

            configuration.initializeException(new SocketException());
            background.ice_getCachedConnection().abort();
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
            }
            configuration.initializeException(null);
            try {
                background.ice_ping();
            } catch (LocalException ex) {
            }
            try {
                background.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }

            configuration.initializeSocketStatus(SocketOperation.Write);
            background.ice_getCachedConnection().abort();

            try {
                background.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false); // Something's wrong with retries.
            }

            configuration.initializeSocketStatus(SocketOperation.None);

            ctl.initializeException(true);
            background.ice_getCachedConnection().abort();
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
            }
            ctl.initializeException(false);
            try {
                background.ice_ping();
            } catch (LocalException ex) {
            }
            try {
                background.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }

            try {
                ctl.initializeSocketStatus(SocketOperation.Write);
                background.ice_getCachedConnection().abort();
                background.op();
                ctl.initializeSocketStatus(SocketOperation.None);
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
        }

        thread1._destroy();
        thread2._destroy();

        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
        }
    }

    private static void validationTests(
            Configuration configuration, BackgroundPrx background, BackgroundControllerPrx ctl) {
        try {
            background.op();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close();

        try {
            // Get the read() of connection validation to throw right away.
            configuration.readException(new SocketException());
            background.op();
            test(false);
        } catch (SocketException ex) {
            configuration.readException(null);
        }

        for (int i = 0; i < 2; i++) {
            configuration.readException(new SocketException());
            BackgroundPrx prx = i == 0 ? background : background.ice_oneway();
            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try {
                r.join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof SocketException);
            }
            test(r.isDone());
            configuration.readException(null);
        }

        if (!"test-ssl"
            .equals(background
                .ice_getCommunicator()
                .getProperties()
                .getIceProperty("Ice.Default.Protocol"))) {
            try {
                // Get the read() of the connection validation to return "would block"
                configuration.readReady(false);
                background.op();
                configuration.readReady(true);
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            background.ice_getConnection().close();

            try {
                // Get the read() of the connection validation to return "would block" and then
                // throw.
                configuration.readReady(false);
                configuration.readException(new SocketException());
                background.op();
                test(false);
            } catch (SocketException ex) {
                configuration.readException(null);
                configuration.readReady(true);
            }

            for (int i = 0; i < 2; i++) {
                configuration.readReady(false);
                configuration.readException(new SocketException());
                CompletableFuture<Void> r = background.opAsync();
                InvocationFuture<Void> f = Util.getInvocationFuture(r);
                test(!f.sentSynchronously());
                try {
                    r.join();
                    test(false);
                } catch (CompletionException ex) {
                    test(ex.getCause() instanceof SocketException);
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

        try {
            // Get the write() of connection validation to throw right away.
            ctl.writeException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
            ctl.writeException(false);
        }

        try {
            // Get the write() of the connection validation to return "would block"
            ctl.writeReady(false);
            background.op();
            ctl.writeReady(true);
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }
        background.ice_getConnection().close();

        try {
            // Get the write() of the connection validation to return "would block" and then throw.
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
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
        backgroundBatchOneway.ice_getConnection().close();

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
        backgroundBatchOneway.ice_getConnection().close();

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
        backgroundBatchOneway.ice_getConnection().close();

        ctl.holdAdapter();
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        ctl.resumeAdapter();
        r = backgroundBatchOneway.ice_flushBatchRequestsAsync();
        r.join();
        backgroundBatchOneway.ice_getConnection().close();
    }

    private static void readWriteTests(
            Configuration configuration, BackgroundPrx background, BackgroundControllerPrx ctl) {
        try {
            background.op();
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        for (int i = 0; i < 2; i++) {
            BackgroundPrx prx = i == 0 ? background : background.ice_oneway();

            try {
                background.ice_ping();
                configuration.writeException(new SocketException());
                prx.op();
                test(false);
            } catch (SocketException ex) {
                configuration.writeException(null);
            }

            background.ice_ping();
            configuration.writeException(new SocketException());
            CompletableFuture<Void> r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try {
                r.join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof SocketException);
            }
            test(r.isDone());
            configuration.writeException(null);
        }

        try {
            background.ice_ping();
            configuration.readException(new SocketException());
            background.op();
            test(false);
        } catch (SocketException ex) {
            configuration.readException(null);
        }

        background.ice_ping();
        configuration.readReady(
            false); // Required in C# to make sure beginRead() doesn't throw too soon.
        configuration.readException(new SocketException());
        CompletableFuture<Void> r = background.opAsync();
        try {
            r.join();
            test(false);
        } catch (CompletionException ex) {
            test(ex.getCause() instanceof SocketException);
        }
        test(r.isDone());
        configuration.readException(null);
        configuration.readReady(true);

        try {
            background.ice_ping();
            configuration.writeReady(false);
            background.op();
            configuration.writeReady(true);
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            background.ice_ping();
            configuration.readReady(false);
            background.op();
            configuration.readReady(true);
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            background.ice_ping();
            configuration.writeReady(false);
            configuration.writeException(new SocketException());
            background.op();
            test(false);
        } catch (SocketException ex) {
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        for (int i = 0; i < 2; i++) {
            BackgroundPrx prx = i == 0 ? background : background.ice_oneway();

            background.ice_ping();
            configuration.writeReady(false);
            configuration.writeException(new SocketException());
            r = prx.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            test(!f.sentSynchronously());
            try {
                r.join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof SocketException);
            }
            test(r.isDone());
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        try {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new SocketException());
            background.op();
            test(false);
        } catch (SocketException ex) {
            configuration.readException(null);
            configuration.readReady(true);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new SocketException());
            r = background.opAsync();
            try {
                r.join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof SocketException);
            }
            test(r.isDone());
            configuration.readReady(true);
            configuration.readException(null);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.writeReady(false);
            configuration.readException(new SocketException());
            r = background.opAsync();
            InvocationFuture<Void> f = Util.getInvocationFuture(r);
            // The read exception might propagate before the message send is seen as completed on
            // IOCP.
            f.waitForSent();
            try {
                r.join();
                test(false);
            } catch (CompletionException ex) {
                test(ex.getCause() instanceof SocketException);
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
        new Random().nextBytes(seq); // Make sure the request doesn't compress too well.

        // Fill up the receive and send buffers
        for (int i = 0; i < 200; i++) // 2MB
            {
                backgroundOneway.opWithPayloadAsync(seq).whenComplete((result, ex) -> test(false));
            }

        OpAMICallback cb = new OpAMICallback();
        CompletableFuture<Void> r1 = background.opAsync();
        r1.whenComplete(
            (result, ex) -> {
                if (ex != null) {
                    cb.exception((LocalException) ex);
                } else {
                    cb.response();
                }
            });
        InvocationFuture<Void> f1 = Util.getInvocationFuture(r1);
        test(!f1.sentSynchronously() && !f1.isSent());
        f1.whenSent(
            (sentSynchronously, ex) -> {
                if (ex != null) {
                    cb.exception((LocalException) ex);
                } else {
                    cb.sent(sentSynchronously);
                }
            });

        OpAMICallback cb2 = new OpAMICallback();
        CompletableFuture<Void> r2 = background.opAsync();
        r2.whenComplete(
            (result, ex) -> {
                if (ex != null) {
                    cb2.exception((LocalException) ex);
                } else {
                    cb2.response();
                }
            });
        InvocationFuture<Void> f2 = Util.getInvocationFuture(r2);
        test(!f2.sentSynchronously() && !f2.isSent());
        f2.whenSent(
            (sentSynchronously, ex) -> {
                if (ex != null) {
                    cb2.exception((LocalException) ex);
                } else {
                    cb2.sent(sentSynchronously);
                }
            });

        test(
            !Util.getInvocationFuture(backgroundOneway.opWithPayloadAsync(seq))
                .sentSynchronously());
        test(
            !Util.getInvocationFuture(backgroundOneway.opWithPayloadAsync(seq))
                .sentSynchronously());

        test(!cb.response(false));
        test(!cb2.response(false));
        ctl.resumeAdapter();
        cb.responseAndSent();
        cb2.responseAndSent();
        test(f1.isSent() && r1.isDone());
        test(f2.isSent() && r2.isDone());

        try {
            background.ice_ping();
            ctl.writeException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
            ctl.writeException(false);
        }

        try {
            background.ice_ping();
            ctl.readException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
            ctl.readException(false);
        }

        try {
            background.ice_ping();
            ctl.writeReady(false);
            background.op();
            ctl.writeReady(true);
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            background.ice_ping();
            ctl.readReady(false);
            background.op();
            ctl.readReady(true);
        } catch (LocalException ex) {
            ex.printStackTrace();
            test(false);
        }

        try {
            background.ice_ping();
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

        try {
            background.ice_ping();
            ctl.readReady(false);
            ctl.readException(true);
            background.op();
            test(false);
        } catch (ConnectionLostException ex) {
            ctl.readException(false);
            ctl.readReady(true);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for (int i = 0; i < 5; i++) {
            try {
                background.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }

            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
            }
            configuration.writeException(new SocketException());
            try {
                background.op();
            } catch (LocalException ex) {
            }
            configuration.writeException(null);

            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
            }

            background.ice_ping();
            background.ice_getCachedConnection().abort();
            try {
                Thread.sleep(10);
            } catch (InterruptedException ex) {
            }

            background.ice_getCachedConnection().abort();
        }

        thread1._destroy();
        thread2._destroy();

        try {
            thread1.join();
            thread2.join();
        } catch (InterruptedException e) {
        }
    }

    private AllTests() {
    }
}
