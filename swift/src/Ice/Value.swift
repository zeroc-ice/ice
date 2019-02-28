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

    func iceReadImpl(from: InputStream) throws;
    func iceWriteImpl(to: OutputStream);

    static func ice_staticId() -> String
}

public extension Value {
    
    public func ice_id() -> String {
        return Self.ice_staticId()
    }

    public func ice_preMarshal() {
    }

    public func ice_postUnmarshal() {
    }
    
    public func ice_getSlicedData() -> SlicedData?{
        return nil;
    }
    
    public func ice_read(from: InputStream) throws {
        from.startValue();
        try self.iceReadImpl(from: from);
        _ = try from.endValue(preserve: false);
    }
    
    public func ice_write(to: OutputStream) {
        to.startValue(data: nil);
        self.iceWriteImpl(to: to);
        to.endValue();
    }
}
