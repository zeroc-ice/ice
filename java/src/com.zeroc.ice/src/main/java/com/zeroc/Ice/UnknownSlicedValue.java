// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This class holds an instance of an unknown Slice-defined class type. */
public final class UnknownSlicedValue extends Value {
    /**
     * Constructs an UnknownSlicedValue with the specified type ID.
     *
     * @param unknownTypeId the Slice type ID of the unknown object
     */
    public UnknownSlicedValue(String unknownTypeId) {
        _unknownTypeId = unknownTypeId;
    }

    /**
     * Returns the Slice type ID associated with this object.
     *
     * @return the type ID
     */
    @Override
    public String ice_id() {
        return _unknownTypeId;
    }

    /** The Slice type ID of the unknown object. */
    private final String _unknownTypeId;

    private static final long serialVersionUID = -3199177633147630863L;
}
