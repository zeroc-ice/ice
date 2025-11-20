// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Represents an instance of an unknown class. */
public final class UnknownSlicedValue extends Value {
    /**
     * Constructs the placeholder instance.
     *
     * @param unknownTypeId the Slice type ID of the unknown value
     */
    public UnknownSlicedValue(String unknownTypeId) {
        _unknownTypeId = unknownTypeId;
    }

    /**
     * Returns the Slice type ID associated with this instance.
     *
     * @return the type ID supplied to the constructor
     */
    @Override
    public String ice_id() {
        return _unknownTypeId;
    }

    /** The Slice type ID of the unknown value. */
    private final String _unknownTypeId;

    private static final long serialVersionUID = -3199177633147630863L;
}
