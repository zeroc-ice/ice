// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/HashMap", "../Ice/LocalException"]);

var HashMap = Ice.HashMap;
var AlreadyRegisteredException = Ice.AlreadyRegisteredException;
var NotRegisteredException = Ice.NotRegisteredException;

//
// Only for use by Instance
//
var ValueFactoryManagerI = Ice.Class({
    __init__: function()
    {
        this._factoryMap = new HashMap(); // Map<String, ValueFactory>
    },
    add: function(factory, id)
    {
        var o, ex;
        o = this._factoryMap.get(id);
        if(o !== undefined)
        {
            ex = new AlreadyRegisteredException();
            ex.id = id;
            ex.kindOfObject = "value factory";
            throw ex;
        }
        this._factoryMap.set(id, factory);
    },
    find: function(id)
    {
        return this._factoryMap.get(id);
    },
    destroy: function()
    {
        this._factoryMap = new HashMap(); // Map<String, ValueFactory>
    }
});

Ice.ValueFactoryManagerI = ValueFactoryManagerI;
module.exports.Ice = Ice;
