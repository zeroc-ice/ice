//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

Ice.Property = class
{
    constructor(pattern, defaultValue, deprecated, deprecatedBy)
    {
        this._pattern = pattern;
        this._default = defaultValue;
        this._deprecated = deprecated;
        this._deprecatedBy = deprecatedBy;
    }

    get pattern()
    {
        return this._pattern;
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
