// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/AsyncResultBase",
        "../Ice/LocalException",
        "../Ice/ObjectAdapterI",
        "../Ice/Promise",
        "../Ice/UUID"
    ]);

var AsyncResultBase = Ice.AsyncResultBase;
var ObjectAdapterI = Ice.ObjectAdapterI;
var Promise = Ice.Promise;

//
// Only for use by Instance.
//
var ObjectAdapterFactory = Ice.Class({
    __init__: function(instance, communicator)
    {
        this._instance = instance;
        this._communicator = communicator;
        this._adapters = [];
        this._adapterNamesInUse = [];
        this._shutdownPromise = new Promise();
    },
    shutdown: function()
    {
        //
        // Ignore shutdown requests if the object adapter factory has
        // already been shut down.
        //
        if(this._instance === null)
        {
            return this._shutdownPromise;
        }

        this._instance = null;
        this._communicator = null;
        this._shutdownPromise = Promise.all(
            this._adapters.map(function(adapter)
                               {
                                   return adapter.deactivate();
                               }));
        return this._shutdownPromise;
    },
    waitForShutdown: function()
    {
        var self = this;
        return this._shutdownPromise.then(
            function()
            {
                return Promise.all(self._adapters.map(function(adapter)
                                                      {
                                                          return adapter.waitForDeactivate();
                                                      }));
            });
    },
    isShutdown: function()
    {
        return this._instance === null;
    },
    destroy: function()
    {
        var self = this;
        return this.waitForShutdown().then(
            function()
            {
                return Promise.all(self._adapters.map(function(adapter)
                                                      {
                                                          return adapter.destroy();
                                                      }));
            });
    },
    createObjectAdapter: function(name, router, promise)
    {
        if(this._instance === null)
        {
            throw new Ice.ObjectAdapterDeactivatedException();
        }

        var adapter = null;
        try
        {
            if(name.length === 0)
            {
                var uuid = Ice.generateUUID();
                adapter = new ObjectAdapterI(this._instance, this._communicator, this, uuid, null, true, promise);
            }
            else
            {
                if(this._adapterNamesInUse.indexOf(name) !== -1)
                {
                    throw new Ice.AlreadyRegisteredException("object adapter", name);
                }
                adapter = new ObjectAdapterI(this._instance, this._communicator, this, name, router, false, promise);
                this._adapterNamesInUse.push(name);
            }
            this._adapters.push(adapter);
        }
        catch(ex)
        {
            promise.fail(ex, promise);
        }
    },
    removeObjectAdapter: function(adapter)
    {
        if(this._instance === null)
        {
            return;
        }

        var n = this._adapters.indexOf(adapter);
        if(n !== -1)
        {
            this._adapters.splice(n, 1);
        }

        n = this._adapterNamesInUse.indexOf(adapter.getName());
        if(n !== -1)
        {
            this._adapterNamesInUse.splice(n, 1);
        }
    }
});

Ice.ObjectAdapterFactory = ObjectAdapterFactory;
module.exports.Ice = Ice;
