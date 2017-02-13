// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/LocalException").Ice;

const AlreadyRegisteredException = Ice.AlreadyRegisteredException;
const NotRegisteredException = Ice.NotRegisteredException;

//
// Only for use by Instance
//
class ValueFactoryManagerI
{
    constructor()
    {
        this._factoryMap = new Map(); // Map<String, ValueFactory>
    }

    add(factory, id)
    {
        if(this._factoryMap.has(id))
        {
            throw new AlreadyRegisteredException("value factory", id);
        }
        this._factoryMap.set(id, factory);
    }

    find(id)
    {
        return this._factoryMap.get(id);
    }

    destroy()
    {
        this._factoryMap = new Map(); // Map<String, ValueFactory>
    }
}

Ice.ValueFactoryManagerI = ValueFactoryManagerI;
module.exports.Ice = Ice;
