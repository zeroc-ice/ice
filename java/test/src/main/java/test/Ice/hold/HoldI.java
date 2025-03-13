// Copyright (c) ZeroC, Inc.

package test.Ice.hold;

import test.Ice.hold.Test.Hold;

public final class HoldI implements Hold {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    HoldI(java.util.Timer timer, com.zeroc.Ice.ObjectAdapter adapter) {
        _timer = timer;
        _adapter = adapter;
        _last = 0;
    }

    @Override
    public void putOnHold(int delay, com.zeroc.Ice.Current current) {
        if (delay < 0) {
            _adapter.hold();
        } else if (delay == 0) {
            _adapter.hold();
            _adapter.activate();
        } else {
            _timer.schedule(
                    new java.util.TimerTask() {
                        @Override
                        public void run() {
                            try {
                                putOnHold(0, null);
                            } catch (com.zeroc.Ice.ObjectAdapterDeactivatedException ex) {
                            }
                        }
                    },
                    delay);
        }
    }

    @Override
    public void waitForHold(final com.zeroc.Ice.Current current) {
        _timer.schedule(
                new java.util.TimerTask() {
                    @Override
                    public void run() {
                        try {
                            current.adapter.waitForHold();

                            current.adapter.activate();
                        } catch (com.zeroc.Ice.LocalException ex) {
                            // This shouldn't occur. The test ensures all the waitForHold timers are finished before shutting down the communicator.
                            test(false);
                        }
                    }
                },
                0);
    }

    @Override
    public int set(int value, int delay, com.zeroc.Ice.Current current) {
        try {
            Thread.sleep(delay);
        } catch (InterruptedException ex) {
        }

        synchronized (this) {
            int tmp = _last;
            _last = value;
            return tmp;
        }
    }

    @Override
    public synchronized void setOneway(int value, int expected, com.zeroc.Ice.Current current) {
        test(_last == expected);
        _last = value;
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current) {
        _adapter.hold();
        _adapter.getCommunicator().shutdown();
    }

    private final java.util.Timer _timer;
    private final com.zeroc.Ice.ObjectAdapter _adapter;
    int _last = 0;
}
