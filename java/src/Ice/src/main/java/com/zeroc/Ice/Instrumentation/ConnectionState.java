//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice.Instrumentation;

/**
 * The state of an Ice connection.
 **/
public enum ConnectionState
{
    /**
     * The connection is being validated.
     **/
    ConnectionStateValidating(0),
    /**
     * The connection is holding the reception of new messages.
     **/
    ConnectionStateHolding(1),
    /**
     * The connection is active and can send and receive messages.
     **/
    ConnectionStateActive(2),
    /**
     * The connection is being gracefully shutdown and waits for the
     * peer to close its end of the connection.
     **/
    ConnectionStateClosing(3),
    /**
     * The connection is closed and waits for potential dispatch to be
     * finished before being destroyed and detached from the observer.
     **/
    ConnectionStateClosed(4);

    public int value()
    {
        return _value;
    }

    public static ConnectionState valueOf(int v)
    {
        switch(v)
        {
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

    private ConnectionState(int v)
    {
        _value = v;
    }

    private final int _value;
}
