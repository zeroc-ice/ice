// Copyright (c) ZeroC, Inc.

// The proxyTypes maps contains entries for proxy types. To support forward declarations of interface types.
const proxyTypes = new Map();

// The valueTypes maps contains entries for value types (types derived from Ice.Value). To support forward declarations
// of classes.
const valueTypes = new Map();

export class TypeRegistry {
    static declareProxyType(name, type) {
        if (proxyTypes.get(name) === undefined) {
            proxyTypes.set(name, type);
        }
    }

    static declareValueType(name, type) {
        if (valueTypes.get(name) === undefined) {
            valueTypes.set(name, type);
        }
        return type;
    }

    static getProxyType(name) {
        return proxyTypes.get(name);
    }

    static getValueType(name) {
        return valueTypes.get(name);
    }
}
