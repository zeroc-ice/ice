// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** The batch compression option when flushing queued batch requests. */
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
        switch (v) {
            case 0:
                return Yes;
            case 1:
                return No;
            case 2:
                return BasedOnProxy;
        }
        return null;
    }

    private CompressBatch(int v) {
        _value = v;
    }

    private final int _value;
}
