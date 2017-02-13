// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Property = function Property(pattern, deprecated, deprecatedBy)
{
    this._pattern = pattern;
    this._deprecated = deprecated;
    this._deprecatedBy = deprecatedBy;
};

Object.defineProperty(Property.prototype, "pattern",{
    get: function() { return this._pattern; }
});

Object.defineProperty(Property.prototype, "deprecated",{
    get: function() { return this._deprecated; }
});

Object.defineProperty(Property.prototype, "deprecatedBy",{
    get: function() { return this._deprecatedBy; }
});

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.Property = Property;
module.exports.Ice = Ice;
