// Copyright (c) ZeroC, Inc.

/**
 * Implements SliceLoader using generated classes that register themselves by calling defineClass.
 */
class DefaultSliceLoader {
    /**
     * Creates a new DefaultSliceLoader.
     */
    constructor() {
        this.typeIdToTypeMap = new Map(); // Map<String, Type>
    }

    /**
     * Adds a new type to the typeIdToTypeMap.
     * @param typeId The type ID or compact type ID.
     * @param type The type.
     */
    add(typeId, type) {
        this.typeIdToTypeMap.set(typeId, type);
    }

    newInstance(typeId) {
        const type = this.typeIdToTypeMap.get(typeId);
        if (type === undefined) {
            return null;
        }
        return new type();
    }
}

/**
 * The DefaultSliceLoader singleton.
 */
export const defaultSliceLoaderInstance = new DefaultSliceLoader();

/**
 * Registers the class for a mapped Slice class or exception. Also adds some methods to the JS class.
 * @param classType The JS class.
 * @param typeId The type ID.
 * @param compactId The compact type ID.
 */
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
