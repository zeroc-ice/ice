// Copyright (c) ZeroC, Inc.

package test.Glacier2.router;

import com.zeroc.Ice.Current;

import test.Glacier2.router.Test.CallbackException;
import test.Glacier2.router.Test.CallbackReceiver;
import test.TestHelper;

final class CallbackReceiverI implements CallbackReceiver {
    CallbackReceiverI() {
        _callback = false;
    }

    public synchronized void callback(Current current) {
        assert (!_callback);
        _callback = true;
        notify();
    }

    public void callbackEx(Current current) throws CallbackException {
        callback(current);
        CallbackException ex = new CallbackException();
        ex.someValue = 3.14;
        ex.someString = "3.14";
        throw ex;
    }

    synchronized void callbackOK() {
        while (!_callback) {
            try {
                wait(30000);
                TestHelper.test(_callback);
            } catch (InterruptedException ex) {}
        }

        _callback = false;
    }

    private boolean _callback;
}
