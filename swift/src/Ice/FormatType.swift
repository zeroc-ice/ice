// Copyright (c) ZeroC, Inc.
/// Specifies the format for marshaling classes and exceptions with the Slice 1.1 encoding.
public enum FormatType: UInt8 {
    /// Favors compactness, but does not support slicing-off unknown slices during unmarshaling.
    case compactFormat = 0
    /// Allows slicing-off unknown slices during unmarshaling, at the cost of some extra space in the marshaled data.
    case slicedFormat = 1
}
