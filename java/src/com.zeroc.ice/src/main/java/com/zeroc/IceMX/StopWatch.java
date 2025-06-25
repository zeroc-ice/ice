// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

/**
 * A simple stopwatch for measuring elapsed time in microseconds.
 */
class StopWatch {
    /**
     * Starts the stopwatch.
     */
    public void start() {
        _s = System.nanoTime();
    }

    /**
     * Stops the stopwatch and returns the elapsed time.
     *
     * @return the elapsed time in microseconds
     */
    public long stop() {
        assert (isStarted());
        long d = (System.nanoTime() - _s) / 1000;
        _s = 0;
        return d;
    }

    /**
     * Checks whether the stopwatch is currently running.
     *
     * @return true if the stopwatch is started, false otherwise
     */
    public boolean isStarted() {
        return _s != 0;
    }

    /**
     * Gets the current elapsed time without stopping the stopwatch.
     *
     * @return the elapsed time in microseconds since start was called
     */
    public long delay() {
        return (System.nanoTime() - _s) / 1000;
    }

    private long _s;
}
