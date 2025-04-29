// Copyright (c) ZeroC, Inc.

export class CompositeSliceLoader {
    constructor(loaderArray = []) {
        this._loaderArray = loaderArray;
    }

    newInstance(typeId) {
        for (const loader of this._loaderArray) {
            let instance = loader.newInstance(typeId);
            if (instance !== null) {
                return instance;
            }
        }
        return null;
    }

    add(loader) {
        this._loaderArray.push(loader);
    }
}
