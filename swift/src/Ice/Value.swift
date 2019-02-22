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
    static func ice_staticId() -> String
}

public extension Value {
    static func ice_staticId() -> String {
        return "::Ice::Object"
    }

    func ice_preMarshal() {}

    func ice_postUnmarshal() {}
}
