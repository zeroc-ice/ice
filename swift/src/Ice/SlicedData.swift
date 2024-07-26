// Copyright (c) ZeroC, Inc.

/// Holds class slices that cannot be unmarshaled because their types are not known locally.
public final class SlicedData {
    /// The details of each slice, in order of most-derived to least-derived.
    public private(set) var slices: [SliceInfo]

    init(slices: [SliceInfo]) {
        self.slices = slices
    }

    /// Clears the slices to break potential cyclic references.
    public func clear() {
        let copy = slices
        slices = []  // need to clear the slices before the loop to avoid recursive calls
        for slice in copy {
            for value in slice.instances {
                value.ice_getSlicedData()?.clear()
            }
        }
    }
}
