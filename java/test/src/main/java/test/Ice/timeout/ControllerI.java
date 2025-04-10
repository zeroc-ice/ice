// Copyright (c) ZeroC, Inc.

package test.Ice.timeout;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.timeout.Test.Controller;

class ControllerI implements Controller {
    static class ActivateAdapterThread extends Thread {
        ActivateAdapterThread(ObjectAdapter adapter, int timeout) {
            _adapter = adapter;
            _timeout = timeout;
        }

        @Override
        public void run() {
            _adapter.waitForHold();
            try {
                sleep(_timeout);
            } catch (InterruptedException ex) {}
            _adapter.activate();
        }

        ObjectAdapter _adapter;
        int _timeout;
    }

    public ControllerI(ObjectAdapter adapter) {
        _adapter = adapter;
    }

    @Override
    public void holdAdapter(int to, Current current) {
        _adapter.hold();
        if (to >= 0) {
            Thread thread = new ActivateAdapterThread(_adapter, to);
            thread.start();
        }
    }

    @Override
    public void resumeAdapter(Current current) {
        _adapter.activate();
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    final ObjectAdapter _adapter;
}
