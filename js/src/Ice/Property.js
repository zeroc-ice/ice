//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

Ice.Property = class
{
    constructor(pattern, usesRegex, defaultValue, deprecated, deprecatedBy)
    {
        this._pattern = pattern;
        this._usesRegex = usesRegex;
        this._default = defaultValue;
        this._deprecated = deprecated;
        this._deprecatedBy = deprecatedBy;
    }

    get pattern()
    {
        return this._pattern;
    }

    get usesRegex()
    {
        return this._usesRegex;
    }

    get defaultValue()
    {
        return this._default;
    }

    get deprecated()
    {
        return this._deprecated;
    }

    get deprecatedBy()
    {
        return this._deprecatedBy;
    }
};

module.exports.Ice = Ice;
