// Copyright (c) ZeroC, Inc.

/// Represents an instance of an unknown class.
public final class UnknownSlicedValue: Value {
    private let unknownTypeId: String

    public required init() {
        fatalError("UnknownSlicedValue must be initialized with an id")
    }

    public init(unknownTypeId: String) {
        self.unknownTypeId = unknownTypeId
    }

    /// Returns the Slice type ID associated with this instance.
    ///
    /// - Returns: The type ID supplied to the constructor.
    override public func ice_id() -> String {
        return unknownTypeId
    }

    /// Returns the Slice type ID of this unknown sliced value.
    ///
    /// - Returns: The return value is always `"::Ice::UnknownSlicedValue"`.
    override public class func ice_staticId() -> String {
        return "::Ice::UnknownSlicedValue"
    }
}
