// Copyright (c) ZeroC, Inc. All rights reserved.

export class Property {
    constructor(pattern, usesRegex, defaultValue, deprecated, propertyArray) {
        this.pattern = pattern;
        this.usesRegex = usesRegex;
        this.defaultValue = defaultValue;
        this.deprecated = deprecated;
        this.propertyArray = propertyArray;
    }
}

export class PropertyArray {
    constructor(name, prefixOnly, isOptIn, properties) {
        this.name = name;
        this.prefixOnly = prefixOnly;
        this.isOptIn = isOptIn;
        this.properties = properties;
    }
}
