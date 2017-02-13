// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;

Ice.TraceLevels = function(properties)
{
    var networkCat = "Network";
    var protocolCat = "Protocol";
    var retryCat = "Retry";
    var locationCat = "Locator";
    var slicingCat = "Slicing";

    var keyBase = "Ice.Trace.";

    var network = properties.getPropertyAsInt(keyBase + networkCat);
    var protocol = properties.getPropertyAsInt(keyBase + protocolCat);
    var retry = properties.getPropertyAsInt(keyBase + retryCat);
    var location = properties.getPropertyAsInt(keyBase + locationCat);
    var slicing = properties.getPropertyAsInt(keyBase + slicingCat);
    properties.getPropertyAsInt(keyBase + "ThreadPool"); // Avoid an "unused property" warning.

    return Object.create(null, {
        'network': {
            get: function() { return network; }
        },
        'networkCat': {
            get: function() { return networkCat; }
        },
        'protocol': {
            get: function() { return protocol; }
        },
        'protocolCat': {
            get: function() { return protocolCat; }
        },
        'retry': {
            get: function() { return retry; }
        },
        'retryCat': {
            get: function() { return retryCat; }
        },
        'location': {
            get: function() { return location; }
        },
        'locationCat': {
            get: function() { return locationCat; }
        },
        'slicing': {
            get: function() { return slicing; }
        },
        'slicingCat': {
            get: function() { return slicingCat; }
        }
    });
};
module.exports.Ice = Ice;
