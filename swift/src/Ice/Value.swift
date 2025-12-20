// Copyright (c) ZeroC, Inc.

/// The base class for instances of Slice-defined classes.
open class Value {
    private var slicedData: SlicedData?

    public required init() {}

    /// Returns the Slice type ID of this type.
    ///
    /// - Returns: The return value is always `"::Ice::Object"`.
    open class func ice_staticId() -> String { "::Ice::Object" }

    /// Returns the Slice type ID of the most-derived class supported by this object.
    ///
    /// - Returns: The Slice type ID.
    open func ice_id() -> String { type(of: self).ice_staticId() }

    open func _iceReadImpl(from _: InputStream) throws {}

    open func _iceWriteImpl(to _: OutputStream) {}

    /// The Ice runtime calls this method before marshaling a class instance.
    /// Subclasses can override this method in order to update or validate their fields before marshaling.
    open func ice_preMarshal() {}

    /// The Ice runtime calls this method after unmarshaling a class instance.
    /// Subclasses can override this method in order to update or validate their fields after unmarshaling.
    open func ice_postUnmarshal() {}

    /// Gets the sliced data associated with this instance.
    ///
    /// - Returns: The sliced data if the value has a preserved-slice base class and has been sliced during
    /// unmarshaling of the value, `nil` otherwise.
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
}
