// Copyright (c) ZeroC, Inc.

export class CompositeSliceLoader {
    constructor(sliceLoaders = []) {
        this.sliceLoaders = sliceLoaders;
    }

    add(loader) {
        this.sliceLoaders.push(loader);
    }

    newInstance(typeId) {
        for (const loader of this.sliceLoaders) {
            let instance = loader.newInstance(typeId);
            if (instance !== null) {
                return instance;
            }
        }
        return null;
    }
}
