//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/LocalException").Ice;

const AlreadyRegisteredException = Ice.AlreadyRegisteredException;

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
