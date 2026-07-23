// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.middleware;

public class AllTests : global::Test.AllTests
{
    public static void allTests(global::Test.TestHelper helper)
    {
        TextWriter output = helper.getWriter();
        Communicator communicator = helper.communicator();
        testMiddlewareExecutionOrder(communicator, output);
        testMiddlewareFactoryException(output);
    }

    // Verifies the middleware execute in installation order.
    private static void testMiddlewareExecutionOrder(Communicator communicator, TextWriter output)
    {
        output.Write("testing middleware execution order... ");
        output.Flush();

        // Arrange
        var inLog = new List<string>();
        var outLog = new List<string>();

        ObjectAdapter oa = communicator.createObjectAdapter("");

        oa.add(new MyObjectI(), new Identity { name = "test" });
        oa.use(next => new Middleware(next, "A", inLog, outLog))
            .use(next => new Middleware(next, "B", inLog, outLog))
            .use(next => new Middleware(next, "C", inLog, outLog));

        Test.MyObjectPrx p = Test.MyObjectPrxHelper.createProxy(communicator, "test");

        // Act
        p.ice_ping();

        // Assert
        test(inLog.SequenceEqual(["A", "B", "C"]));
        test(outLog.SequenceEqual(["C", "B", "A"]));

        output.WriteLine("ok");
        oa.destroy();
    }

    // Verifies a middleware factory exception makes all dispatches fail with a generic UnknownException.
    private static void testMiddlewareFactoryException(TextWriter output)
    {
        output.Write("testing middleware factory exception... ");
        output.Flush();

        // Use a separate communicator with a null logger: the pipeline creation failure is logged as an error.
        var initData = new InitializationData { logger = new NullLogger() };
        using Communicator communicator = Util.initialize(initData);

        ObjectAdapter oa = communicator.createObjectAdapter("");
        oa.add(new MyObjectI(), new Identity { name = "test" });

        oa.use(next => throw new InvalidOperationException("middleware factory exception"));

        Test.MyObjectPrx p = Test.MyObjectPrxHelper.createProxy(communicator, "test");

        try
        {
            p.ice_ping();
            test(false);
        }
        catch (UnknownException ex)
        {
            // The message does not reveal the middleware factory exception.
            test(!ex.Message.Contains("middleware factory exception"));
        }

        // The failure is permanent for this object adapter.
        try
        {
            p.ice_ping();
            test(false);
        }
        catch (UnknownException)
        {
        }

        output.WriteLine("ok");
        oa.destroy();
    }

    private class Middleware : Object
    {
        private readonly Object _next;
        private readonly string _name;
        private readonly List<string> _inLog;
        private readonly List<string> _outLog;

        public async ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
        {
            _inLog.Add(_name);
            OutgoingResponse response = await _next.dispatchAsync(request);
            _outLog.Add(_name);
            return response;
        }

        internal Middleware(Object next, string name, List<string> inLog, List<string> outLog)
        {
            _next = next;
            _name = name;
            _inLog = inLog;
            _outLog = outLog;
        }
    }

    private class NullLogger : Logger
    {
        public void print(string message)
        {
        }

        public void trace(string category, string message)
        {
        }

        public void warning(string message)
        {
        }

        public void error(string message)
        {
        }

        public string getPrefix() => "NullLogger";

        public Logger cloneWithPrefix(string prefix) => new NullLogger();

        public void Dispose()
        {
        }
    }
}
