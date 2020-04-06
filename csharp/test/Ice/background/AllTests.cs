//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Ice;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if (!b)
        {
            Debug.Assert(false);
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
        public void response() => _response.called();

        public void responseNoOp()
        {
        }

        public void noResponse() => test(false);

        public void exception(System.Exception ex) => _response.called();

        public void noException(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }

        public void sent(bool ss) => _sent.called();

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
        internal OpThread(IBackgroundPrx background)
        {
            _background = background.Clone(oneway: true);
            Start();
        }

        public void Join()
        {
            Debug.Assert(_thread != null);
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
                        _background.Clone(oneway: false).IcePing();
                    }
                    _background.opAsync();
                    Thread.Sleep(1);
                }
                catch (System.Exception)
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
        private IBackgroundPrx _background;
        private Thread? _thread;
    }

    public static Test.IBackgroundPrx allTests(Test.TestHelper helper)
    {
        Communicator communicator = helper.communicator();
        var background = IBackgroundPrx.Parse($"background:{helper.getTestEndpoint(0)}", communicator);

        var backgroundController = IBackgroundControllerPrx.Parse("backgroundController:" + helper.getTestEndpoint(1, "tcp"), communicator);

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

            var locator = ILocatorPrx.Parse($"locator:{helper.getTestEndpoint(0)}", communicator).Clone(
                invocationTimeout: 250);
            var obj = IObjectPrx.Parse("background@Test", communicator).Clone(locator: locator, oneway: true);

            backgroundController.pauseCall("findAdapterById");
            try
            {
                obj.IcePing();
                test(false);
            }
            catch (TimeoutException)
            {
            }
            backgroundController.resumeCall("findAdapterById");

            locator = ILocatorPrx.Parse($"locator:{helper.getTestEndpoint(0)}", communicator).Clone(locator: locator);
            locator.IcePing();

            var bg = IBackgroundPrx.Parse("background@Test", communicator).Clone(locator: locator);

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
            var router = IRouterPrx.Parse($"router:{helper.getTestEndpoint(0)}", communicator).Clone(
                invocationTimeout: 250);
            var obj = IObjectPrx.Parse("background@Test", communicator).Clone(router: router, oneway: true);

            backgroundController.pauseCall("getClientProxy");
            try
            {
                obj.IcePing();
                test(false);
            }
            catch (TimeoutException)
            {
            }
            backgroundController.resumeCall("getClientProxy");

            router = IRouterPrx.Parse($"router:{helper.getTestEndpoint(0)}", communicator);
            var bg = IBackgroundPrx.Parse("background@Test", communicator).Clone(router: router);
            test(bg.Router != null);

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

        bool ws = communicator.GetProperty("Ice.Default.Transport") == "test-ws";
        bool wss = communicator.GetProperty("Ice.Default.Transport") == "test-wss";
        if (!ws && !wss)
        {
            Console.Write("testing buffered transport... ");
            Console.Out.Flush();

            configuration.buffered(true);
            backgroundController.buffered(true);
            background.opAsync();
            background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
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
                    catch (System.Exception ex)
                    {
                        cb.noException(ex);
                    }
                });
                results.Add(t);
                if (i % 50 == 0)
                {
                    backgroundController.readReady(false);
                    backgroundController.readReady(true);
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

    private static void connectTests(Configuration configuration, Test.IBackgroundPrx background)
    {
        try
        {
            background.op();
        }
        catch (System.Exception ex)
        {
            System.Console.Out.WriteLine(ex);
            test(false);
        }
        background.GetConnection().Close(ConnectionClose.GracefullyWithWait);

        for (int i = 0; i < 4; ++i)
        {
            if (i == 0 || i == 2)
            {
                configuration.connectorsException(new DNSException());
            }
            else
            {
                configuration.connectException(new TransportException(""));
            }
            IBackgroundPrx prx = (i == 1 || i == 3) ? background : background.Clone(oneway: true);

            bool called = false;
            try
            {
                prx.op();
                called = true;
            }
            catch (DNSException)
            {
                test(i == 0 || i == 2);
            }
            catch (TransportException)
            {
                test(i == 1 || i == 3);
            }
            test(!called);

            try
            {
                prx.opAsync(progress: new Progress<bool>(value => test(false)));
                test(false);
            }
            catch (DNSException)
            {
                test(i == 0 || i == 2);
            }
            catch (TransportException)
            {
                test(i == 1 || i == 3);
            }

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
                    background.IcePing();
                }
                catch (System.Exception)
                {
                    test(false);
                }

                configuration.connectException(new TransportException(""));
                background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
                Thread.Sleep(10);
                configuration.connectException(null);
                try
                {
                    background.IcePing();
                }
                catch (System.Exception)
                {
                }
            }
        }
        catch (Exception ex)
        {
            Console.Out.WriteLine(ex);
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

    private static void initializeTests(Configuration configuration, IBackgroundPrx background,
        IBackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (Exception)
        {
            test(false);
        }
        background.GetConnection().Close(ConnectionClose.GracefullyWithWait);

        for (int i = 0; i < 4; ++i)
        {
            if (i == 0 || i == 2)
            {
                configuration.initializeException(new TransportException(""));
            }
            else
            {
                continue;
            }
            IBackgroundPrx prx = (i == 1 || i == 3) ? background : background.Clone(oneway: true);

            try
            {
                prx.op();
                test(false);
            }
            catch (TransportException)
            {
            }

            try
            {
                prx.opAsync(progress: new Progress<bool>(value => test(false)));
                test(false);
            }
            catch (TransportException)
            {
            }

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
        catch (ConnectionLostException)
        {
            ctl.initializeException(false);
        }
        catch (SecurityException)
        {
            ctl.initializeException(false);
        }

        OpThread thread1 = new OpThread(background);
        OpThread thread2 = new OpThread(background);

        for (int i = 0; i < 5; i++)
        {
            try
            {
                background.IcePing();
            }
            catch (System.Exception)
            {
                test(false);
            }

            configuration.initializeException(new TransportException(""));
            background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
            Thread.Sleep(10);
            configuration.initializeException(null);
            try
            {
                background.IcePing();
            }
            catch (Exception)
            {
            }
            try
            {
                background.IcePing();
            }
            catch (Exception)
            {
                test(false);
            }

            background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
            background.IcePing();

            ctl.initializeException(true);
            background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
            Thread.Sleep(10);
            ctl.initializeException(false);
            try
            {
                background.IcePing();
            }
            catch (Exception)
            {
            }
            try
            {
                background.IcePing();
            }
            catch (System.Exception)
            {
                test(false);
            }

            try
            {
                background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
                background.op();
            }
            catch (Exception)
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

        public void closed(Connection con)
        {
            called();
        }
    }

    //
    // Close the connection associated with the proxy and wait until the close completes.
    //
    private static void closeConnection(IObjectPrx prx)
    {
        CloseCallback cb = new CloseCallback();
        prx.GetConnection().SetCloseCallback(cb.closed);
        prx.GetConnection().Close(ConnectionClose.GracefullyWithWait);
        cb.check();
    }

    private static void validationTests(Configuration configuration, IBackgroundPrx background, IBackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (System.Exception)
        {
            test(false);
        }
        closeConnection(background);

        try
        {
            // Get the read() of connection validation to throw right away.
            configuration.readException(new TransportException(""));
            background.op();
            test(false);
        }
        catch (TransportException)
        {
            configuration.readException(null);
        }

        for (int i = 0; i < 2; ++i)
        {
            configuration.readException(new TransportException(""));
            IBackgroundPrx prx = i == 0 ? background : background.Clone(oneway: true);
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
            catch (AggregateException ex) when (ex.InnerException is TransportException)
            {
            }
            test(t.IsCompleted);
            configuration.readException(null);
        }

        if (background.Communicator.GetProperty("Ice.Default.Transport") != "test-ssl" &&
            background.Communicator.GetProperty("Ice.Default.Transport") != "test-wss")
        {
            try
            {
                // Get the read() of the connection validation to return "would block"
                configuration.readReady(false);
                background.op();
                configuration.readReady(true);
            }
            catch (System.Exception ex)
            {
                Console.Error.WriteLine(ex);
                test(false);
            }
            closeConnection(background);

            try
            {
                // Get the read() of the connection validation to return "would block" and then throw.
                configuration.readReady(false);
                configuration.readException(new TransportException(""));
                background.op();
                test(false);
            }
            catch (TransportException)
            {
                configuration.readException(null);
                configuration.readReady(true);
            }

            for (int i = 0; i < 2; ++i)
            {
                configuration.readReady(false);
                configuration.readException(new TransportException(""));
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
                catch (AggregateException ex) when (ex.InnerException is TransportException)
                {
                }
                test(t.IsCompleted);
                configuration.readException(null);
                configuration.readReady(true);
            }
        }

        {
            // TODO: This test relied on the connection hold behavior which has been removed. The test is disabled
            // for now and until the background tests are rewritten when we'll refactor the transport + thread pool.
            //
            // ctl.readReady(false); // Hold to block in connection validation
            // var t1SentSynchronously = false;
            // var t2SentSynchronously = false;
            // var t1 = background.opAsync(progress: new Progress<bool>(value =>
            // {
            //     t1SentSynchronously = value;
            // }));
            // var t2 = background.opAsync(progress: new Progress<bool>(value =>
            // {
            //     t2SentSynchronously = value;
            // }));
            // test(!t1SentSynchronously && !t2SentSynchronously);
            // test(!t1.IsCompleted && !t2.IsCompleted);
            // ctl.readReady(true);
            // t1.Wait();
            // t2.Wait();
            // test(t1.IsCompleted && t2.IsCompleted);
        }

        try
        {
            // Get the write() of connection validation to throw right away.
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch (ConnectionLostException)
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
        catch (System.Exception ex)
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
        catch (ConnectionLostException)
        {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

    }

    private static void readWriteTests(Configuration configuration, IBackgroundPrx background, IBackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            test(false);
        }

        for (int i = 0; i < 2; ++i)
        {
            IBackgroundPrx prx = i == 0 ? background : background.Clone(oneway: true);

            try
            {
                prx.IcePing();
                configuration.writeException(new TransportException(""));
                prx.op();
                test(false);
            }
            catch (TransportException)
            {
                configuration.writeException(null);
            }

            try
            {
                background.IcePing();
                configuration.writeException(new TransportException(""));
                prx.opAsync(progress: new Progress<bool>(value => test(false)));
                test(false);
            }
            catch (TransportException)
            {
                configuration.writeException(null);
            }
        }

        try
        {
            background.IcePing();
            configuration.readException(new TransportException(""));
            background.op();
            test(false);
        }
        catch (TransportException)
        {
            configuration.readException(null);
        }

        {
            background.IcePing();
            configuration.readReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
            configuration.readException(new TransportException(""));
            var t = background.opAsync();
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is TransportException)
            {
            }
            test(t.IsCompleted);
            configuration.readException(null);
            configuration.readReady(true);
        }

        try
        {
            background.IcePing();
            configuration.writeReady(false);
            background.op();
            configuration.writeReady(true);
        }
        catch (System.Exception)
        {
            test(false);
        }

        try
        {
            background.IcePing();
            configuration.readReady(false);
            background.op();
            configuration.readReady(true);
        }
        catch (System.Exception)
        {
            test(false);
        }

        try
        {
            background.IcePing();
            configuration.writeReady(false);
            configuration.writeException(new TransportException(""));
            background.op();
            test(false);
        }
        catch (TransportException)
        {
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        for (int i = 0; i < 2; ++i)
        {
            IBackgroundPrx prx = i == 0 ? background : background.Clone(oneway: true);

            background.IcePing();
            configuration.writeReady(false);
            configuration.writeException(new TransportException(""));
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
            catch (AggregateException ex) when (ex.InnerException is TransportException)
            {
            }
            test(t.IsCompleted);
            configuration.writeReady(true);
            configuration.writeException(null);
        }

        try
        {
            background.IcePing();
            configuration.readReady(false);
            configuration.readException(new TransportException(""));
            background.op();
            test(false);
        }
        catch (TransportException)
        {
            configuration.readException(null);
            configuration.readReady(true);
        }

        {
            background.IcePing();
            configuration.readReady(false);
            configuration.readException(new TransportException(""));
            var t = background.opAsync();
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is TransportException)
            {
            }
            test(t.IsCompleted);
            configuration.readReady(true);
            configuration.readException(null);
        }

        {
            background.IcePing();
            configuration.readReady(false);
            configuration.writeReady(false);
            configuration.readException(new TransportException(""));
            var t = background.opAsync();
            // The read exception might propagate before the message send is seen as completed on IOCP.
            //r.waitForSent();
            try
            {
                t.Wait();
                test(false);
            }
            catch (AggregateException ex) when (ex.InnerException is TransportException)
            {
            }
            test(t.IsCompleted);
            configuration.writeReady(true);
            configuration.readReady(true);
            configuration.readException(null);
        }

        background.IcePing(); // Establish the connection

        IBackgroundPrx backgroundOneway = background.Clone(oneway: true);
        test(backgroundOneway.GetConnection() == background.GetConnection());

        // TODO: This test relied on the connection hold behavior which has been removed. The test is disabled
        // for now and until the background tests are rewritten when we'll refactor the transport + thread pool.
        //
        // ctl.readReady(false); // Hold to block in request send.

        // byte[] seq = new byte[1000 * 1024];
        // (new System.Random()).NextBytes(seq);
        // OpAMICallback cbWP = new OpAMICallback();

        // // Fill up the receive and send buffers
        // for (int i = 0; i < 10; ++i) // 10MB
        // {
        //     backgroundOneway.opWithPayloadAsync(seq);
        // }

        // OpAMICallback cb = new OpAMICallback();
        // bool t1Sent = false;
        // var t1 = background.opAsync(progress: new Progress<bool>(value =>
        // {
        //     cb.sent(value);
        //     t1Sent = true;
        // }));

        // t1.ContinueWith(p =>
        // {
        //     try
        //     {
        //         p.Wait();
        //         cb.response();
        //     }
        //     catch (System.Exception ex)
        //     {
        //         cb.exception(ex);
        //     }
        // });
        // test(!t1Sent);

        // OpAMICallback cb2 = new OpAMICallback();
        // var t2Sent = false;
        // var t2 = background.opAsync(progress: new Progress<bool>(value =>
        // {
        //     cb2.sent(value);
        //     t2Sent = true;
        // }));
        // t2.ContinueWith((Task p) =>
        // {
        //     try
        //     {
        //         p.Wait();
        //         cb2.response();
        //     }
        //     catch (System.Exception ex)
        //     {
        //         cb2.noException(ex);
        //     }
        // });
        // test(!t2Sent);

        // var t3SentSynchronously = false;
        // var t3 = backgroundOneway.opWithPayloadAsync(seq, progress: new Progress<bool>(value =>
        // {
        //     t3SentSynchronously = value;
        // }));
        // test(!t3SentSynchronously);
        // t3.ContinueWith((Task p) =>
        // {
        //     try
        //     {
        //         p.Wait();
        //     }
        //     catch (System.Exception ex)
        //     {
        //         cbWP.noException(ex);
        //     }
        // });

        // var t4SentSynchronously = false;
        // var t4 = backgroundOneway.opWithPayloadAsync(seq, progress: new Progress<bool>(value =>
        // {
        //     t4SentSynchronously = value;
        // }));
        // test(!t4SentSynchronously);
        // t4.ContinueWith((Task p) =>
        // {
        //     try
        //     {
        //         p.Wait();
        //     }
        //     catch (System.Exception ex)
        //     {
        //         cbWP.noException(ex);
        //     }
        // });

        // test(!cb.checkResponse(false));
        // test(!cb2.checkResponse(false));
        // ctl.readReady(true);
        // cb.checkResponseAndSent();
        // cb2.checkResponseAndSent();
        // test(t1Sent);
        // test(t1.IsCompleted);
        // test(t2Sent);
        // test(t2.IsCompleted);

        try
        {
            background.IcePing();
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch (ConnectionLostException)
        {
            ctl.writeException(false);
        }

        try
        {
            background.IcePing();
            ctl.readException(true);
            background.op();
            test(false);
        }
        catch (ConnectionLostException)
        {
            ctl.readException(false);
        }

        try
        {
            background.IcePing();
            ctl.writeReady(false);
            background.op();
            ctl.writeReady(true);
        }
        catch (System.Exception)
        {
            test(false);
        }

        try
        {
            background.IcePing();
            ctl.readReady(false);
            background.op();
            ctl.readReady(true);
        }
        catch (System.Exception)
        {
            test(false);
        }

        try
        {
            background.IcePing();
            ctl.writeReady(false);
            ctl.writeException(true);
            background.op();
            test(false);
        }
        catch (ConnectionLostException)
        {
            ctl.writeException(false);
            ctl.writeReady(true);
        }

        try
        {
            background.IcePing();
            ctl.readReady(false);
            ctl.readException(true);
            background.op();
            test(false);
        }
        catch (ConnectionLostException)
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
                background.IcePing();
            }
            catch (System.Exception)
            {
                test(false);
            }

            Thread.Sleep(10);
            configuration.writeException(new TransportException(""));
            try
            {
                background.op();
            }
            catch (System.Exception)
            {
            }
            configuration.writeException(null);

            Thread.Sleep(10);

            background.IcePing();
            background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
            Thread.Sleep(10);

            background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
        }

        thread1.destroy();
        thread2.destroy();

        thread1.Join();
        thread2.Join();
    }
}
