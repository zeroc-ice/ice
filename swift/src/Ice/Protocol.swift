// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/**
 * SliceInfo encapsulates the details of a slice for an unknown class or exception type.
 **/
struct Protocol: OptionSet {
    let rawValue: UInt8

    static let FLAG_HAS_TYPE_ID_STRING = Protocol(rawValue: 1 << 0)
    static let FLAG_HAS_TYPE_ID_INDEX = Protocol(rawValue: 1 << 1)
    static let FLAG_HAS_TYPE_ID_COMPACT = Protocol(rawValue: 1 << 1 | 1 << 0)
    static let FLAG_HAS_OPTIONAL_MEMBERS = Protocol(rawValue: 1 << 2)
    static let FLAG_HAS_INDIRECTION_TABLE = Protocol(rawValue: 1 << 3)
    static let FLAG_HAS_SLICE_SIZE = Protocol(rawValue: 1 << 4)
    static let FLAG_IS_LAST_SLICE = Protocol(rawValue: 1 << 5)

    static let OPTIONAL_END_MARKER = Protocol(rawValue: 0xFF)

    static let Encoding_1_0 = Ice.EncodingVersion(major: 1, minor: 0)
    static let Encoding_1_1 = Ice.EncodingVersion(major: 1, minor: 1)

    static func checkSupportedEncoding(_ v: EncodingVersion) throws {
        let c = currentEncoding()
        if v.major != c.major || v.minor > c.minor {
            throw Ice.UnsupportedEncodingException(reason: "", bad: v, supported: c)
        }
    }
}
