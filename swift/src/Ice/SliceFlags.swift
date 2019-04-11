// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

struct SliceFlags: OptionSet {
    let rawValue: UInt8

    static let FLAG_HAS_TYPE_ID_STRING = SliceFlags(rawValue: 1 << 0)
    static let FLAG_HAS_TYPE_ID_INDEX = SliceFlags(rawValue: 1 << 1)
    static let FLAG_HAS_TYPE_ID_COMPACT = SliceFlags(rawValue: 1 << 1 | 1 << 0)
    static let FLAG_HAS_OPTIONAL_MEMBERS = SliceFlags(rawValue: 1 << 2)
    static let FLAG_HAS_INDIRECTION_TABLE = SliceFlags(rawValue: 1 << 3)
    static let FLAG_HAS_SLICE_SIZE = SliceFlags(rawValue: 1 << 4)
    static let FLAG_IS_LAST_SLICE = SliceFlags(rawValue: 1 << 5)

    static let OPTIONAL_END_MARKER = SliceFlags(rawValue: 0xFF)
}
