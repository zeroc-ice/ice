// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/Debug", "../Ice/Connection", "../Ice/Debug"]);

var Class = Ice.Class;
var Debug = Ice.Debug;

var ACMConfig = Class({
    __init__: function(p, l, prefix, dflt)
    {
        if(p === undefined)
        {
            this.timeout = 60 * 1000;
            this.heartbeat = Ice.ACMHeartbeat.HeartbeatOnInvocation;
            this.close = Ice.ACMClose.CloseOnInvocationAndIdle;
            return;
        }

        var timeoutProperty;
        if((prefix == "Ice.ACM.Client" || prefix == "Ice.ACM.Server") &&
            p.getProperty(prefix + ".Timeout").length === 0)
        {
            timeoutProperty = prefix; // Deprecated property.
        }
        else
        {
            timeoutProperty = prefix + ".Timeout";
        }

        this.timeout = p.getPropertyAsIntWithDefault(timeoutProperty, dflt.timeout / 1000) * 1000; // To ms

        var hb = p.getPropertyAsIntWithDefault(prefix + ".Heartbeat", dflt.heartbeat.value);
        if(hb >= 0 && hb <= Ice.ACMHeartbeat.maxValue)
        {
            this.heartbeat = Ice.ACMHeartbeat.valueOf(hb);
        }
        else
        {
            l.warning("invalid value for property `" + prefix + ".Heartbeat" +
                        "', default value will be used instead");
            this.heartbeat = dflt.heartbeat;
        }

        var cl = p.getPropertyAsIntWithDefault(prefix + ".Close", dflt.close.value);
        if(cl >= 0 && cl <= Ice.ACMClose.maxValue)
        {
            this.close = Ice.ACMClose.valueOf(cl);
        }
        else
        {
            l.warning("invalid value for property `" + prefix + ".Close" +
                        "', default value will be used instead");
            this.close = dflt.close;
        }
    }
});

var ACMMonitor = Class({
    add: function(con)
    {
        Debug.assert(false); // Absract
    },
    remove: function(con)
    {
        Debug.assert(false); // Absract
    },
    reap: function(con)
    {
        Debug.assert(false); // Absract
    },
    acm: function(timeout)
    {
        Debug.assert(false); // Absract
        return null;
    },
    getACM: function()
    {
        Debug.assert(false); // Absract
        return 0;
    }
});

var FactoryACMMonitor = Class(ACMMonitor, {
    __init__: function(instance, config)
    {
        this._instance = instance;
        this._config = config;
        this._reapedConnections = [];
        this._connections = [];
    },
    destroy: function()
    {
        if(this._instance === null)
        {
            return;
        }
        this._instance = null;
    },
    add: function(connection)
    {
        if(this._config.timeout === 0)
        {
            return;
        }

        this._connections.push(connection);
        if(this._connections.length == 1)
        {
            var self = this;
            this._timerToken = this._instance.timer().scheduleRepeated(
                function()
                    {
                        self.runTimerTask();
                    },
                this._config.timeout / 2);
        }
    },
    remove: function(connection)
    {
        if(this._config.timeout === 0)
        {
            return;
        }

        var i = this._connections.indexOf(connection);
        Debug.assert(i >= 0);
        this._connections.splice(i, 1);
        if(this._connections.length === 0)
        {
            this._instance.timer().cancel(this._timerToken);
            return;
        }
    },
    reap: function(connection)
    {
        this._reapedConnections.push(connection);
    },
    acm: function(timeout, close, heartbeat)
    {
        Debug.assert(this._instance !== null);

        var config = new ACMConfig();
        config.timeout = this._config.timeout;
        config.close = this._config.close;
        config.heartbeat = this._config.heartbeat;
        if(timeout !== undefined)
        {
            config.timeout = timeout * 1000; // To milliseconds
        }
        if(close !== undefined)
        {
            config.close = close;
        }
        if(heartbeat !== undefined)
        {
            config.heartbeat = heartbeat;
        }
        return new ConnectionACMMonitor(this, this._instance.timer(), config);
    },
    getACM: function()
    {
        return new Ice.ACM(this._config.timeout / 1000, this._config.close, this._config.heartbeat);
    },
    swapReapedConnections: function()
    {
        if(this._reapedConnections.length === 0)
        {
            return null;
        }
        var connections = this._reapedConnections;
        this._reapedConnections = [];
        return connections;
    },
    runTimerTask: function()
    {
        if(this._instance === null)
        {
            this._connections = null;
            return;
        }

        //
        // Monitor connections outside the thread synchronization, so
        // that connections can be added or removed during monitoring.
        //
        var now = Date.now();
        for(var i = 0; i < this._connections.length; i++)
        {
            try
            {
                this._connections[i].monitor(now, this._config);
            }
            catch(ex)
            {
                this.handleException(ex);
            }
        }
    },
    handleException: function(ex)
    {
        if(this._instance === null)
        {
            return;
        }
        this._instance.initializationData().logger.error("exception in connection monitor:\n" + ex);
    }
});

var ConnectionACMMonitor = Class(ACMMonitor, {
    __init__: function(parent, timer, config)
    {
        this._parent = parent;
        this._timer = timer;
        this._config = config;
        this._connection = null;
    },
    add: function(connection)
    {
        Debug.assert(this._connection === null);
        this._connection = connection;
        if(this._config.timeout > 0)
        {
            var self = this;
            this._timerToken = this._timer.scheduleRepeated(function() { self.runTimerTask(); },
                                                            this._config.timeout / 2);
        }
    },
    remove: function(connection)
    {
        Debug.assert(this._connection === connection);
        this._connection = null;
        if(this._config.timeout > 0)
        {
            this._timer.cancel(this._timerToken);
        }
    },
    reap: function(connection)
    {
        this._parent.reap(connection);
    },
    acm: function(timeout, close, heartbeat)
    {
        return this._parent.acm(timeout, close, heartbeat);
    },
    getACM: function()
    {
        return new Ice.ACM(this._config.timeout / 1000, this._config.close, this._config.heartbeat);
    },
    runTimerTask: function()
    {
        try
        {
            this._connection.monitor(Date.now(), this._config);
        }
        catch(ex)
        {
            this._parent.handleException(ex);
        }
    }
});

Ice.FactoryACMMonitor = FactoryACMMonitor;
Ice.ACMConfig = ACMConfig;
module.exports.Ice = Ice;
