// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.Instrumentation;

/** The state of an Ice connection. */
public enum ConnectionState {
    /** The connection is being validated. */
    ConnectionStateValidating(0),
    /** The connection is holding the reception of new messages. */
    ConnectionStateHolding(1),
    /** The connection is active and can send and receive messages. */
    ConnectionStateActive(2),
    /**
     * The connection is being gracefully shutdown and waits for the peer to close its end of the
     * connection.
     */
    ConnectionStateClosing(3),
    /**
     * The connection is closed and waits for potential dispatch to be finished before being
     * destroyed and detached from the observer.
     */
    ConnectionStateClosed(4);

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
    public static ConnectionState valueOf(int v) {
        switch (v) {
            case 0:
                return ConnectionStateValidating;
            case 1:
                return ConnectionStateHolding;
            case 2:
                return ConnectionStateActive;
            case 3:
                return ConnectionStateClosing;
            case 4:
                return ConnectionStateClosed;
        }
        return null;
    }

    private ConnectionState(int v) {
        _value = v;
    }

    private final int _value;
}
