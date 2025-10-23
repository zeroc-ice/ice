// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.HashSet;
import java.util.concurrent.TimeUnit;

class RetryQueue {
    RetryQueue(Instance instance) {
        _instance = instance;
    }

    public synchronized void add(ProxyOutgoingAsyncBase outAsync, int interval) {
        if (_instance == null) {
            throw new CommunicatorDestroyedException();
        }
        RetryTask task = new RetryTask(_instance, this, outAsync);
        outAsync.cancelable(task); // This will throw if the request is canceled
        task.setFuture(_instance.timer().schedule(task, interval, TimeUnit.MILLISECONDS));
        _requests.add(task);
    }

    public synchronized void destroy() {
        if (_instance == null) {
            return; // Already destroyed.
        }

        HashSet<RetryTask> keep = new HashSet<>();
        for (RetryTask task : _requests) {
            if (!task.destroy()) {
                keep.add(task);
            }
        }
        _requests = keep;
        _instance = null;

        // Wait for the tasks to be executed, it shouldn't take long since they couldn't be canceled.
        // If interrupted, we preserve the interrupt.
        boolean interrupted = false;
        while (!_requests.isEmpty()) {
            try {
                wait();
            } catch (InterruptedException ex) {
                interrupted = true;
            }
        }
        if (interrupted) {
            Thread.currentThread().interrupt();
        }
    }

    synchronized boolean remove(RetryTask task) {
        boolean removed = _requests.remove(task);
        if (_instance == null && _requests.isEmpty()) {
            notify();
        }
        return removed;
    }

    private Instance _instance;
    private HashSet<RetryTask> _requests = new HashSet<>();
}
