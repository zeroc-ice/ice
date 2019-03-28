// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

open class Value {

    public required init() {}

    open func ice_id() -> String {
        return "::Ice::Value"
    }

    open func _iceReadImpl(from: InputStream) throws {}

    open func _iceWriteImpl(to: OutputStream) {}

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
        self.id = ""
    }
    public init(id: String) {
        self.id = id
    }

    override open func ice_id() -> String {
        return id
    }

    override open func _iceReadImpl(from ostr: InputStream) throws {
        _ = try ostr.startSlice()
        try ostr.endSlice()
    }

    override open func _iceWriteImpl(to istr: OutputStream) {
        istr.startSlice(typeId: ice_id(), compactId: -1, last: true)
        istr.endSlice()
    }
}
