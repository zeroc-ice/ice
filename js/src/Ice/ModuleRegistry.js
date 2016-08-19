// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const __modules__ = {};

class __M
{
    static module(name)
    {
        let m =  __modules__[name];
        if(m === undefined)
        {
            m = {};
            __modules__[name] =  m;
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
        let T = __modules__;

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

const Ice = __M.module("Ice");
Ice.Slice = Ice.Slice || {};
Ice.__M = __M;
exports.Ice = Ice;
