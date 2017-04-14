// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* global
    self : false
*/
const root = typeof(window) !== "undefined" ? window : 
             typeof(global) !== "undefined" ? global : 
             typeof(self) !== "undefined" ? self : {};
/* global
    self : true
*/

class _ModuleRegistry
{
    static module(name)
    {
        var m = root[name];
        if(m === undefined)
        {
            m = {};
            root[name] =  m;
        }
        return m;
    }
    
    static require(name)
    {
        return root;
    }
    
    static type(scoped)
    {
        if(scoped === undefined)
        {
            return undefined;
        }
        var components = scoped.split(".");
        var T = root;

        for(var i = 0, length = components.length; i < length; ++i)
        {
            T = T[components[i]];
            if(T === undefined)
            {
                return undefined;
            }
        }
        return T;
    }
}

const Ice = _ModuleRegistry.module("Ice");

Ice._require = function()
{
    return root;
};

Ice.Slice = Ice.Slice || {};
Ice._ModuleRegistry = _ModuleRegistry;
