// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.Instrumentation;

/** The thread state enumeration keeps track of the different possible states of Ice threads. */
public enum ThreadState {
    /** The thread is idle. */
    ThreadStateIdle(0),
    /**
     * The thread is in use performing reads or writes for Ice connections. This state is only for
     * threads from an Ice thread pool.
     */
    ThreadStateInUseForIO(1),
    /**
     * The thread is calling user code (servant implementation, AMI callbacks). This state is only
     * for threads from an Ice thread pool.
     */
    ThreadStateInUseForUser(2),
    /** The thread is performing other internal activities (DNS lookups, timer callbacks, etc). */
    ThreadStateInUseForOther(3);

    /**
     * Returns the integer value of this enumerator.
     *
     * @return the integer value of this enumerator
     */
    public int value() {
        return _value;
    }

    /**
     * Returns the enumerator corresponding to the given integer value.
     *
     * @param v the integer value of the enumerator
     * @return the enumerator corresponding to the given integer value, or {@code null} if no such enumerator exists
     */
    public static ThreadState valueOf(int v) {
        switch (v) {
            case 0:
                return ThreadStateIdle;
            case 1:
                return ThreadStateInUseForIO;
            case 2:
                return ThreadStateInUseForUser;
            case 3:
                return ThreadStateInUseForOther;
        }
        return null;
    }

    private ThreadState(int v) {
        _value = v;
    }

    private final int _value;
}
