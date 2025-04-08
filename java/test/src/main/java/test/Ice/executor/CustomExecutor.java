// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

import com.zeroc.Ice.Connection;

import java.util.LinkedList;
import java.util.Queue;
import java.util.concurrent.Executor;
import java.util.function.BiConsumer;

public class CustomExecutor
    implements Runnable,
    BiConsumer<Runnable, Connection>,
    Executor {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public CustomExecutor() {
        _thread = new Thread(this);
        _thread.start();
    }

    @Override
    public void run() {
        while (true) {
            Runnable call = null;
            synchronized (this) {
                if (!_terminated && _calls.isEmpty()) {
                    try {
                        wait();
                    } catch (InterruptedException ex) {
                    }
                }

                if (!_calls.isEmpty()) {
                    call = _calls.poll();
                } else if (_terminated) {
                    // Terminate only once all calls are dispatched.
                    return;
                }
            }

            if (call != null) {
                try {
                    call.run();
                } catch (Exception ex) {
                    // Exceptions should never propagate here.
                    ex.printStackTrace();
                    test(false);
                }
            }
        }
    }

    @Override
    public synchronized void accept(Runnable call, Connection con) {
        boolean added = _calls.offer(call);
        assert added;
        if (_calls.size() == 1) {
            notify();
        }
    }

    @Override
    public void execute(Runnable call) {
        accept(call, null);
    }

    public void terminate() {
        synchronized (this) {
            _terminated = true;
            notify();
        }
        while (true) {
            try {
                _thread.join();
                break;
            } catch (InterruptedException ex) {
            }
        }
    }

    public boolean isCustomExecutorThread() {
        return Thread.currentThread() == _thread;
    }

    private final Queue<Runnable> _calls = new LinkedList<>();
    private final Thread _thread;
    private boolean _terminated;
}
