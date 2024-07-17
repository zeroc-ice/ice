// Copyright (c) ZeroC, Inc.

/// Unknown sliced value holds an instance of an unknown Slice class type.
public final class UnknownSlicedValue: Value {
    private let unknownTypeId: String

    public required init() {
        fatalError("UnknownSlicedValue must be initialized with an id")
    }

    public init(unknownTypeId: String) {
        self.unknownTypeId = unknownTypeId
    }

    /// Returns the Slice type ID associated with this object.
    ///
    /// - returns: `String` - The type ID.
    override public func ice_id() -> String {
        return unknownTypeId
    }

    override public class func ice_staticId() -> String {
        return "::Ice::UnknownSlicedValue"
    }
}
