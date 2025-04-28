// Copyright (c) ZeroC, Inc.

/// Implements SliceLoader by combining multiple SliceLoaders.
public final class CompositeSliceLoader: SliceLoader {
    private var sliceLoaders: [SliceLoader] = []

    /// Creates a CompositeSliceLoader.
    /// - Parameter sliceLoaders: The initial Slice loaders.
    public init(_ sliceLoaders: SliceLoader...) {
        self.sliceLoaders = sliceLoaders
    }

    /// Adds a SliceLoader to the CompositeSliceLoader.
    /// Parameter loader: The SliceLoader to add.
    public func add(_ loader: SliceLoader) {
        sliceLoaders.append(loader)
    }

    public func newInstance(_ typeId: String) -> AnyObject? {
        for sliceLoader in sliceLoaders {
            if let instance = sliceLoader.newInstance(typeId) {
                return instance
            }
        }
        return nil
    }
}
