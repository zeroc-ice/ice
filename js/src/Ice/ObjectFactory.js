// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/Class").Ice;
Ice.ObjectFactory = Ice.Class({
    create: function(type)
    {
        throw new Error("not implemented");
    },
    destroy: function()
    {
        throw new Error("not implemented");
    }
});
module.exports.Ice = Ice;
