// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.middleware;

public class AllTests : global::Test.AllTests
{
    public static void allTests(global::Test.TestHelper helper)
    {
        var output = helper.getWriter();
        Communicator communicator = helper.communicator();
        testMiddlewareExecutionOrder(communicator, output);
    }

    // Verifies the middleware execute in installation order.
    private static void testMiddlewareExecutionOrder(Communicator communicator, TextWriter output)
    {
        output.Write("testing middleware execution order... ");
        output.Flush();

        // Arrange
        var inLog = new List<string>();
        var outLog = new List<string>();

        var oa = communicator.createObjectAdapter("");

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
        oa.deactivate();
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
            var response = await _next.dispatchAsync(request);
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
}
