// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public struct SlicedData {
    //
    // The details of each slice, in order of most-derived to least-derived.
    //
    public let slices: [SliceInfo]

    public init (slices: [SliceInfo]) {
        self.slices = slices
    }
}
