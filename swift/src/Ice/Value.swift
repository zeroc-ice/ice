// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public protocol Value: AnyObject {
    init()
    func ice_id() -> String
    func ice_preMarshal()
    func ice_postUnmarshal()
    func ice_getSlicedData() -> SlicedData?

    func _iceReadImpl(from: InputStream) throws
    func _iceWriteImpl(to: OutputStream)

    static func ice_staticId() -> String
}

public extension Value {
    func ice_id() -> String {
        return Self.ice_staticId()
    }

    func ice_getSlicedData() -> SlicedData? {
        return nil
    }

    func _iceRead(from istr: InputStream) throws {
        istr.startValue()
        try _iceReadImpl(from: istr)
        _ = try istr.endValue(preserve: false)
    }

    func _iceWrite(to os: OutputStream) {
        os.startValue(data: nil)
        _iceWriteImpl(to: os)
        os.endValue()
    }

    static func ice_staticId() -> String {
        precondition(false, "Abstract method")
    }
}

open class InterfaceByValue: Value {
    var id: String

    required public init() {
        preconditionFailure("Abstract method")
    }

    public init(id: String) {
        self.id = id
    }

    public func ice_id() -> String {
        return id
    }

    public func _iceReadImpl(from ostr: InputStream) throws {
        try ostr.startSlice()
        try ostr.endSlice()
    }

    public func _iceWriteImpl(to istr: OutputStream) {
        istr.startSlice(typeId: ice_id(), compactId: -1, last: true)
        istr.endSlice()
    }

    public func ice_preMarshal() {}
    public func ice_postUnmarshal() {}
}
