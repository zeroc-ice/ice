//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

Ice.Property = class
{
    constructor(pattern, deprecated, deprecatedBy)
    {
        this._pattern = pattern;
        this._deprecated = deprecated;
        this._deprecatedBy = deprecatedBy;
    }

    get pattern()
    {
        return this._pattern;
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
