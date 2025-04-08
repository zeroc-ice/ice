// Copyright (c) ZeroC, Inc.

package test.Ice.hold;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectAdapterDeactivatedException;

import test.Ice.hold.Test.Hold;

import java.util.Timer;
import java.util.TimerTask;

public final class HoldI implements Hold {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    HoldI(Timer timer, ObjectAdapter adapter) {
        _timer = timer;
        _adapter = adapter;
        _last = 0;
    }

    @Override
    public void putOnHold(int delay, Current current) {
        if (delay < 0) {
            _adapter.hold();
        } else if (delay == 0) {
            _adapter.hold();
            _adapter.activate();
        } else {
            _timer.schedule(
                new TimerTask() {
                    @Override
                    public void run() {
                        try {
                            putOnHold(0, null);
                        } catch (ObjectAdapterDeactivatedException ex) {
                        }
                    }
                },
                delay);
        }
    }

    @Override
    public void waitForHold(final Current current) {
        _timer.schedule(
            new TimerTask() {
                @Override
                public void run() {
                    try {
                        current.adapter.waitForHold();

                        current.adapter.activate();
                    } catch (LocalException ex) {
                        // This shouldn't occur. The test ensures all the waitForHold timers are
                        // finished before shutting down the communicator.
                        test(false);
                    }
                }
            },
            0);
    }

    @Override
    public int set(int value, int delay, Current current) {
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
    public synchronized void setOneway(int value, int expected, Current current) {
        test(_last == expected);
        _last = value;
    }

    @Override
    public void shutdown(Current current) {
        _adapter.hold();
        _adapter.getCommunicator().shutdown();
    }

    private final Timer _timer;
    private final ObjectAdapter _adapter;
    int _last;
}
