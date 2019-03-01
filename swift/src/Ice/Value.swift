// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public protocol Value: StreamableValue, AnyObject {
    func ice_id() -> String
    func ice_preMarshal()
    func ice_postUnmarshal()
    func ice_getSlicedData() -> SlicedData?

    func iceReadImpl(from: InputStream) throws
    func iceWriteImpl(to: OutputStream)

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

    func ice_read(from: InputStream) throws {
        from.startValue()
        try iceReadImpl(from: from)
        _ = try from.endValue(preserve: false)
    }

    func ice_write(to: OutputStream) {
        to.startValue(data: nil)
        iceWriteImpl(to: to)
        to.endValue()
    }
}
