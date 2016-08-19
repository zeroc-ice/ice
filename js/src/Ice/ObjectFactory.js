// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

class ObjectFactory
{
    create(type)
    {
        throw new Error("not implemented");
    }

    destroy()
    {
        throw new Error("not implemented");
    }
}

Ice.ObjectFactory = ObjectFactory;
module.exports.Ice = Ice;
