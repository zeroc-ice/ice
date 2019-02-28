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

    func ice_preMarshal() {}

    func ice_postUnmarshal() {}

    func ice_getSlicedData() -> SlicedData? {
        return nil
    }

    func _iceRead(from: InputStream) throws {
        from.startValue()
        try _iceReadImpl(from: from)
        _ = try from.endValue(preserve: false)
    }

    func _iceWrite(to: OutputStream) {
        to.startValue(data: nil)
        _iceWriteImpl(to: to)
        to.endValue()
    }
}
