//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

open class Value {
    public required init() {}

    open func ice_id() -> String {
        return "::Ice::Value"
    }

    open func _iceReadImpl(from _: InputStream) throws {}

    open func _iceWriteImpl(to _: OutputStream) {}

    open func ice_preMarshal() {}

    open func ice_postUnmarshal() {}

    open func ice_getSlicedData() -> SlicedData? {
        return nil
    }

    open func _iceRead(from istr: InputStream) throws {
        istr.startValue()
        try _iceReadImpl(from: istr)
        _ = try istr.endValue(preserve: false)
    }

    open func _iceWrite(to os: OutputStream) {
        os.startValue(data: nil)
        _iceWriteImpl(to: os)
        os.endValue()
    }

    open class func ice_staticId() -> String {
        return "::Ice::Value"
    }
}

open class InterfaceByValue: Value {
    var id: String

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
