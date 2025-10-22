// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Represents batch compression options when flushing queued batch requests. */
public enum CompressBatch {
    /** Compress the batch requests. */
    Yes(0),

    /** Don't compress the batch requests. */
    No(1),

    /** Compress the batch requests if at least one request was made on a compressed proxy. */
    BasedOnProxy(2);

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
    public static CompressBatch valueOf(int v) {
        return switch (v) {
            case 0 -> Yes;
            case 1 -> No;
            case 2 -> BasedOnProxy;
            default -> null;
        };
    }

    private CompressBatch(int v) {
        _value = v;
    }

    private final int _value;
}
