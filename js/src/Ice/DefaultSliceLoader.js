// Copyright (c) ZeroC, Inc.

class DefaultSliceLoader {
    constructor() {
        this._typeIdToTypeMap = new Map(); // Map<String, Type>
    }

    newInstance(typeId) {
        const type = this._typeIdToTypeMap.get(typeId);
        if (type === undefined) {
            return null;
        }
        return new type();
    }

    add(typeId, type) {
        this._typeIdToTypeMap.set(typeId, type);
    }
}

export const defaultSliceLoaderInstance = new DefaultSliceLoader();

// Register the class for a mapped Slice class or exception. Also adds some methods to the JS class.
export function defineClass(classType, typeId, compactId = -1) {
    classType.prototype.ice_id = function () {
        return typeId;
    };

    classType.prototype._iceMostDerivedType = function () {
        return classType;
    };

    classType.ice_staticId = function () {
        return typeId;
    };

    defaultSliceLoaderInstance.add(typeId, classType);

    if (compactId !== -1) {
        defaultSliceLoaderInstance.add(compactId.toString(), classType);
    }
}
