// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;
    var Promise = Ice.Promise;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var LoggerI = Ice.Class({
        __init__: function(out)
        {
            this._messages = [];
            this._out = out;
        },
        print: function(msg)
        {
            this._messages.push(msg);
        },
        trace: function(category, message)
        {
            this._messages.push("[" + category + "] " + message);
        },
        warning: function(message)
        {
            this._messages.push("warning: " + message);
        },
        error: function(message)
        {
            this._messages.push("error: " + message);
        },
        cloneWithPrefix: function(prefix)
        {
            return this;
        },
        dump: function()
        {
            for(var i = 0; i < this._messages.length; ++i)
            {
                this._out.writeLine(this._messages[i]);
            }
            this._messages = [];
        }
    });

    var TestCase = Ice.Class({
        __init__: function(name, com, out)
        {
            this._name = name;
            this._com = com;
            this._logger = new LoggerI(out);
            this._msg = null;

            this._clientACMTimeout = -1;
            this._clientACMClose = -1;
            this._clientACMHeartbeat = -1;

            this._serverACMTimeout = -1;
            this._serverACMClose = -1;
            this._serverACMHeartbeat = -1;

            this._heartbeat = 0;
            this._closed = false;
        },
        init: function()
        {
            var initData = new Ice.InitializationData();
            initData.properties = this._com.ice_getCommunicator().getProperties().clone();
            initData.logger = this._logger;
            initData.properties.setProperty("Ice.ACM.Timeout", "1");
            if(this._clientACMTimeout >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Timeout", "" + this._clientACMTimeout);
            }
            if(this._clientACMClose >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Close", "" + this._clientACMClose);
            }
            if(this._clientACMHeartbeat >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Heartbeat", "" + this._clientACMHeartbeat);
            }
            //initData.properties.setProperty("Ice.Trace.Protocol", "2");
            //initData.properties.setProperty("Ice.Trace.Network", "2");
            this._communicator = Ice.initialize(initData);

            var self = this;
            return this._com.createObjectAdapter(this._serverACMTimeout,
                                                 this._serverACMClose,
                                                 this._serverACMHeartbeat).then(function(adapter)
                                                                                {
                                                                                    self._adapter = adapter;
                                                                                });
        },
        destroy: function()
        {
            var self = this;
            return this._adapter.deactivate().then(function()
                                                   {
                                                       return self._communicator.destroy();
                                                   });
        },
        join: function(out)
        {
            this._logger.dump();
            out.write("testing " + this._name + "... ");
            if(this._msg === null)
            {
                out.writeLine("ok");
            }
            else
            {
                out.writeLine("failed! " + this._msg);
                test(false);
            }
        },
        start: function()
        {
            var proxy = null;
            var self = this;
            return this._adapter.getTestIntf().then(
                function(prx)
                {
                    proxy = Test.TestIntfPrx.uncheckedCast(self._communicator.stringToProxy(prx.toString()));
                    return proxy.ice_getConnection();
                }
            ).then(
                function(con)
                {
                    con.setCallback(self);
                    return self.runTestCase(self._adapter, proxy);
                }
            ).exception(
                function(ex)
                {
                    self._msg = "unexpected exception:\n" + ex.toString() + "\n" + ex.stack;
                }
            );
        },
        heartbeat: function(con)
        {
            ++this._heartbeat;
        },
        closed: function(con)
        {
            this._closed = true;
        },
        runTestCase: function(adapter, proxy)
        {
            test(false); // Abstract
        },
        setClientACM: function(timeout, close, heartbeat)
        {
            this._clientACMTimeout = timeout;
            this._clientACMClose = close;
            this._clientACMHeartbeat = heartbeat;
        },
        setServerACM: function(timeout, close, heartbeat)
        {
            this._serverACMTimeout = timeout;
            this._serverACMClose = close;
            this._serverACMHeartbeat = heartbeat;
        }
    });

    var InvocationHeartbeatTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "invocation heartbeat", com, out);
        },
        runTestCase: function(adapter, proxy)
        {
            var self = this;
            return proxy.sleep(2).then(
                function()
                {
                    test(self._heartbeat >= 2);
                }
            );
        }
    });

    var InvocationHeartbeatOnHoldTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "invocation with heartbeat on hold", com, out);
            // Use default ACM configuration.
        },
        runTestCase: function(adapter, proxy)
        {
            // When the OA is put on hold, connections shouldn't
            // send heartbeats, the invocation should therefore
            // fail.
            var self = this;
            return proxy.sleepAndHold(10).then(
                function()
                {
                    test(false);
                },
                function(ex)
                {
                    test(self._closed);
                    return adapter.activate().then(function()
                                                   {
                                                       return proxy.interruptSleep();
                                                   });
                }
            );
        }
    });

    var InvocationNoHeartbeatTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "invocation with no heartbeat", com, out);
            this.setServerACM(1, 2, 0); // Disable heartbeat on invocations
        },
        runTestCase: function(adapter, proxy)
        {
            // Heartbeats are disabled on the server, the
            // invocation should fail since heartbeats are
            // expected.
            var self = this;
            return proxy.sleep(10).then(
                function()
                {
                    test(false);
                },
                function(ex)
                {
                    test(self._heartbeat === 0);
                    test(self._closed);
                    return proxy.interruptSleep();
                }
            );
        }
    });

    var InvocationHeartbeatCloseOnIdleTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "invocation with no heartbeat and close on idle", com, out);
            this.setClientACM(1, 1, 0); // Only close on idle.
            this.setServerACM(1, 2, 0); // Disable heartbeat on invocations
        },
        runTestCase: function(adapter, proxy)
        {
            // No close on invocation, the call should succeed this
            // time.
            var self = this;
            return proxy.sleep(2).then(function()
                                       {
                                           test(self._heartbeat === 0);
                                           test(!self._closed);
                                       });
        }
    });

    var CloseOnIdleTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "close on idle", com, out);
            this.setClientACM(1, 1, 0); // Only close on idle
        },
        runTestCase: function(adapter, proxy)
        {
            var self = this;
            return Ice.Promise.delay(2000).then(function()
                                                {
                                                    test(self._heartbeat === 0);
                                                    test(self._closed);
                                                });
        }
    });

    var CloseOnInvocationTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "close on invocation", com, out);
            this.setClientACM(1, 2, 0); // Only close on invocation
        },
        runTestCase: function(adapter, proxy)
        {
            var self = this;
            return Ice.Promise.delay(1500).then(function()
                                                {
                                                    test(self._heartbeat === 0);
                                                    test(!self._closed);
                                                });
        }
    });

    var CloseOnIdleAndInvocationTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "close on idle and invocation", com, out);
            this.setClientACM(1, 3, 0); // Only close on idle and invocation
        },
        runTestCase: function(adapter, proxy)
        {
            //
            // Put the adapter on hold. The server will not respond to
            // the graceful close. This allows to test whether or not
            // the close is graceful or forceful.
            //
            var self = this;
            return adapter.hold().delay(1500).then(
                function()
                {
                    test(self._heartbeat === 0);
                    test(!self._closed); // Not closed yet because of graceful close.
                    return adapter.activate();
                }
            ).delay(500).then(
                function()
                {
                    test(self._closed); // Connection should be closed this time.
                }
            );
        }
    });

    var ForcefullCloseOnIdleAndInvocationTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "forcefull close on idle and invocation", com, out);
            this.setClientACM(1, 4, 0); // Only close on idle and invocation
        },
        runTestCase: function(adapter, proxy)
        {
            var self = this;
            return adapter.hold().delay(1500).then(
                function()
                {
                    test(self._heartbeat === 0);
                    test(self._closed); // Connection closed forcefully by ACM
                });
        }
    });

    var HeartbeatOnIdleTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "heartbeat on idle", com, out);
            this.setServerACM(1, -1, 2); // Enable server heartbeats.
        },
        runTestCase: function(adapter, proxy)
        {
            var self = this;
            return Ice.Promise.delay(2000).then(
                function()
                {
                    test(self._heartbeat >= 3);
                });
        }
    });

    var HeartbeatAlwaysTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "heartbeat always", com, out);
            this.setServerACM(1, -1, 3); // Enable server heartbeats.
        },
        runTestCase: function(adapter, proxy)
        {
            var self = this;
            var p = new Ice.Promise().succeed();

            // Use this function so we don't have a function defined
            // inside of a loop
            function icePing(prx)
            {
                return proxy.ice_ping();
            }

            for(var i = 0; i < 12; ++i)
            {
                p = p.then(icePing(proxy)).delay(200);
            }
            return p.then(function()
                          {
                              test(self._heartbeat >= 3);
                          });
        }
    });

    var SetACMTest = Ice.Class(TestCase, {
        __init__: function(com, out)
        {
            TestCase.call(this, "setACM/getACM", com, out);
            this.setClientACM(15, 4, 0);
        },
        runTestCase: function(adapter, proxy)
        {
            var acm = new Ice.ACM();
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout === 15);
            test(acm.close === Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat === Ice.ACMHeartbeat.HeartbeatOff);

            proxy.ice_getCachedConnection().setACM(undefined, undefined, undefined);
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout === 15);
            test(acm.close === Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat === Ice.ACMHeartbeat.HeartbeatOff);

            proxy.ice_getCachedConnection().setACM(1,
                                                   Ice.ACMClose.CloseOnInvocationAndIdle,
                                                   Ice.ACMHeartbeat.HeartbeatAlways);
            acm = proxy.ice_getCachedConnection().getACM();
            test(acm.timeout === 1);
            test(acm.close === Ice.ACMClose.CloseOnInvocationAndIdle);
            test(acm.heartbeat === Ice.ACMHeartbeat.HeartbeatAlways);

            return proxy.waitForHeartbeat(2);
        }
    });

    var allTests = function(out, communicator)
    {
        var ref = "communicator:default -p 12010";
        var com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

        var tests = [];
        //
        // Skip some tests with IE it opens too many connections and
        // IE doesn't allow more than 6 connections.
        //
        if(typeof(navigator) !== "undefined" &&
           (navigator.userAgent.indexOf("MSIE") != -1 ||
            navigator.userAgent.indexOf("Trident/7.0") != -1 ||
            navigator.userAgent.indexOf("Edge/12") != -1 ||
            navigator.userAgent.indexOf("Edge/13") != -1))
        {
            tests.push(new HeartbeatOnIdleTest(com, out));
            tests.push(new HeartbeatAlwaysTest(com, out));
            tests.push(new SetACMTest(com, out));
        }
        else
        {
            tests.push(new InvocationHeartbeatTest(com, out));
            tests.push(new InvocationHeartbeatOnHoldTest(com, out));
            tests.push(new InvocationNoHeartbeatTest(com, out));
            tests.push(new InvocationHeartbeatCloseOnIdleTest(com, out));

            tests.push(new CloseOnIdleTest(com, out));
            tests.push(new CloseOnInvocationTest(com, out));
            tests.push(new CloseOnIdleAndInvocationTest(com, out));
            tests.push(new ForcefullCloseOnIdleAndInvocationTest(com, out));

            tests.push(new HeartbeatOnIdleTest(com, out));
            tests.push(new HeartbeatAlwaysTest(com, out));
            tests.push(new SetACMTest(com, out));
        }

        var promises = [];
        for(var test in tests)
        {
            promises.push(tests[test].init());
        }

        return Ice.Promise.all(promises).then(
            function()
            {
                promises = [];
                for(var test in tests)
                {
                    promises.push(tests[test].start());
                }
                return Ice.Promise.all(promises);
            }
        ).then(
            function()
            {
                for(var test in tests)
                {
                    tests[test].join(out);
                }
            }
        ).then(
            function()
            {
                promises = [];
                for(var test in tests)
                {
                    promises.push(tests[test].destroy());
                }
                return Ice.Promise.all(promises);
            }
        ).then(
            function()
            {
                out.write("shutting down... ");
                return com.shutdown();
            }
        ).then(
            function()
            {
                out.writeLine("ok");
            }
        );
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.Warn.Connections", "0");
        var c = Ice.initialize(id);
        return Promise.try(
            function()
            {
                return allTests(out, c);
            }
        ).finally(
            function()
            {
                return c.destroy();
            }
        );
    };
    exports.__test__ = run;
    exports.__runServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
