// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.ErrorObserverMiddleware;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.IncomingRequest;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OutgoingResponse;
import com.zeroc.Ice.ServantLocator;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UserException;

import test.Ice.middleware.Test.MyException;
import test.Ice.middleware.Test.MyObject;
import test.Ice.middleware.Test.MyObjectPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

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

    // A custom dispatcher whose dispatch always completes exceptionally with a (bare) user exception.
    private static class ThrowingServant implements Object {
        @Override
        public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request) {
            return CompletableFuture.failedFuture(new MyException());
        }
    }

    private static class ThrowingServantLocator implements ServantLocator {
        @Override
        public ServantLocator.LocateResult locate(Current curr) {
            return new ServantLocator.LocateResult(new ThrowingServant(), null);
        }

        @Override
        public void finished(Current curr, Object servant, java.lang.Object cookie) {}

        @Override
        public void deactivate(String category) {}
    }

    private static class ThrowObserver {
        boolean observed;
    }

    private static class ThrowObservingMiddleware implements Object {
        private final Object _next;
        private final ThrowObserver _observer;

        ThrowObservingMiddleware(Object next, ThrowObserver observer) {
            _next = next;
            _observer = observer;
        }

        @Override
        public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
            throws UserException {
            return _next.dispatch(request)
                .whenComplete(
                    (response, ex) -> {
                        if (ex != null) {
                            _observer.observed = true;
                        }
                    });
        }
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

        testMiddlewareObservesLocatorDispatchError(communicator, output);
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

    private static void testMiddlewareObservesLocatorDispatchError(
            Communicator communicator, PrintWriter output) {
        output.write("testing middleware observes a servant-locator dispatch error... ");
        output.flush();

        // Arrange
        var observer = new ThrowObserver();
        ObjectAdapter oa =
            communicator.createObjectAdapterWithEndpoints("MiddlewareLocatorOA", "tcp -h 127.0.0.1 -p 0");
        oa.addServantLocator(new ThrowingServantLocator(), "");
        oa.use(next -> new ThrowObservingMiddleware(next, observer));
        oa.activate();

        var p = MyObjectPrx.uncheckedCast(oa.createProxy(new Identity("test", "")));

        // Act
        boolean threw = false;
        try {
            p.getName();
        } catch (LocalException e) {
            threw = true;
        }

        // Assert: the located servant always fails, so the client sees a dispatch error and the middleware
        // observed the dispatch completing exceptionally, rather than it being converted to a response inside
        // ServantManager.
        test(threw);
        test(observer.observed);

        output.println("ok");
        oa.destroy();
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private AllTests() {}
}
