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
public struct SliceInfo {
    /**
     * The Slice type ID for this slice.
     **/
    public let typeId: String

    /**
     * The Slice compact type ID for this slice.
     **/
    public let compactId: Int32

    /**
     * The encoded bytes for this slice, including the leading size integer.
     **/
    public let bytes: [UInt8]

    /**
     * The class instances referenced by this slice.
     **/
    public var instances: [Ice.Value]

    /**
     * Whether or not the slice contains optional members.
     **/
    public let hasOptionalMembers: Bool

    /**
     * Whether or not this is the last slice.
     **/
    public let isLastSlice: Bool
}
