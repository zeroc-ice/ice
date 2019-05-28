//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/// Unknown sliced value holds an instance of an unknown Slice class type.
public final class UnknownSlicedValue: Value {
    private let unknownTypeId: String
    private var slicedData: SlicedData?

    public required init() {
        unknownTypeId = ""
    }

    public init(unknownTypeId: String) {
        self.unknownTypeId = unknownTypeId
    }

    /// Returns the Slice type ID associated with this object.
    ///
    /// - returns: `String` - The type ID.
    public override func ice_id() -> String {
        return unknownTypeId
    }

    public override class func ice_staticId() -> String {
        return "::Ice::UnknownSlicedValue"
    }

    /// Returns the sliced data if the value has a preserved-slice base class and has been sliced during
    /// un-marshaling of the value, nil is returned otherwise.
    ///
    /// - returns: `Ice.SlicedData?` - The sliced data or nil.
    public override func ice_getSlicedData() -> SlicedData? {
        return slicedData
    }

    public override func _iceRead(from ins: InputStream) throws {
        ins.startValue()
        slicedData = try ins.endValue(preserve: true)
    }

    public override func _iceWrite(to os: OutputStream) {
        os.startValue(data: slicedData)
        os.endValue()
    }
}
