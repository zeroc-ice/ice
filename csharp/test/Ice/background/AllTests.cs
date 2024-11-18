// Copyright (c) ZeroC, Inc.

using Test;

public class AllTests : global::Test.AllTests
{
    public class Progress : IProgress<bool>
    {
        private readonly Action<bool> _onProgress;

        public Progress(Action<bool> onProgress = null) =>
            _onProgress = onProgress;

        public bool SentSynchronously { get; private set; }

        public void Report(bool value)
        {
            SentSynchronously = value;
            _onProgress?.Invoke(value);
        }
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
                    _ = _background.opAsync();
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

    public static async Task<BackgroundPrx> allTests(Test.TestHelper helper)
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
            await connectTests(configuration, background);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing initialization... ");
        Console.Out.Flush();
        {
            await initializeTests(configuration, background, backgroundController);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing connection validation... ");
        Console.Out.Flush();
        {
            await validationTests(configuration, background, backgroundController);
        }
        Console.Out.WriteLine("ok");

        Console.Write("testing read/write... ");
        Console.Out.Flush();
        {
            await readWriteTests(configuration, background, backgroundController);
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
            Task t1 = bg.opAsync();
            Task t2 = bg.opAsync();
            test(!t1.IsCompleted);
            test(!t2.IsCompleted);
            backgroundController.resumeCall("findAdapterById");
            await t1;
            await t2;
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
            Task t1 = bg.opAsync();
            Task t2 = bg.opAsync();
            test(!t1.IsCompleted);
            test(!t2.IsCompleted);
            backgroundController.resumeCall("getClientProxy");
            await t1;
            await t2;
        }
        Console.Out.WriteLine("ok");

        bool ws = communicator.getProperties().getIceProperty("Ice.Default.Protocol") == "test-ws";
        bool wss = communicator.getProperties().getIceProperty("Ice.Default.Protocol") == "test-wss";
        if (!ws && !wss)
        {
            Console.Write("testing buffered transport... ");
            Console.Out.Flush();

            configuration.buffered(true);
            backgroundController.buffered(true);
            _ = background.opAsync();
            background.ice_getCachedConnection().abort();
            _ = background.opAsync();

            var results = new List<Task>();
            for (int i = 0; i < 10000; ++i)
            {
                Task t = background.opAsync();
                results.Add(t);
                if (i % 50 == 0)
                {
                    backgroundController.holdAdapter();
                    backgroundController.resumeAdapter();
                }
                if (i % 100 == 0)
                {
                    await t;
                }
            }

            foreach (Task t in results)
            {
                await t;
            }
            Console.Out.WriteLine("ok");
        }

        return background;
    }

