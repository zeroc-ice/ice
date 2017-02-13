// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;

Ice.Class = function()
{
    var base;
    var desc;
    var constructor;
    
    if(arguments.length == 1)
    {
        desc = arguments[0];
    }
    else if(arguments.length == 2)
    {
        base = arguments[0];
        desc = arguments[1];
    }

    if(desc !== undefined)
    {
        constructor = desc.__init__;
        if(constructor)
        {
            delete desc.__init__;
        }
    }
    
    var o = constructor || function(){};

    if(base !== undefined)
    {
        o.prototype = new base();
        o.prototype.constructor = o;
    }

    if(desc !== undefined)
    {
        for(var key in desc)
        {
            o.prototype[key] = desc[key];
        }
    }
    return o;
};

module.exports.Ice = Ice;
