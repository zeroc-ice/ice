//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

public class AllTests
{
    private static void test(bool b)
    {
        if (!b)
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
            lock (this)
            {
                while (!_called)
                {
                    System.Threading.Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public virtual void called()
        {
            lock (this)
            {
                Debug.Assert(!_called);
                _called = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        public virtual bool isCalled()
        {
            lock (this)
            {
                return _called;
            }
        }

        private bool _called;
    }

    private class OpAMICallback
    {
        public void response()
        {
            _response.called();
        }

        public void responseNoOp()
        {
        }

        public void noResponse()
        {
            test(false);
        }

        public void exception(Ice.Exception ex)
        {
            _response.called();
        }

        public void noException(Ice.Exception ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }

        public void sent(bool ss)
        {
            _sent.called();
        }

        public bool checkException(bool wait)
        {
            if (wait)
            {
                _response.check();
                return true;
            }
            else
            {
                return _response.isCalled();
            }
        }

        public bool checkResponse(bool wait)
        {
            if (wait)
            {
                _response.check();
                return true;
            }
            else
            {
                return _response.isCalled();
            }
        }

        public void checkResponseAndSent()
        {
            _sent.check();
            _response.check();
        }

        private Callback _response = new Callback();
        private Callback _sent = new Callback();
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
            while (true)
            {
                lock (this)
                {
                    if (_destroyed)
                    {
                        return;
                    }
                }

                try
                {
                    if (++count == 10) // Don't blast the connection with only oneway's
                    {
                        count = 0;
                        _background.ice_twoway().ice_ping();
                    }
                    _background.opAsync();
                    Thread.Sleep(1);
                }
                catch (Ice.LocalException)
                {
                }
            }
        }

        public void destroy()
        {
            lock (this)
            {
                _destroyed = true;
            }
        }

        private bool _destroyed = false;
        private BackgroundPrx _background = null;
        private Thread _thread;
    }

    public static Test.BackgroundPrx allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        string sref = "background:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        BackgroundPrx background = BackgroundPrxHelper.uncheckedCast(obj);

        sref = "backgroundController:" + helper.getTestEndpoint(1, "tcp");
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
            obj = communicator.stringToProxy("locator:" + helper.getTestEndpoint(0)).ice_invocationTimeout(250);
            locator = Ice.LocatorPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_locator(locator).ice_oneway();

            backgroundController.pauseCall("findAdapterById");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch (Ice.TimeoutException)
            {
            }
            backgroundController.resumeCall("findAdapterById");

            obj = communicator.stringToProxy("locator:" + helper.getTestEndpoint(0));
            locator = Ice.LocatorPrxHelper.uncheckedCast(obj);
            obj = obj.ice_locator(locator);
            obj.ice_ping();

            obj = communicator.stringToProxy("background@Test").ice_locator(locator);
            BackgroundPrx bg = BackgroundPrxHelper.uncheckedCast(obj);

            backgroundController.pauseCall("findAdapterById");
            var t1 = bg.opAsync();
            var t2 = bg.opAsync();
            test(!t1.IsCompleted);
            test(!t2.IsCompleted);
            backgroundController.resumeCall("findAdapterById");
            t1.Wait();
            t2.Wait();
            test(t1.IsCompleted);
            test(t2.IsCompleted);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing router... ");
        Console.Out.Flush();
        {
            Ice.RouterPrx router;

            obj = communicator.stringToProxy("router:" + helper.getTestEndpoint(0)).ice_invocationTimeout(250);
            router = Ice.RouterPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router).ice_oneway();

            backgroundController.pauseCall("getClientProxy");
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch (Ice.TimeoutException)
            {
            }
            backgroundController.resumeCall("getClientProxy");

            obj = communicator.stringToProxy("router:" + helper.getTestEndpoint(0));
            router = Ice.RouterPrxHelper.uncheckedCast(obj);
            obj = communicator.stringToProxy("background@Test").ice_router(router);
            BackgroundPrx bg = BackgroundPrxHelper.uncheckedCast(obj);
            test(bg.ice_getRouter() != null);

            backgroundController.pauseCall("getClientProxy");
            var t1 = bg.opAsync();
            var t2 = bg.opAsync();
            test(!t1.IsCompleted);
            test(!t2.IsCompleted);
            backgroundController.resumeCall("getClientProxy");
            t1.Wait();
            t2.Wait();
            test(t1.IsCompleted);
            test(t2.IsCompleted);
        }
        Console.Out.WriteLine("ok");

        bool ws = communicator.getProperties().getProperty("Ice.Default.Protocol").Equals("test-ws");
        bool wss = communicator.getProperties().getProperty("Ice.Default.Protocol").Equals("test-wss");
        if (!ws && !wss)
        {
            Console.Write("testing buffered transport... ");
            Console.Out.Flush();

            configuration.buffered(true);
            backgroundController.buffered(true);
            background.opAsync();
            background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
            background.opAsync();

            OpAMICallback cb = new OpAMICallback();
            var results = new List<Task>();
            for (int i = 0; i < 10000; ++i)
            {
                var t = background.opAsync().ContinueWith((Task p) =>
                {
                    try
                    {
                        p.Wait();
                        cb.responseNoOp();
                    }
                    catch (Ice.Exception ex)
                    {
                        cb.noException(ex);
                    }
                });
                results.Add(t);
                if (i % 50 == 0)
                {
                    backgroundController.holdAdapter();
                    backgroundController.resumeAdapter();
                }
                if (i % 100 == 0)
                {
                    t.Wait();
                }
            }
            Task.WaitAll(results.ToArray());
            Console.Out.WriteLine("ok");
        }

        return background;
    }

    private static void connectTests(Configuration configuration, Test.BackgroundPrx background)
    {
        try
        {
            background.op();
        }
        catch (Ice.LocalException ex)
        {
            System.Console.Out.WriteLine(ex);
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

        for (int i = 0; i < 4; ++i)
        {
            if (i == 0 || i == 2)
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
            catch (Ice.Exception)
            {
            }

            var sentSynchronously = false;
            var t = prx.opAsync(progress: new Progress<bool>(value =>
            {
                sentSynchronously = value;
            }));
            test(!sentSynchronously);
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.Exception)
            {
            }
            test(t.IsCompleted);

            OpAMICallback cbEx = new OpAMICallback();
            t = prx.opAsync(progress: new Progress<bool>(value =>
            {
                sentSynchronously = value;
            }));
            test(!sentSynchronously);

            t.ContinueWith((Task p) =>
            {
                try
                {
                    p.Wait();
                }
                catch (AggregateException ex) when (ex.InnerException is Ice.Exception)
                {
                    cbEx.exception(ex.InnerException as Ice.Exception);
                }
            });
            cbEx.checkException(true);
            test(t.IsCompleted);

            if (i == 0 || i == 2)
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
        try
        {
            for (int i = 0; i < 5; i++)
            {
                try
                {
                    background.ice_ping();
                }
                catch (Ice.LocalException)
                {
                    test(false);
                }

                configuration.connectException(new Ice.SocketException());
                background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
                Thread.Sleep(10);
                configuration.connectException(null);
                try
                {
                    background.ice_ping();
                }
                catch (Ice.LocalException)
                {
                }
            }
        }
        catch (Exception ex)
        {
            System.Console.Out.WriteLine(ex);
            test(false);
        }
        finally
        {
            thread1.destroy();
            thread2.destroy();

            thread1.Join();
            thread2.Join();
        }
    }

    private static void initializeTests(Configuration configuration, Test.BackgroundPrx background,
                                        Test.BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
        background.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);

        for (int i = 0; i < 4; ++i)
        {
            if (i == 0 || i == 2)
            {
                configuration.initializeException(new Ice.SocketException());
            }
            else
            {
                continue;
            }
            BackgroundPrx prx = (i == 1 || i == 3) ? background : (BackgroundPrx)background.ice_oneway();

            try
            {
                prx.op();
                test(false);
            }
            catch (Ice.SocketException)
            {
            }

            bool sentSynchronously = false;
            var t = prx.opAsync(progress: new Progress<bool>(value =>
            {
                sentSynchronously = value;
            }));
            test(!sentSynchronously);
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.Exception)
            {
            }
            test(t.IsCompleted);

            OpAMICallback cbEx = new OpAMICallback();
            t = prx.opAsync(progress: new Progress<bool>(value =>
            {
                sentSynchronously = false;
            }));
            test(!sentSynchronously);
            try
            {
                t.Wait();
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.Exception)
            {
                cbEx.exception(ex.InnerException as Ice.Exception);
            }
            cbEx.checkException(true);
            test(t.IsCompleted);

            if (i == 0 || i == 2)
            {
                configuration.initializeException(null);
            }
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
        catch (Ice.ConnectionLostException)
        {
            ctl.initializeException(false);
        }
        catch (Ice.SecurityException)
        {
            ctl.initializeException(false);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for (int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch (Ice.LocalException)
            {
                test(false);
            }

            configuration.initializeException(new Ice.SocketException());
            background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
            Thread.Sleep(10);
            configuration.initializeException(null);
            try
            {
                background.ice_ping();
            }
            catch (Ice.LocalException)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch (Ice.LocalException)
            {
                test(false);
            }

            background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
            background.ice_ping();

            ctl.initializeException(true);
            background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
            Thread.Sleep(10);
            ctl.initializeException(false);
            try
            {
                background.ice_ping();
            }
            catch (Ice.LocalException)
            {
            }
            try
            {
                background.ice_ping();
            }
            catch (Ice.LocalException)
            {
                test(false);
            }

            try
            {
                background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
                background.op();
            }
            catch (Ice.LocalException)
            {
                test(false);
            }
        }

        thread1.destroy();
        thread2.destroy();

        thread1.Join();
        thread2.Join();
    }

    private sealed class CloseCallback : Callback
    {

        public void closed(Ice.Connection con)
        {
            called();
        }
    }

    //
    // Close the connection associated with the proxy and wait until the close completes.
    //
    private static void closeConnection(Ice.ObjectPrx prx)
    {
        CloseCallback cb = new CloseCallback();
        prx.ice_getConnection().setCloseCallback(cb.closed);
        prx.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
        cb.check();
    }

    private static void validationTests(Configuration configuration, Test.BackgroundPrx background,
                                        Test.BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
        closeConnection(background);

        try
        {
            // Get the read() of connection validation to throw right away.
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch (Ice.SocketException)
        {
            configuration.readException(null);
        }

        for (int i = 0; i < 2; ++i)
        {
            configuration.readException(new Ice.SocketException());
            BackgroundPrx prx = i == 0 ? background : (BackgroundPrx)background.ice_oneway();
            bool sentSynchronously = false;
            var t = prx.opAsync(progress: new Progress<bool>(value =>
            {
                sentSynchronously = value;
            }));
            test(!sentSynchronously);
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.SocketException)
            {
            }
            test(t.IsCompleted);
            configuration.readException(null);
        }

        if (!background.ice_getCommunicator().getProperties().getProperty("Ice.Default.Protocol").Equals("test-ssl") &&
           !background.ice_getCommunicator().getProperties().getProperty("Ice.Default.Protocol").Equals("test-wss"))
        {
            try
            {
                // Get the read() of the connection validation to return "would block"
                configuration.readReady(false);
                background.op();
                configuration.readReady(true);
            }
            catch (Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                test(false);
            }
            closeConnection(background);

            try
            {
                // Get the read() of the connection validation to return "would block" and then throw.
                configuration.readReady(false);
                configuration.readException(new Ice.SocketException());
                background.op();
                test(false);
            }
            catch (Ice.SocketException)
            {
                configuration.readException(null);
                configuration.readReady(true);
            }

            for (int i = 0; i < 2; ++i)
            {
                configuration.readReady(false);
                configuration.readException(new Ice.SocketException());
                var sentSynchronously = false;
                var t = background.opAsync(progress: new Progress<bool>(value =>
                {
                    sentSynchronously = value;
                }));
                test(!sentSynchronously);
                try
                {
                    t.Wait();
                    test(false);
                }
                catch (AggregateException ex) when (ex.InnerException is Ice.SocketException)
                {
                }
                test(t.IsCompleted);
                configuration.readException(null);
                configuration.readReady(true);
            }
        }

        {
            ctl.holdAdapter(); // Hold to block in connection validation
            var t1SentSynchronously = false;
            var t2SentSynchronously = false;
            var t1 = background.opAsync(progress: new Progress<bool>(value =>
            {
                t1SentSynchronously = value;
            }));
            var t2 = background.opAsync(progress: new Progress<bool>(value =>
            {
                t2SentSynchronously = value;
            }));
            test(!t1SentSynchronously && !t2SentSynchronously);
            test(!t1.IsCompleted && !t2.IsCompleted);
            ctl.resumeAdapter();
            t1.Wait();
            t2.Wait();
            test(t1.IsCompleted && t2.IsCompleted);
        }

        try
        {
            // Get the write() of connection validation to throw right away.
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch (Ice.ConnectionLostException)
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
        catch (Ice.LocalException ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }
        closeConnection(background);

        try
        {
            // Get the write() of the connection validation to return "would block" and then throw.
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch (Ice.ConnectionLostException)
        {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

    }

    private static void readWriteTests(Configuration configuration, Test.BackgroundPrx background,
                                       Test.BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (Ice.LocalException ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }

        for (int i = 0; i < 2; ++i)
        {
            BackgroundPrx prx = i == 0 ? background : (BackgroundPrx)background.ice_oneway();

            try
            {
                prx.ice_ping();
                configuration.writeException(new Ice.SocketException());
                prx.op();
                test(false);
            }
            catch (Ice.SocketException)
            {
                configuration.writeException(null);
            }

            background.ice_ping();
            configuration.writeException(new Ice.SocketException());
            var sentSynchronously = false;
            var t = prx.opAsync(progress: new Progress<bool>(value =>
            {
                sentSynchronously = value;
            }));
            test(!sentSynchronously);
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.SocketException)
            {
            }
            test(t.IsCompleted);
            configuration.writeException(null);
        }

        try
        {
            background.ice_ping();
            configuration.readException(new Ice.SocketException());
            background.op();
            test(false);
        }
        catch (Ice.SocketException)
        {
            configuration.readException(null);
        }

        {
            background.ice_ping();
            configuration.readReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
            configuration.readException(new Ice.SocketException());
            var t = background.opAsync();
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.SocketException)
            {
            }
            test(t.IsCompleted);
            configuration.readException(null);
            configuration.readReady(true);
        }

        try
        {
            background.ice_ping();
            configuration.writeReady(false);
            background.op();
            configuration.writeReady(true);
        }
        catch (Ice.LocalException)
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
        catch (Ice.LocalException)
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
        catch (Ice.SocketException)
        {
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        for (int i = 0; i < 2; ++i)
        {
            BackgroundPrx prx = i == 0 ? background : (BackgroundPrx)background.ice_oneway();

            background.ice_ping();
            configuration.writeReady(false);
            configuration.writeException(new Ice.SocketException());
            bool sentSynchronously = false;
            var t = prx.opAsync(progress: new Progress<bool>(value =>
            {
                sentSynchronously = value;
            }));
            test(!sentSynchronously);
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.SocketException)
            {
            }
            test(t.IsCompleted);
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
        catch (Ice.SocketException)
        {
            configuration.readException(null);
            configuration.readReady(true);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.readException(new Ice.SocketException());
            var t = background.opAsync();
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.SocketException)
            {
            }
            test(t.IsCompleted);
            configuration.readReady(true);
            configuration.readException(null);
        }

        {
            background.ice_ping();
            configuration.readReady(false);
            configuration.writeReady(false);
            configuration.readException(new Ice.SocketException());
            var t = background.opAsync();
            // The read exception might propagate before the message send is seen as completed on IOCP.
            //r.waitForSent();
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is Ice.SocketException)
            {
            }
            test(t.IsCompleted);
            configuration.writeReady(true);
            configuration.readReady(true);
            configuration.readException(null);
        }

        background.ice_ping(); // Establish the connection

        BackgroundPrx backgroundOneway = BackgroundPrxHelper.uncheckedCast(background.ice_oneway());
        test(backgroundOneway.ice_getConnection() == background.ice_getConnection());

        ctl.holdAdapter(); // Hold to block in request send.

        byte[] seq = new byte[10024];
        (new System.Random()).NextBytes(seq);
        OpAMICallback cbWP = new OpAMICallback();

        // Fill up the receive and send buffers
        for (int i = 0; i < 200; ++i) // 2MB
        {
            backgroundOneway.opWithPayloadAsync(seq);
        }

        OpAMICallback cb = new OpAMICallback();
        bool t1Sent = false;
        var t1 = background.opAsync(progress: new Progress<bool>(value =>
        {
            cb.sent(value);
            t1Sent = true;
        })).ContinueWith(p =>
        {
            try
            {
                p.Wait();
                cb.response();
            }
            catch (Ice.Exception ex)
            {
                cb.exception(ex);
            }
        });
        test(!t1Sent);

        OpAMICallback cb2 = new OpAMICallback();
        var t2Sent = false;
        var t2 = background.opAsync(progress: new Progress<bool>(value =>
        {
            cb2.sent(value);
            t2Sent = true;
        })).ContinueWith((Task p) =>
        {
            try
            {
                p.Wait();
                cb2.response();
            }
            catch (Ice.Exception ex)
            {
                cb2.noException(ex);
            }
        });
        test(!t2Sent);

        var t1SentSynchronously = false;
        t1 = backgroundOneway.opWithPayloadAsync(seq, progress: new Progress<bool>(value =>
        {
            t1SentSynchronously = value;
        }));
        test(!t1SentSynchronously);
        t1.ContinueWith((Task p) =>
        {
            try
            {
                p.Wait();
                cbWP.noResponse();
            }
            catch (Ice.Exception ex)
            {
                cbWP.noException(ex);
            }
        });

        var t2SentSynchronously = false;
        t2 = backgroundOneway.opWithPayloadAsync(seq, progress: new Progress<bool>(value =>
        {
            t2SentSynchronously = value;
        }));
        test(!t2SentSynchronously);
        t2.ContinueWith((Task p) =>
        {
            try
            {
                p.Wait();
                cbWP.noResponse();
            }
            catch (Ice.Exception ex)
            {
                cbWP.noException(ex);
            }
        });

        test(!cb.checkResponse(false));
        test(!cb2.checkResponse(false));
        ctl.resumeAdapter();
        cb.checkResponseAndSent();
        cb2.checkResponseAndSent();
        test(t1Sent && t1.IsCompleted);
        test(t2Sent && t2.IsCompleted);

        try
        {
            background.ice_ping();
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch (Ice.ConnectionLostException)
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
        catch (Ice.ConnectionLostException)
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
        catch (Ice.LocalException)
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
        catch (Ice.LocalException)
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
        catch (Ice.ConnectionLostException)
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
        catch (Ice.ConnectionLostException)
        {
            ctl.readException(false);
            ctl.readReady(true);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for (int i = 0; i < 5; i++)
        {
            try
            {
                background.ice_ping();
            }
            catch (Ice.LocalException)
            {
                test(false);
            }

            Thread.Sleep(10);
            configuration.writeException(new Ice.SocketException());
            try
            {
                background.op();
            }
            catch (Ice.LocalException)
            {
            }
            configuration.writeException(null);

            Thread.Sleep(10);

            background.ice_ping();
            background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
            Thread.Sleep(10);

            background.ice_getCachedConnection().close(Ice.ConnectionClose.Forcefully);
        }

        thread1.destroy();
        thread2.destroy();

        thread1.Join();
        thread2.Join();
    }
}
