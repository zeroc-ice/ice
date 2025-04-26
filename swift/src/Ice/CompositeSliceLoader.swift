// Copyright (c) ZeroC, Inc.

/// TBD
public class CompositeSliceLoader: SliceLoader {
    private var sliceLoaders: [SliceLoader] = []

    public init(_ sliceLoaders: SliceLoader...) {
        self.sliceLoaders = sliceLoaders
    }

    public func add(_ sliceLoader: SliceLoader) {
        sliceLoaders.append(sliceLoader)
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
