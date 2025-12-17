// Copyright (c) ZeroC, Inc.

import Foundation

/// Encapsulates the details of a class slice with an unknown type.
public struct SliceInfo {
    /// The Slice type ID for this slice.
    public let typeId: String

    /// The Slice compact type ID for this slice, or `-1` if the slice has no compact ID.
    public let compactId: Int32

    /// The encoded bytes for this slice, including the leading size integer.
    public let bytes: Data

    /// The class instances referenced by this slice.
    public var instances: [Ice.Value]

    /// Whether or not the slice contains optional members.
    public let hasOptionalMembers: Bool

    /// Whether or not this is the last slice.
    public let isLastSlice: Bool
}
