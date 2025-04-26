// Copyright (c) ZeroC, Inc.

/// Creates class and exception instances from Slice type IDs.
public protocol SliceLoader {
    /// Creates an instance of a class mapped from a Slice class or exception based on a Slice type ID.
    /// - Parameter typeId: The Slice type ID or compact type ID.
    /// - Returns: A new instance of the class or exception identified by typeId, or nil if the implementation cannot
    /// find the corresponding class.
    func newInstance(_ typeId: String) -> AnyObject?
}
