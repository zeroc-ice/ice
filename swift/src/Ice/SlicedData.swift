//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public struct SlicedData {
    //
    // The details of each slice, in order of most-derived to least-derived.
    //
    public let slices: [SliceInfo]

    public init(slices: [SliceInfo]) {
        self.slices = slices
    }
}
