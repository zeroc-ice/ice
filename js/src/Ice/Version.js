// Copyright (c) ZeroC, Inc.

export const Ice = {};

/**
 *  A version structure for the protocol version.
 **/
Ice.ProtocolVersion = class {
    constructor(major = 0, minor = 0) {
        this.major = major;
        this.minor = minor;
    }

    _write(ostr) {
        ostr.writeByte(this.major);
        ostr.writeByte(this.minor);
    }

    _read(istr) {
        this.major = istr.readByte();
        this.minor = istr.readByte();
    }

    static get minWireSize() {
        return 2;
    }
};

// Moved to VersionExtensions.js to avoid circular dependencies.
// defineStruct(ProtocolVersion, true, false);

/**
 *  A version structure for the encoding version.
 **/
Ice.EncodingVersion = class {
    constructor(major = 0, minor = 0) {
        this.major = major;
        this.minor = minor;
    }

    _write(ostr) {
        ostr.writeByte(this.major);
        ostr.writeByte(this.minor);
    }

    _read(istr) {
        this.major = istr.readByte();
        this.minor = istr.readByte();
    }

    static get minWireSize() {
        return 2;
    }
};

// Moved to VersionExtensions.js to avoid circular dependencies.
// defineStruct(EncodingVersion, true, false);
