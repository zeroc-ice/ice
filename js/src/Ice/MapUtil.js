// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

class MapUtil
{
    static equals(m1, m2)
    {
        if(m1 === m2)
        {
            return true;
        }
        else if(m1.size != m2.size)
        {
            return false;
        }
        else
        {
            for(let [key, value] of m1)
            {
                if(value === undefined)
                {
                    if(!m2.has(key))
                    {
                        return false;
                    }
                    else if(m2.get(key) !== value)
                    {
                        return false;
                    }
                }
                else if(m2.get(key) !== value)
                {
                    return false;
                }
            }
        }
        return true;
    }
}

Ice.MapUtil = MapUtil;

module.exports.Ice = Ice;
