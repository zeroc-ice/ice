//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const modules = {};

class _ModuleRegistry
{
    static module(name)
    {
        let m = modules[name];
        if(m === undefined)
        {
            m = {};
            modules[name] = m;
        }
        return m;
    }

    static require(m, paths)
    {
        let o;
        paths.forEach(path =>
            {
                o = m.require(path);
            });
        return o;
    }

    static type(scoped)
    {
        if(scoped === undefined)
        {
            return undefined;
        }

        const components = scoped.split(".");
        let T = modules;

        for(let i = 0; i < components.length; ++i)
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
Ice.Slice = Ice.Slice || {};
Ice._ModuleRegistry = _ModuleRegistry;
exports.Ice = Ice;
