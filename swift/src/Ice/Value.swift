//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// The base class for all Ice values.
open class Value {
    public required init() {}

    /// Returns the Slice type ID of the most-derived interface supported by this object.
    ///
    /// - returns: `String` - The Slice type ID.
    open func ice_id() -> String {
        return "::Ice::Object"
    }

    open func _iceReadImpl(from _: InputStream) throws {}

    open func _iceWriteImpl(to _: OutputStream) {}

    /// The Ice run time invokes this method prior to marshaling an object's data members.
    /// This allows a subclass to override this method in order to validate its data
    /// members.
    open func ice_preMarshal() {}

    /// This Ice run time invokes this method after unmarshaling an object's data members. This allows a
    /// subclass to override this method in order to perform additional initialization.
    open func ice_postUnmarshal() {}

    /// Returns the sliced data if the value has a preserved-slice base class and has been sliced during
    /// un-marshaling of the value, nil is returned otherwise.
    ///
    /// - returns: `SlicedData?` - The sliced data or nil.
    open func ice_getSlicedData() -> SlicedData? {
        return nil
    }

    open func _iceRead(from istr: InputStream) throws {
        istr.startValue()
        try _iceReadImpl(from: istr)
        try istr.endValue(preserve: false)
    }

    open func _iceWrite(to os: OutputStream) {
        os.startValue(data: nil)
        _iceWriteImpl(to: os)
        os.endValue()
    }

    /// Returns the Slice type ID of this object.
    ///
    /// - returns: `String` - The Slice type ID.
    open class func ice_staticId() -> String {
        return "::Ice::Object"
    }
}

/// Helper class used to represent an interface passed by value. Note that
/// passing interface by values is deprecated.
open class InterfaceByValue: Value {
    private var id: String

    public required init() {
        precondition(false, "Not supported")
        id = ""
    }

    public init(id: String) {
        self.id = id
    }

    open override func ice_id() -> String {
        return id
    }

    open override func _iceReadImpl(from ostr: InputStream) throws {
        _ = try ostr.startSlice()
        try ostr.endSlice()
    }

    open override func _iceWriteImpl(to istr: OutputStream) {
        istr.startSlice(typeId: ice_id(), compactId: -1, last: true)
        istr.endSlice()
    }
}
