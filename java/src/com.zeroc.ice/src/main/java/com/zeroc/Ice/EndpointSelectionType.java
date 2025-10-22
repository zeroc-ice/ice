// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Determines how the Ice runtime sorts proxy endpoints when establishing a connection. */
public enum EndpointSelectionType {
    /** The Ice runtime shuffles the endpoints in a random order. */
    Random(0),

    /** The Ice runtime uses the endpoints in the order they appear in the proxy. */
    Ordered(1);

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
    public static EndpointSelectionType valueOf(int v) {
        switch (v) {
            case 0:
                return Random;
            case 1:
                return Ordered;
        }
        return null;
    }

    private EndpointSelectionType(int v) {
        _value = v;
    }

    private final int _value;
}
