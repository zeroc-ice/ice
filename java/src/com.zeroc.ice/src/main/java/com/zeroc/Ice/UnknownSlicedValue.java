// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Unknown sliced value holds an instance of an unknown Slice class type. */
public final class UnknownSlicedValue extends Value {
    /**
     * Represents an instance of a Slice class type having the given Slice type.
     *
     * @param unknownTypeId The Slice type ID of the unknown object.
     */
    public UnknownSlicedValue(String unknownTypeId) {
        _unknownTypeId = unknownTypeId;
    }

    /**
     * Determine the Slice type ID associated with this object.
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
