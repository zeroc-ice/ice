//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
const Ice = _ModuleRegistry.require(module,
                                    [
                                        "../Ice/Struct",
                                        "../Ice/Long",
                                        "../Ice/HashMap",
                                        "../Ice/HashUtil",
                                        "../Ice/ArrayUtil",
                                        "../Ice/StreamHelpers"
                                    ]).Ice;

const Slice = Ice.Slice;

/**
 * A version structure for the protocol version.
 *
 **/
Ice.ProtocolVersion = class
{
    constructor(major = 0, minor = 0)
    {
        this.major = major;
        this.minor = minor;
    }

    _write(ostr)
    {
        ostr.writeByte(this.major);
        ostr.writeByte(this.minor);
    }

    _read(istr)
    {
        this.major = istr.readByte();
        this.minor = istr.readByte();
    }

    static get minWireSize()
    {
        return  2;
    }
};

Slice.defineStruct(Ice.ProtocolVersion, true, false);

/**
 * A version structure for the encoding version.
 *
 **/
Ice.EncodingVersion = class
{
    constructor(major = 0, minor = 0)
    {
        this.major = major;
        this.minor = minor;
    }

    _write(ostr)
    {
        ostr.writeByte(this.major);
        ostr.writeByte(this.minor);
    }

    _read(istr)
    {
        this.major = istr.readByte();
        this.minor = istr.readByte();
    }

    static get minWireSize()
    {
        return  2;
    }
};

Slice.defineStruct(Ice.EncodingVersion, true, false);

exports.Ice = Ice;
