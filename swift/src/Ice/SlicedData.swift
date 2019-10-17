//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/// SlicedData holds the slices of unknown class or exception types.
public final class SlicedData {
    /// The details of each slice, in order of most-derived to least-derived.
    public private(set) var slices: [SliceInfo]

    init(slices: [SliceInfo]) {
        self.slices = slices
    }

    /// Clears the slices to break potential cyclic references.
    public func clear() {
        let copy = slices
        slices = [] // need to clear the slices before the loop to avoid recursive calls
        for slice in copy {
            for value in slice.instances {
                value.ice_getSlicedData()?.clear()
            }
        }
    }
}
