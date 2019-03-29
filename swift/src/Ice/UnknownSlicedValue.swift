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

    public required init() {
        unknownTypeId = ""
    }

    public init(unknownTypeId: String) {
        self.unknownTypeId = unknownTypeId
    }

    override public func ice_id() -> String {
        return unknownTypeId
    }

    public override class func ice_staticId() -> String {
        return "::Ice::UnknownSlicedValue"
    }

    override public func ice_getSlicedData() -> SlicedData? {
        return slicedData
    }

    override public func _iceRead(from ins: InputStream) throws {
        ins.startValue()
        slicedData = try ins.endValue(preserve: true)
    }

    override public func _iceWrite(to os: OutputStream) {
        os.startValue(data: slicedData)
        os.endValue()
    }
}
