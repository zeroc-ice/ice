//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

let Ice = {};

if (typeof process !== 'undefined')
{
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
            let type = modules;
            for(let i = 0; i < components.length; ++i)
            {
                type = type[components[i]];
                if(type === undefined)
                {
                    return undefined;
                }
            }
            return type;
        }
    }

    Ice = _ModuleRegistry.module("Ice");
    Ice.Slice = Ice.Slice || {};
    Ice._ModuleRegistry = _ModuleRegistry;
}
else
{
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
            let type = root;
            for(let i = 0, length = components.length; i < length; ++i)
            {
                type = type[components[i]];
                if(type === undefined)
                {
                    return undefined;
                }
            }
            return type;
        }
    }

    Ice = _ModuleRegistry.module("Ice");

    Ice._require = function()
    {
        return root;
    };

    Ice.Slice = Ice.Slice || {};
    Ice._ModuleRegistry = _ModuleRegistry;
}

exports.Ice = Ice;
