// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Determines the order in which the Ice run time uses the endpoints in a proxy when establishing a
 * connection.
 */
public enum EndpointSelectionType {
    /** <code>Random</code> causes the endpoints to be arranged in a random order. */
    Random(0),
    /**
     * <code>Ordered</code> forces the Ice run time to use the endpoints in the order they appeared
     * in the proxy.
     */
    Ordered(1);

    /**
     * Returns the integer value of this endpoint selection type.
     *
     * @return the integer value of this endpoint selection type
     */
    public int value() {
        return _value;
    }

    /**
     * Returns the endpoint selection type corresponding to the given integer value.
     *
     * @param v the integer value of the endpoint selection type
     * @return the endpoint selection type corresponding to the given integer value, or {@code null}
     *     if no such type exists
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
