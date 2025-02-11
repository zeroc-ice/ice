// Copyright (c) ZeroC, Inc.

package test.Slice.escape;

import test.Slice.escape.java_abstract._assert;
import test.Slice.escape.java_abstract._break;
import test.Slice.escape.java_abstract._catch;
import test.Slice.escape.java_abstract._catchPrx;
import test.Slice.escape.java_abstract._default;
import test.Slice.escape.java_abstract._defaultPrx;
import test.Slice.escape.java_abstract._else;
import test.Slice.escape.java_abstract._finalize;
import test.Slice.escape.java_abstract._finalizePrx;
import test.Slice.escape.java_abstract._hashCode;
import test.Slice.escape.java_abstract._import;
import test.Slice.escape.java_abstract.java_synchronized;

public class Client extends test.TestHelper {
    public static class _catchI implements _catch {
        public _catchI() {}

        @Override
        public java.util.concurrent.CompletionStage<Integer> myCheckedCastAsync(
                int java_clone, com.zeroc.Ice.Current current) {
            int _continue = 0;
            return java.util.concurrent.CompletableFuture.completedFuture(_continue);
        }
    }

    public static class _defaultI implements _default {
        public _defaultI() {}

        @Override
        public void _do(com.zeroc.Ice.Current current) {
            assert current.operation.equals("do");
        }
    }

    public static class _elseI extends _else {
        public _elseI() {}
    }

    public static class finalizeServantI implements _finalize {
        @Override
        public java.util.concurrent.CompletionStage<Integer> myCheckedCastAsync(
                int java_clone, com.zeroc.Ice.Current current) {
            int _continue = 0;
            return java.util.concurrent.CompletableFuture.completedFuture(_continue);
        }

        @Override
        public void _do(com.zeroc.Ice.Current current) {}

        @Override
        public _assert _notify(
                _break java_notifyAll,
                _else java_null,
                _finalizePrx java_package,
                _defaultPrx java_return,
                int java_super,
                com.zeroc.Ice.Current current)
                throws _hashCode, _import {
            return null;
        }
    }

    // This section of the test is present to ensure that the C++ types are named correctly.
    // It is not expected to run.
    @SuppressWarnings({"unused", "null"})
    private static void testtypes() {
        _assert v = _assert.java_boolean;
        _break b = new _break();
        b.java_case = 0;
        _catchPrx c = null;
        c._myCheckedCast(0);
        _catch c1 = new _catchI();
        _defaultPrx d = null;
        d._do();
        _default d1 = new _defaultI();
        _else e1 = new _elseI();
        e1.foo = 0;
        e1._equals = null;

        _finalizePrx f = null;
        f._myCheckedCast(0);
        f._do();

        _hashCode i = new _hashCode();
        i.bar = 0;
        _import j = new _import();
        j.bar = 0;
        j.java_native = 2;
        _finalize k = new finalizeServantI();
        assert java_synchronized.value == 0;
    }

    public void run(String[] args) {
        // In this test, we need at least two threads in the client side thread pool for nested AMI.
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.java_abstract", "test.Slice.escape");
        properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        properties.setProperty("TestAdapter.Endpoints", "default");

        // We must set MessageSizeMax to an explicit value,
        // because we run tests to check whether Ice.MarshalException is raised as expected.
        properties.setProperty("Ice.MessageSizeMax", "100");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new _defaultI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.activate();

            System.out.print("Testing operation name... ");
            System.out.flush();
            _defaultPrx p =
                    _defaultPrx.uncheckedCast(
                            adapter.createProxy(com.zeroc.Ice.Util.stringToIdentity("test")));
            p._do();
            System.out.println("ok");
        }
    }
}
