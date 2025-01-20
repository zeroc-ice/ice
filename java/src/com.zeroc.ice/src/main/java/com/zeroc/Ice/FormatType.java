// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This enumeration describes the possible formats for classes and exceptions. */
public enum FormatType {
    /** A minimal format that eliminates the possibility for slicing unrecognized types. */
    CompactFormat,
    /** Allow slicing and preserve slices for unknown types. */
    SlicedFormat
}
