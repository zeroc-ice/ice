//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Connection");
require("../Ice/Debug");

const Debug = Ice.Debug;

class ACMConfig
{
    constructor(p, l, prefix, dflt)
    {
        if(p === undefined)
        {
            this.timeout = 60 * 1000;
            this.heartbeat = Ice.ACMHeartbeat.HeartbeatOnDispatch;
            this.close = Ice.ACMClose.CloseOnInvocationAndIdle;
            return;
        }

        let timeoutProperty;
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
        if(this.timeout < 0)
        {
            l.warning("invalid value for property `" + timeoutProperty + "', default value will be used instead");
            this.timeout = dflt.timeout;
        }

        const hb = p.getPropertyAsIntWithDefault(prefix + ".Heartbeat", dflt.heartbeat.value);
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

        const cl = p.getPropertyAsIntWithDefault(prefix + ".Close", dflt.close.value);
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
}

class FactoryACMMonitor
{
    constructor(instance, config)
    {
        this._instance = instance;
        this._config = config;
        this._reapedConnections = [];
        this._connections = [];
    }

    destroy()
    {
        if(this._instance === null)
        {
            return;
        }
        this._instance = null;
    }

    add(connection)
    {
        if(this._config.timeout === 0)
        {
            return;
        }

        this._connections.push(connection);
        if(this._connections.length == 1)
        {
            this._timerToken = this._instance.timer().scheduleRepeated(
                () => this.runTimerTask(), this._config.timeout / 2);
        }
    }

    remove(connection)
    {
        if(this._config.timeout === 0)
        {
            return;
        }

        const i = this._connections.indexOf(connection);
        Debug.assert(i >= 0);
        this._connections.splice(i, 1);
        if(this._connections.length === 0)
        {
            this._instance.timer().cancel(this._timerToken);
        }
    }

    reap(connection)
    {
        this._reapedConnections.push(connection);
    }

    acm(timeout, close, heartbeat)
    {
        Debug.assert(this._instance !== null);

        const config = new ACMConfig();
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
    }

    getACM()
    {
        return new Ice.ACM(this._config.timeout / 1000, this._config.close, this._config.heartbeat);
    }

    swapReapedConnections()
    {
        if(this._reapedConnections.length === 0)
        {
            return null;
        }
        const connections = this._reapedConnections;
        this._reapedConnections = [];
        return connections;
    }

    runTimerTask()
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
        const now = Date.now();
        this._connections.forEach(connection =>
            {
                try
                {
                    connection.monitor(now, this._config);
                }
                catch(ex)
                {
                    this.handleException(ex);
                }
            });
    }

    handleException(ex)
    {
        if(this._instance === null)
        {
            return;
        }
        this._instance.initializationData().logger.error("exception in connection monitor:\n" + ex);
    }
}

class ConnectionACMMonitor
{
    constructor(parent, timer, config)
    {
        this._parent = parent;
        this._timer = timer;
        this._config = config;
        this._connection = null;
    }

    add(connection)
    {
        Debug.assert(this._connection === null);
        this._connection = connection;
        if(this._config.timeout > 0)
        {
            this._timerToken = this._timer.scheduleRepeated(() => this.runTimerTask(), this._config.timeout / 2);
        }
    }

    remove(connection)
    {
        Debug.assert(this._connection === connection);
        this._connection = null;
        if(this._config.timeout > 0)
        {
            this._timer.cancel(this._timerToken);
        }
    }

    reap(connection)
    {
        this._parent.reap(connection);
    }

    acm(timeout, close, heartbeat)
    {
        return this._parent.acm(timeout, close, heartbeat);
    }

    getACM()
    {
        return new Ice.ACM(this._config.timeout / 1000, this._config.close, this._config.heartbeat);
    }

    runTimerTask()
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
}

Ice.FactoryACMMonitor = FactoryACMMonitor;
Ice.ACMConfig = ACMConfig;
module.exports.Ice = Ice;
