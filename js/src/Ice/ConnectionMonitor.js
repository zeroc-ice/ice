// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/Debug");
    require("Ice/ExUtil");
    
    var Ice = global.Ice || {};
    
    var Debug = Ice.Debug;
    var ExUtil = Ice.ExUtil;

    var ConnectionMonitor = Ice.Class({
        __init__: function(instance, interval)
        {
            this._instance = instance;
            this._interval = interval;
            this._scheduledInterval = 0;
            this._timerToken = -1;
            this._connections = [];
        },
        destroy: function()
        {
            Debug.assert(this._instance !== null);
            this._instance.timer().cancel(this._timerToken);
            this._instance = null;
            this._connections = null;
        },
        checkIntervalForACM: function(acmTimeout)
        {
            if(acmTimeout <= 0)
            {
                return;
            }

            //
            // If Ice.MonitorConnections isn't set (_interval == 0), the given ACM is used
            // to determine the check interval: 1/10 of the ACM timeout with a minmal value
            // of 5 seconds and a maximum value of 5 minutes.
            //
            // Note: if Ice.MonitorConnections is set, the timer is scheduled only if ACM
            // is configured for the communicator or some object adapters.
            //
            var interval;
            if(this._interval === 0)
            {
                interval = Math.floor(Math.min(300, Math.max(5, acmTimeout / 10)));
            }
            else if(this._scheduledInterval === this._interval)
            {
                return; // Nothing to do, the timer is already scheduled.
            }
            else
            {
                interval = this._interval;
            }

            //
            // If no timer is scheduled yet or if the given ACM requires a smaller interval,
            // we re-schedule the timer.
            //
            if(this._scheduledInterval === 0 || this._scheduledInterval > interval)
            {
                this._scheduledInterval = interval;
                this._instance.timer().cancel(this._timerToken);
                var self = this;
                this._timerToken = this._instance.timer().scheduleRepeated(
                    function() { self.runTimerTask(); }, interval * 1000);
            }
        },
        add: function(connection)
        {
            Debug.assert(this._instance !== null);
            this._connections.push(connection);
        },
        remove: function(connection)
        {
            Debug.assert(this._instance !== null);
            var pos = this._connections.indexOf(connection);
            Debug.assert(pos !== -1);
            this._connections.splice(pos, 1);
        },
        runTimerTask: function()
        {
            var now = Date.now();
            for(var i = 0; i < this._connections.length; ++i)
            {
                try
                {
                    this._connections[i].monitor(now);
                }
                catch(ex)
                {
                    if(this._instance === null)
                    {
                        return;
                    }
                    var msg = "exception in connection monitor:\n" + ExUtil.toString(ex);
                    this._instance.initializationData().logger.error(msg);
                }
            }
        }
    });
    
    Ice.ConnectionMonitor = ConnectionMonitor;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
