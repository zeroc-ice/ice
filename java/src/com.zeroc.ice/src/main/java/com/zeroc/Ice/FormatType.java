// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Specifies the format for marshaling classes and exceptions with the Slice 1.1 encoding. */
public enum FormatType {
    /** Favors compactness, but does not support slicing-off unknown slices during unmarshaling. */
    CompactFormat,
    /** Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the marshaled data. */
    SlicedFormat
}
