// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import com.zeroc.Ice.*;

import test.Ice.middleware.Test.MyObject;
import test.Ice.middleware.Test.MyObjectPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

import com.zeroc.Ice.Object;

public class AllTests {

    private static class Middleware implements Object {
        private final Object _next;
        private final String _name;
        private final List<String> _inLog;
        private final List<String> _outLog;

        Middleware(
                Object next, String name, List<String> inLog, List<String> outLog) {
            _next = next;
            _name = name;
            _inLog = inLog;
            _outLog = outLog;
        }

        @Override
        public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
            throws UserException {
            _inLog.add(_name);
            return _next.dispatch(request)
                .thenApply(
                    response -> {
                        _outLog.add(_name);
                        return response;
                    });
        }
    }

    private static class MyFragileObject implements MyObject {
        private final boolean _throwError;

        @Override
        public CompletionStage<String> getNameAsync(Current current) {
            if (_throwError) {
                return CompletableFuture.failedFuture(new StackOverflowError());
            } else {
                return CompletableFuture.completedFuture("Foo");
            }
        }

        @Override
        public void ice_ping(Current current) {
            if (_throwError) {
                throw new StackOverflowError();
            }
        }

        MyFragileObject(boolean throwError) {
            _throwError = throwError;
        }
    }

    private static class ErrorHolder {
        Error error;
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter output = helper.getWriter();
        testMiddlewareExecutionOrder(communicator, output);

        // No error
        testErrorObserverMiddleware(communicator, output, false, false);
        testErrorObserverMiddleware(communicator, output, false, true);

        // With error
        testErrorObserverMiddleware(communicator, output, true, false);
        testErrorObserverMiddleware(communicator, output, true, true);
    }

    private static void testMiddlewareExecutionOrder(
            Communicator communicator, PrintWriter output) {
        output.write("testing middleware execution order... ");
        output.flush();

        // Arrange
        List<String> inLog = new ArrayList<>();
        List<String> outLog = new ArrayList<>();

        ObjectAdapter oa = communicator.createObjectAdapter("");

        ObjectPrx obj = oa.add(new MyObjectI(), new Identity("test", ""));

        oa.use(next -> new Middleware(next, "A", inLog, outLog))
            .use(next -> new Middleware(next, "B", inLog, outLog))
            .use(next -> new Middleware(next, "C", inLog, outLog));

        var p = MyObjectPrx.uncheckedCast(obj);

        // Act
        p.ice_ping();

        // Assert
        test(inLog.equals(Arrays.asList("A", "B", "C")));
        test(outLog.equals(Arrays.asList("C", "B", "A")));

        output.println("ok");
        oa.destroy();
    }

    private static void testErrorObserverMiddleware(
            Communicator communicator, PrintWriter output, boolean withError, boolean amd) {
        output.write(
            "testing error observer middleware "
                + (withError ? "with" : "without")
                + " error"
                + (amd ? " + amd" : "")
                + "... ");
        output.flush();

        // Arrange
        var errorHolder = new ErrorHolder();

        ObjectAdapter oa = communicator.createObjectAdapter("");
        ObjectPrx obj = oa.add(new MyFragileObject(withError), new Identity("test", ""));
        oa.use(next -> new ErrorObserverMiddleware(next, error -> errorHolder.error = error));
        var p = MyObjectPrx.uncheckedCast(obj);

        // Act
        try {
            if (amd) {
                p.getName();
            } else {
                p.ice_ping();
            }
            test(!withError);
        } catch (UnknownException e) {
            test(withError); // expected
        }

        // Assert
        if (withError) {
            test(errorHolder.error instanceof StackOverflowError);
        } else {
            test(errorHolder.error == null);
        }

        output.println("ok");
        oa.destroy();
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private AllTests() {
    }
}
