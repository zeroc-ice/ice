//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Background
{
    public class AllTests
    {
        private class Callback
        {
            internal Callback() => _called = false;

            public virtual void Check()
            {
                lock (this)
                {
                    while (!_called)
                    {
                        Monitor.Wait(this);
                    }

                    _called = false;
                }
            }

            public virtual void Called()
            {
                lock (this)
                {
                    TestHelper.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(this);
                }
            }

            public virtual bool IsCalled()
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
            public void Response() => _response.Called();

            public void ResponseNoOp()
            {
            }

            public void NoResponse() => TestHelper.Assert(false);

            public void Exception() => _response.Called();

            public void NoException(Exception ex)
            {
                Console.Error.WriteLine(ex);
                TestHelper.Assert(false);
            }

            public void Sent() => _sent.Called();

            public bool CheckException(bool wait)
            {
                if (wait)
                {
                    _response.Check();
                    return true;
                }
                else
                {
                    return _response.IsCalled();
                }
            }

            public bool CheckResponse(bool wait)
            {
                if (wait)
                {
                    _response.Check();
                    return true;
                }
                else
                {
                    return _response.IsCalled();
                }
            }

            public void CheckResponseAndSent()
            {
                _sent.Check();
                _response.Check();
            }

            private readonly Callback _response = new Callback();
            private readonly Callback _sent = new Callback();
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
                TestHelper.Assert(_thread != null);
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
                    catch (Exception)
                    {
                    }
                }
            }

            public void Destroy()
            {
                lock (this)
                {
                    _destroyed = true;
                }
            }

            private bool _destroyed = false;
            private readonly IBackgroundPrx _background;
            private Thread? _thread;
        }

        public static IBackgroundPrx allTests(TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var background = IBackgroundPrx.Parse($"background:{helper.GetTestEndpoint(0)}", communicator);

            var backgroundController = IBackgroundControllerPrx.Parse("backgroundController:" + helper.GetTestEndpoint(1, "tcp"), communicator);

            var configuration = Configuration.GetInstance();

            Console.Write("testing connect... ");
            Console.Out.Flush();
            {
                ConnectTests(configuration, background);
            }
            Console.Out.WriteLine("ok");

            Console.Write("testing initialization... ");
            Console.Out.Flush();
            {
                InitializeTests(configuration, background, backgroundController);
            }
            Console.Out.WriteLine("ok");

            Console.Write("testing connection validation... ");
            Console.Out.Flush();
            {
                ValidationTests(configuration, background, backgroundController);
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

                ILocatorPrx locator = ILocatorPrx.Parse($"locator:{helper.GetTestEndpoint(0)}", communicator).Clone(
                    invocationTimeout: 250);
                IObjectPrx obj = IObjectPrx.Parse("background@Test", communicator).Clone(locator: locator, oneway: true);

                backgroundController.pauseCall("findAdapterById");
                try
                {
                    obj.IcePing();
                    TestHelper.Assert(false);
                }
                catch (TimeoutException)
                {
                }
                backgroundController.resumeCall("findAdapterById");

                locator = ILocatorPrx.Parse($"locator:{helper.GetTestEndpoint(0)}", communicator).Clone(locator: locator);
                locator.IcePing();

                IBackgroundPrx bg = IBackgroundPrx.Parse("background@Test", communicator).Clone(locator: locator);

                backgroundController.pauseCall("findAdapterById");
                Task t1 = bg.opAsync();
                Task t2 = bg.opAsync();
                TestHelper.Assert(!t1.IsCompleted);
                TestHelper.Assert(!t2.IsCompleted);
                backgroundController.resumeCall("findAdapterById");
                t1.Wait();
                t2.Wait();
                TestHelper.Assert(t1.IsCompleted);
                TestHelper.Assert(t2.IsCompleted);
            }
            Console.Out.WriteLine("ok");

            Console.Write("testing router... ");
            Console.Out.Flush();
            {
                IRouterPrx router = IRouterPrx.Parse($"router:{helper.GetTestEndpoint(0)}", communicator).Clone(
                    invocationTimeout: 250);
                IObjectPrx obj = IObjectPrx.Parse("background@Test", communicator).Clone(router: router, oneway: true);

                backgroundController.pauseCall("getClientProxy");
                try
                {
                    obj.IcePing();
                    TestHelper.Assert(false);
                }
                catch (TimeoutException)
                {
                }
                backgroundController.resumeCall("getClientProxy");

                router = IRouterPrx.Parse($"router:{helper.GetTestEndpoint(0)}", communicator);
                IBackgroundPrx bg = IBackgroundPrx.Parse("background@Test", communicator).Clone(router: router);
                TestHelper.Assert(bg.Router != null);

                backgroundController.pauseCall("getClientProxy");
                Task t1 = bg.opAsync();
                Task t2 = bg.opAsync();
                TestHelper.Assert(!t1.IsCompleted);
                TestHelper.Assert(!t2.IsCompleted);
                backgroundController.resumeCall("getClientProxy");
                t1.Wait();
                t2.Wait();
                TestHelper.Assert(t1.IsCompleted);
                TestHelper.Assert(t2.IsCompleted);
            }
            Console.Out.WriteLine("ok");

            bool ws = communicator.GetProperty("Ice.Default.Transport") == "test-ws";
            bool wss = communicator.GetProperty("Ice.Default.Transport") == "test-wss";
            if (!ws && !wss)
            {
                Console.Write("testing buffered transport... ");
                Console.Out.Flush();

                configuration.Buffered(true);
                backgroundController.buffered(true);
                background.opAsync();
                background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
                background.opAsync();

                var cb = new OpAMICallback();
                var results = new List<Task>();
                for (int i = 0; i < 10000; ++i)
                {
                    Task t = background.opAsync().ContinueWith((Task p) =>
                    {
                        try
                        {
                            p.Wait();
                            cb.ResponseNoOp();
                        }
                        catch (Exception ex)
                        {
                            cb.NoException(ex);
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

        private static void ConnectTests(Configuration configuration, IBackgroundPrx background)
        {
            try
            {
                background.op();
            }
            catch (Exception ex)
            {
                Console.Out.WriteLine(ex);
                TestHelper.Assert(false);
            }
            background.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);

            for (int i = 0; i < 4; ++i)
            {
                if (i == 0 || i == 2)
                {
                    configuration.ConnectorsException(new DNSException());
                }
                else
                {
                    configuration.ConnectException(new TransportException(""));
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
                    TestHelper.Assert(i == 0 || i == 2);
                }
                catch (TransportException)
                {
                    TestHelper.Assert(i == 1 || i == 3);
                }
                TestHelper.Assert(!called);

                try
                {
                    prx.opAsync(progress: new Progress<bool>(value => TestHelper.Assert(false)));
                    TestHelper.Assert(false);
                }
                catch (DNSException)
                {
                    TestHelper.Assert(i == 0 || i == 2);
                }
                catch (TransportException)
                {
                    TestHelper.Assert(i == 1 || i == 3);
                }

                if (i == 0 || i == 2)
                {
                    configuration.ConnectorsException(null);
                }
                else
                {
                    configuration.ConnectException(null);
                }
            }

            var thread1 = new OpThread(background);
            var thread2 = new OpThread(background);
            try
            {
                for (int i = 0; i < 5; i++)
                {
                    try
                    {
                        background.IcePing();
                    }
                    catch (Exception)
                    {
                        TestHelper.Assert(false);
                    }

                    configuration.ConnectException(new TransportException(""));
                    background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
                    Thread.Sleep(10);
                    configuration.ConnectException(null);
                    try
                    {
                        background.IcePing();
                    }
                    catch (Exception)
                    {
                    }
                }
            }
            catch (Exception ex)
            {
                Console.Out.WriteLine(ex);
                TestHelper.Assert(false);
            }
            finally
            {
                thread1.Destroy();
                thread2.Destroy();

                thread1.Join();
                thread2.Join();
            }
        }

        private static void InitializeTests(Configuration configuration, IBackgroundPrx background,
            IBackgroundControllerPrx ctl)
        {
            try
            {
                background.op();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            background.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);

            for (int i = 0; i < 4; ++i)
            {
                if (i == 0 || i == 2)
                {
                    configuration.InitializeException(new TransportException(""));
                }
                else
                {
                    continue;
                }
                IBackgroundPrx prx = (i == 1 || i == 3) ? background : background.Clone(oneway: true);

                try
                {
                    prx.op();
                    TestHelper.Assert(false);
                }
                catch (TransportException)
                {
                }

                try
                {
                    prx.opAsync(progress: new Progress<bool>(value => TestHelper.Assert(false)));
                    TestHelper.Assert(false);
                }
                catch (TransportException)
                {
                }

                if (i == 0 || i == 2)
                {
                    configuration.InitializeException(null);
                }
            }

            //
            // Now run the same tests with the server side.
            //

            try
            {
                ctl.initializeException(true);
                background.op();
                TestHelper.Assert(false);
            }
            catch (TransportException)
            {
                ctl.initializeException(false);
            }

            var thread1 = new OpThread(background);
            var thread2 = new OpThread(background);

            for (int i = 0; i < 5; i++)
            {
                try
                {
                    background.IcePing();
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }

                configuration.InitializeException(new TransportException(""));
                background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
                Thread.Sleep(10);
                configuration.InitializeException(null);
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
                    TestHelper.Assert(false);
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
                    TestHelper.Assert(false);
                }

                try
                {
                    background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
                    background.op();
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }

            thread1.Destroy();
            thread2.Destroy();

            thread1.Join();
            thread2.Join();
        }

        private sealed class CloseCallback : Callback
        {

            public void Closed() => Called();
        }

        //
        // Close the connection associated with the proxy and wait until the close completes.
        //
        private static void CloseConnection(IObjectPrx prx)
        {
            var cb = new CloseCallback();
            prx.GetConnection()!.SetCloseCallback(_ => cb.Closed());
            prx.GetConnection()!.Close(ConnectionClose.GracefullyWithWait);
            cb.Check();
        }

        private static void ValidationTests(Configuration configuration, IBackgroundPrx background,
            IBackgroundControllerPrx ctl)
        {
            try
            {
                background.op();
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
            CloseConnection(background);

            try
            {
                // Get the read() of connection validation to throw right away.
                configuration.ReadException(new TransportException(""));
                background.op();
                TestHelper.Assert(false);
            }
            catch (TransportException)
            {
                configuration.ReadException(null);
            }

            for (int i = 0; i < 2; ++i)
            {
                configuration.ReadException(new TransportException(""));
                IBackgroundPrx prx = i == 0 ? background : background.Clone(oneway: true);
                bool sentSynchronously = false;
                Task t = prx.opAsync(progress: new Progress<bool>(value => sentSynchronously = value));
                TestHelper.Assert(!sentSynchronously);
                try
                {
                    t.Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is TransportException)
                {
                }
                TestHelper.Assert(t.IsCompleted);
                configuration.ReadException(null);
            }

            if (background.Communicator.GetProperty("Ice.Default.Transport") != "test-ssl" &&
                background.Communicator.GetProperty("Ice.Default.Transport") != "test-wss")
            {
                try
                {
                    // Get the read() of the connection validation to return "would block"
                    configuration.ReadReady(false);
                    background.op();
                    configuration.ReadReady(true);
                }
                catch (System.Exception ex)
                {
                    Console.Error.WriteLine(ex);
                    TestHelper.Assert(false);
                }
                CloseConnection(background);

                try
                {
                    // Get the read() of the connection validation to return "would block" and then throw.
                    configuration.ReadReady(false);
                    configuration.ReadException(new TransportException(""));
                    background.op();
                    TestHelper.Assert(false);
                }
                catch (TransportException)
                {
                    configuration.ReadException(null);
                    configuration.ReadReady(true);
                }

                for (int i = 0; i < 2; ++i)
                {
                    configuration.ReadReady(false);
                    configuration.ReadException(new TransportException(""));
                    bool sentSynchronously = false;
                    Task t = background.opAsync(progress: new Progress<bool>(value => sentSynchronously = value));
                    TestHelper.Assert(!sentSynchronously);
                    try
                    {
                        t.Wait();
                        TestHelper.Assert(false);
                    }
                    catch (AggregateException ex) when (ex.InnerException is TransportException)
                    {
                    }
                    TestHelper.Assert(t.IsCompleted);
                    configuration.ReadException(null);
                    configuration.ReadReady(true);
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
                // TestHelper.Assert(!t1SentSynchronously && !t2SentSynchronously);
                // TestHelper.Assert(!t1.IsCompleted && !t2.IsCompleted);
                // ctl.readReady(true);
                // t1.Wait();
                // t2.Wait();
                // TestHelper.Assert(t1.IsCompleted && t2.IsCompleted);
            }

            try
            {
                // Get the write() of connection validation to throw right away.
                ctl.writeException(true);
                background.op();
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
            }
            CloseConnection(background);

            try
            {
                // Get the write() of the connection validation to return "would block" and then throw.
                ctl.writeReady(false);
                ctl.writeException(true);
                background.op();
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
            }

            for (int i = 0; i < 2; ++i)
            {
                IBackgroundPrx prx = i == 0 ? background : background.Clone(oneway: true);

                try
                {
                    prx.IcePing();
                    configuration.WriteException(new TransportException(""));
                    prx.op();
                    TestHelper.Assert(false);
                }
                catch (TransportException)
                {
                    configuration.WriteException(null);
                }

                try
                {
                    background.IcePing();
                    configuration.WriteException(new TransportException(""));
                    prx.opAsync(progress: new Progress<bool>(value => TestHelper.Assert(false)));
                    TestHelper.Assert(false);
                }
                catch (TransportException)
                {
                    configuration.WriteException(null);
                }
            }

            try
            {
                background.IcePing();
                configuration.ReadException(new TransportException(""));
                background.op();
                TestHelper.Assert(false);
            }
            catch (TransportException)
            {
                configuration.ReadException(null);
            }

            {
                background.IcePing();
                configuration.ReadReady(false); // Required in C# to make sure beginRead() doesn't throw too soon.
                configuration.ReadException(new TransportException(""));
                Task t = background.opAsync();
                try
                {
                    t.Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is TransportException)
                {
                }
                TestHelper.Assert(t.IsCompleted);
                configuration.ReadException(null);
                configuration.ReadReady(true);
            }

            try
            {
                background.IcePing();
                configuration.WriteReady(false);
                background.op();
                configuration.WriteReady(true);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                background.IcePing();
                configuration.ReadReady(false);
                background.op();
                configuration.ReadReady(true);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                background.IcePing();
                configuration.WriteReady(false);
                configuration.WriteException(new TransportException(""));
                background.op();
                TestHelper.Assert(false);
            }
            catch (TransportException)
            {
                configuration.WriteReady(true);
                configuration.WriteException(null);
            }

            for (int i = 0; i < 2; ++i)
            {
                IBackgroundPrx prx = i == 0 ? background : background.Clone(oneway: true);

                background.IcePing();
                configuration.WriteReady(false);
                configuration.WriteException(new TransportException(""));
                bool sentSynchronously = false;
                Task t = prx.opAsync(progress: new Progress<bool>(value => sentSynchronously = value));
                TestHelper.Assert(!sentSynchronously);
                try
                {
                    t.Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is TransportException)
                {
                }
                TestHelper.Assert(t.IsCompleted);
                configuration.WriteReady(true);
                configuration.WriteException(null);
            }

            try
            {
                background.IcePing();
                configuration.ReadReady(false);
                configuration.ReadException(new TransportException(""));
                background.op();
                TestHelper.Assert(false);
            }
            catch (TransportException)
            {
                configuration.ReadException(null);
                configuration.ReadReady(true);
            }

            {
                background.IcePing();
                configuration.ReadReady(false);
                configuration.ReadException(new TransportException(""));
                Task t = background.opAsync();
                try
                {
                    t.Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is TransportException)
                {
                }
                TestHelper.Assert(t.IsCompleted);
                configuration.ReadReady(true);
                configuration.ReadException(null);
            }

            {
                background.IcePing();
                configuration.ReadReady(false);
                configuration.WriteReady(false);
                configuration.ReadException(new TransportException(""));
                Task t = background.opAsync();
                // The read exception might propagate before the message send is seen as completed on IOCP.
                //r.waitForSent();
                try
                {
                    t.Wait();
                    TestHelper.Assert(false);
                }
                catch (AggregateException ex) when (ex.InnerException is TransportException)
                {
                }
                TestHelper.Assert(t.IsCompleted);
                configuration.WriteReady(true);
                configuration.ReadReady(true);
                configuration.ReadException(null);
            }

            background.IcePing(); // Establish the connection

            IBackgroundPrx backgroundOneway = background.Clone(oneway: true);
            TestHelper.Assert(backgroundOneway.GetConnection() == background.GetConnection());

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
            // TestHelper.Assert(!t1Sent);

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
            // TestHelper.Assert(!t2Sent);

            // var t3SentSynchronously = false;
            // var t3 = backgroundOneway.opWithPayloadAsync(seq, progress: new Progress<bool>(value =>
            // {
            //     t3SentSynchronously = value;
            // }));
            // TestHelper.Assert(!t3SentSynchronously);
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
            // TestHelper.Assert(!t4SentSynchronously);
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

            // TestHelper.Assert(!cb.checkResponse(false));
            // TestHelper.Assert(!cb2.checkResponse(false));
            // ctl.readReady(true);
            // cb.checkResponseAndSent();
            // cb2.checkResponseAndSent();
            // TestHelper.Assert(t1Sent);
            // TestHelper.Assert(t1.IsCompleted);
            // TestHelper.Assert(t2Sent);
            // TestHelper.Assert(t2.IsCompleted);

            try
            {
                background.IcePing();
                ctl.writeException(true);
                background.op();
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
            }

            try
            {
                background.IcePing();
                ctl.writeReady(false);
                ctl.writeException(true);
                background.op();
                TestHelper.Assert(false);
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
                TestHelper.Assert(false);
            }
            catch (ConnectionLostException)
            {
                ctl.readException(false);
                ctl.readReady(true);
            }

            var thread1 = new OpThread(background);
            var thread2 = new OpThread(background);

            for (int i = 0; i < 5; i++)
            {
                try
                {
                    background.IcePing();
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }

                Thread.Sleep(10);
                configuration.WriteException(new TransportException(""));
                try
                {
                    background.op();
                }
                catch (Exception)
                {
                }
                configuration.WriteException(null);

                Thread.Sleep(10);

                background.IcePing();
                background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
                Thread.Sleep(10);

                background.GetCachedConnection()!.Close(ConnectionClose.Forcefully);
            }

            thread1.Destroy();
            thread2.Destroy();

            thread1.Join();
            thread2.Join();
        }
    }
}
