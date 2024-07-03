// Copyright (c) ZeroC, Inc.

/// The base class for all Ice values.
open class Value {
    private var slicedData: SlicedData?

    public required init() {}

    /// Returns the Slice type ID of the most-derived interface supported by this object.
    ///
    /// - returns: `String` - The Slice type ID.
    open func ice_id() -> String {
        return ObjectTraits.staticId
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
        return slicedData
    }

    open func _iceRead(from istr: InputStream) throws {
        istr.startValue()
        try _iceReadImpl(from: istr)
        slicedData = try istr.endValue()
    }

    open func _iceWrite(to os: OutputStream) {
        os.startValue(data: slicedData)
        _iceWriteImpl(to: os)
        os.endValue()
    }

    /// Returns the Slice type ID of this object.
    ///
    /// - returns: `String` - The Slice type ID.
    open class func ice_staticId() -> String {
        return ObjectTraits.staticId
    }
}
