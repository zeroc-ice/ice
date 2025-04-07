// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Current;

import test.Ice.maxDispatches.Test.Responder;

import java.util.ArrayList;
import java.util.List;

class ResponderI implements Responder {
    private final List<Runnable> _responses = new ArrayList<>();
    private boolean _started;

    @Override
    public void start(Current current) {
        ArrayList<Runnable> responses;

        synchronized (this) {
            _started = true;
            responses = new ArrayList<>(_responses);
            _responses.clear();
        }

        for (var response : responses) {
            response.run();
        }
    }

    @Override
    public synchronized void stop(Current current) {
        _started = false;
    }

    @Override
    public synchronized int pendingResponseCount(Current current) {
        return _responses.size();
    }

    void queueResponse(Runnable response) {
        boolean queued = false;

        synchronized (this) {
            if (!_started) {
                _responses.add(response);
                queued = true;
            }
        }

        if (!queued) {
            response.run();
        }
    }
}
