//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* global
    self : false
*/
const root = typeof window !== "undefined" ? window :
             typeof global !== "undefined" ? global :
             typeof self !== "undefined" ? self : {};
/* global
    self : true
*/

class _ModuleRegistry
{
    static module(name)
    {
        let m = root[name];
        if(m === undefined)
        {
            m = {};
            root[name] = m;
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
        const components = scoped.split(".");
        let T = root;

        for(let i = 0, length = components.length; i < length; ++i)
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
