// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Represents an instance of an unknown class. */
public final class UnknownSlicedValue extends Value {
    /**
     * Constructs the placeholder instance.
     *
     * @param unknownTypeId the Slice type ID of the unknown value, or the string form of the
     *     compact type ID (for example, "1") when the most-derived slice was marshaled with a
     *     compact type ID
     */
    public UnknownSlicedValue(String unknownTypeId) {
        _unknownTypeId = unknownTypeId;
    }

    /**
     * Returns the Slice type ID associated with this instance.
     *
     * @return the type ID supplied to the constructor; it's the string form of the compact type ID
     *     (for example, "1") when the most-derived slice was marshaled with a compact type ID
     */
    @Override
    public String ice_id() {
        return _unknownTypeId;
    }

    /** The Slice type ID of the unknown value. */
    private final String _unknownTypeId;

    private static final long serialVersionUID = -3199177633147630863L;
}