    private static async Task connectTests(Configuration configuration, Test.BackgroundPrx background)
    {
        try
        {
            background.op();
        }
        catch (Ice.LocalException ex)
        {
            Console.Out.WriteLine(ex);
            test(false);
        }
        await background.ice_getConnection().closeAsync();

        for (int i = 0; i < 4; ++i)
        {
            if (i == 0 || i == 2)
            {
                configuration.connectorsException(new Ice.DNSException("dummy"));
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

            var progress = new Progress();
            Task t = prx.opAsync(progress: progress);
            test(!progress.SentSynchronously);
            try
            {
                await t;
                test(false);
            }
            catch (Ice.Exception)
            {
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
                    background.ice_ping();
                }
                catch (Ice.LocalException)
                {
                    test(false);
                }

                configuration.connectException(new Ice.SocketException());
                background.ice_getCachedConnection().abort();
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

    private static async Task initializeTests(
        Configuration configuration,
        BackgroundPrx background,
        BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
        await background.ice_getConnection().closeAsync();

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

            var progress = new Progress();
            Task t = prx.opAsync(progress: progress);
            test(!progress.SentSynchronously);
            try
            {
                await t;
                test(false);
            }
            catch (Ice.Exception)
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
            background.ice_getCachedConnection().abort();
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

            background.ice_getCachedConnection().abort();
            background.ice_ping();

            ctl.initializeException(true);
            background.ice_getCachedConnection().abort();
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
                background.ice_getCachedConnection().abort();
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

    //
    // Close the connection associated with the proxy and wait until the close completes.
    //
    private static Task closeConnection(Ice.ObjectPrx prx) => prx.ice_getConnection().closeAsync();

    private static async Task validationTests(
        Configuration configuration,
        BackgroundPrx background,
        BackgroundControllerPrx ctl)
    {
        try
        {
            background.op();
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
        await closeConnection(background);

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
            var progress = new Progress();
            var t = prx.opAsync(progress: progress);
            test(!progress.SentSynchronously);
            try
            {
                await t;
                test(false);
            }
            catch (Ice.SocketException)
            {
            }
            configuration.readException(null);
        }

        if (background.ice_getCommunicator().getProperties().getIceProperty("Ice.Default.Protocol") != "test-ssl" &&
           background.ice_getCommunicator().getProperties().getIceProperty("Ice.Default.Protocol") != "test-wss")
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
            await closeConnection(background);

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
                var progress = new Progress();
                var t = background.opAsync(progress: progress);
                test(!progress.SentSynchronously);
                try
                {
                    await t;
                    test(false);
                }
                catch (Ice.SocketException)
                {
                }
                configuration.readException(null);
                configuration.readReady(true);
            }
        }

        {
            ctl.holdAdapter(); // Hold to block in connection validation
            var p1 = new Progress();
            var p2 = new Progress();
            var t1 = background.opAsync(progress: p1);
            var t2 = background.opAsync(progress: p2);
            test(!p1.SentSynchronously && !p2.SentSynchronously);
            test(!t1.IsCompleted && !t2.IsCompleted);
            ctl.resumeAdapter();
            await t1;
            await t2;
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
        await closeConnection(background);

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

        //
        // Then try the same thing with async flush.
        //

        ctl.holdAdapter();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        backgroundBatchOneway.op();
        ctl.resumeAdapter();
        _ = backgroundBatchOneway.ice_flushBatchRequestsAsync();
        await closeConnection(backgroundBatchOneway);

        ctl.holdAdapter();
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        backgroundBatchOneway.opWithPayload(seq);
        ctl.resumeAdapter();
        //
        // We can't close the connection before ensuring all the batches have been sent since
        // with auto-flushing the close connection message might be sent once the first call
        // opWithPayload is sent and before the flushBatchRequests (this would therefore result
        // in the flush to report a CloseConnectionException). Instead we flush a second time
        // with the same callback to wait for the first flush to complete.
        //
        await backgroundBatchOneway.ice_flushBatchRequestsAsync();
        await closeConnection(backgroundBatchOneway);
    }

    private static async Task readWriteTests(
        Configuration configuration,
        BackgroundPrx background,
        BackgroundControllerPrx ctl)
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
            var progress = new Progress();
            var t = prx.opAsync(progress: progress);
            test(!progress.SentSynchronously);
            try
            {
                await t;
                test(false);
            }
            catch (Ice.SocketException)
            {
            }
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
            try
            {
                await background.opAsync();
                test(false);
            }
            catch (Ice.SocketException)
            {
            }
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
            var progress = new Progress();
            var t = prx.opAsync(progress: progress);
            test(!progress.SentSynchronously);
            try
            {
                await t;
                test(false);
            }
            catch (Ice.SocketException)
            {
            }
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
            try
            {
                await background.opAsync();
                test(false);
            }
            catch (Ice.SocketException)
            {
            }
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
                await t;
                test(false);
            }
            catch (Ice.SocketException)
            {
            }
            configuration.writeReady(true);
            configuration.readReady(true);
            configuration.readException(null);
        }

        background.ice_ping(); // Establish the connection

        BackgroundPrx backgroundOneway = BackgroundPrxHelper.uncheckedCast(background.ice_oneway());
        test(backgroundOneway.ice_getConnection() == background.ice_getConnection());

        ctl.holdAdapter(); // Hold to block in request send.

        byte[] seq = new byte[10024];
        new Random().NextBytes(seq);

        // Fill up the receive and send buffers
        for (int i = 0; i < 200; ++i) // 2MB
        {
            _ = backgroundOneway.opWithPayloadAsync(seq);
        }

        var tcs1 = new TaskCompletionSource();
        var p1 = new Progress(value => tcs1.SetResult());
        var t1 = background.opAsync(progress: p1);
        test(!p1.SentSynchronously && !tcs1.Task.IsCompleted);

        var tcs2 = new TaskCompletionSource();
        var p2 = new Progress(value => tcs2.SetResult());
        var t2 = background.opAsync(progress: p2);
        test(!p2.SentSynchronously && !tcs2.Task.IsCompleted);

        var p0 = new Progress();
        _ = backgroundOneway.opWithPayloadAsync(seq, progress: p0);
        test(!p0.SentSynchronously);

        p0 = new Progress();
        _ = backgroundOneway.opWithPayloadAsync(seq, progress: p0);
        test(!p0.SentSynchronously);

        test(!t1.IsCompleted && !tcs1.Task.IsCompleted);
        test(!t2.IsCompleted && !tcs2.Task.IsCompleted);
        ctl.resumeAdapter();
        await t1;
        await t2;
        test(tcs1.Task.IsCompleted);
        test(tcs2.Task.IsCompleted);

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
            background.ice_getCachedConnection().abort();
            Thread.Sleep(10);

            background.ice_getCachedConnection().abort();
        }

        thread1.destroy();
        thread2.destroy();

        thread1.Join();
        thread2.Join();
    }
}
