// Copyright (c) ZeroC, Inc.

package test.Slice.escape;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.Slice.escape.escaped_abstract._assert;
import test.Slice.escape.escaped_abstract._break;
import test.Slice.escape.escaped_abstract._catch;
import test.Slice.escape.escaped_abstract._catchPrx;
import test.Slice.escape.escaped_abstract._default;
import test.Slice.escape.escaped_abstract._defaultPrx;
import test.Slice.escape.escaped_abstract._finalize;
import test.Slice.escape.escaped_abstract._finalizePrx;
import test.Slice.escape.escaped_abstract.clone;
import test.Slice.escape.escaped_abstract.escaped_synchronized;
import test.Slice.escape.escaped_abstract.hashCode;
import test.Slice.escape.escaped_abstract.notify;
import test.TestHelper;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public class Client extends TestHelper {
    public static class _catchI implements _catch {
        public _catchI() {}

        @Override
        public CompletionStage<Integer> myCheckedCastAsync(
                int escaped_clone, Current current) {
            int _continue = 0;
            return CompletableFuture.completedFuture(_continue);
        }
    }

    public static class _defaultI implements _default {
        public _defaultI() {}

        @Override
        public void _do(Current current) {
            assert "do".equals(current.operation);
        }
    }

    public static class notifyI extends notify {
        public notifyI() {}
    }

    public static class finalizeServantI implements _finalize {
        @Override
        public CompletionStage<Integer> myCheckedCastAsync(
                int escaped_clone, Current current) {
            int _continue = 0;
            return CompletableFuture.completedFuture(_continue);
        }

        @Override
        public void _do(Current current) {}

        @Override
        public _assert _notify(
                _break escaped_notifyAll,
                notify escaped_null,
                _finalizePrx escaped_package,
                _defaultPrx escaped_return,
                int escaped_super,
                Current current)
            throws hashCode, clone {
            return null;
        }
    }

    // This section of the test is present to ensure that the C++ types are named correctly. It is
    // not expected to run.
    @SuppressWarnings({"unused", "null"})
    private static void testtypes() {
        _assert v = _assert.escaped_boolean;
        _break b = new _break();
        b.escaped_case = 0;
        _catchPrx c = null;
        c.myCheckedCast(0);
        _catch c1 = new _catchI();
        _defaultPrx d = null;
        d._do();
        _default d1 = new _defaultI();
        notify e1 = new notifyI();
        e1.foo = 0;
        e1._equals = null;

        _finalizePrx f = null;
        f.myCheckedCast(0);
        f._do();

        hashCode i = new hashCode();
        i.bar = 0;
        clone j = new clone();
        j.bar = 0;
        j.escaped_native = "native";
        _finalize k = new finalizeServantI();
        assert escaped_synchronized.value == 0;
    }

    public void run(String[] args) {
        // In this test, we need at least two threads in the client side thread pool for nested AMI.
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.escaped_abstract", "test.Slice.escape");
        properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        properties.setProperty("TestAdapter.Endpoints", "default");

        // We must set MessageSizeMax to an explicit value,
        // because we run tests to check whether Ice.MarshalException is raised as expected.
        properties.setProperty("Ice.MessageSizeMax", "100");
        try (Communicator communicator = initialize(properties)) {
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new _defaultI(), Util.stringToIdentity("test"));
            adapter.activate();

            System.out.print("Testing operation name... ");
            System.out.flush();
            _defaultPrx p =
                _defaultPrx.uncheckedCast(
                    adapter.createProxy(Util.stringToIdentity("test")));
            p._do();
            System.out.println("ok");
        }
    }
}
