// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/**
 * Unknown sliced value holds an instance of an unknown Slice class type.
 **/
public final class UnknownSlicedValue: Value {

    private let unknownTypeId: String
    private var slicedData: SlicedData?

    public init() {
        self.unknownTypeId = ""
    }

    public init(unknownTypeId: String) {
        self.unknownTypeId = unknownTypeId
    }

    public init(from ins: InputStream) throws {
        ins.startValue()
        self.unknownTypeId = ""
        self.slicedData = try ins.endValue(preserve: true)
    }

    public func ice_id() -> String {
        return unknownTypeId
    }

    public func ice_getSlicedData() -> SlicedData? {
        return slicedData
    }

    public func ice_read(from ins: InputStream) throws {
        ins.startValue()
        slicedData = try ins.endValue(preserve: true)
    }

    public func ice_write(to os: OutputStream) {
        os.startValue(data: slicedData)
        os.endValue()
    }
}
