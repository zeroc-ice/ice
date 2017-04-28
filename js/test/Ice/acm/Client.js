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

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    class LoggerI
    {
        constructor(out)
        {
            this._messages = [];
            this._out = out;
        }

        print(msg)
        {
            this._messages.push(msg);
        }

        trace(category, message)
        {
            this._messages.push("[" + category + "] " + message);
        }

        warning(message)
        {
            this._messages.push("warning: " + message);
        }

        error(message)
        {
            this._messages.push("error: " + message);
        }

        cloneWithPrefix(prefix)
        {
            return this;
        }

        dump()
        {
            this._messages.forEach(message => this._out.writeLine(message));
            this._messages = [];
        }
    }

    class TestCase
    {
        constructor(name, com, out)
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
        }

        init()
        {
            var initData = new Ice.InitializationData();
            initData.properties = this._com.ice_getCommunicator().getProperties().clone();
            initData.logger = this._logger;
            initData.properties.setProperty("Ice.ACM.Timeout", "2");
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

            return this._com.createObjectAdapter(this._serverACMTimeout,
                                                 this._serverACMClose,
                                                 this._serverACMHeartbeat).then(adapter =>
                                                                                {
                                                                                    this._adapter = adapter;
                                                                                });
        }

        destroy()
        {
            return this._adapter.deactivate().then(() => this._communicator.destroy());
        }

        join(out)
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
        }

        start()
        {
            return this._adapter.getTestIntf().then(prx =>
                {
                    prx = Test.TestIntfPrx.uncheckedCast(this._communicator.stringToProxy(prx.toString()));
                    return prx.ice_getConnection().then(con =>
                        {
                            con.setCloseCallback(connection => this._closed = true);

                            con.setHeartbeatCallback(connection => ++this._heartbeat);

                            return this.runTestCase(this._adapter, prx);
                        }).catch(ex =>
                            {
                                this._msg = "unexpected exception:\n" + ex.toString() + "\n" + ex.stack;
                            });
                });
        }

        waitForClosed()
        {
            if(!this._closed)
            {
                var now = Date.now();
                var promise = Ice.Promise.delay(100);
                var p = promise;
                for(var i = 0; i < 20; ++i)
                {
                    p = p.then(() => {
                        if(this._closed)
                        {
                            return;
                        }
                        else if(Date.now() - now >= 2000)
                        {
                            test(false);
                        }
                        else
                        {
                            return Ice.Promise.delay(100);
                        }
                    });
                }
                return p;
            }
            return Ice.Promise.resolve();
        }

        runTestCase(adapter, proxy)
        {
            test(false); // Abstract
        }

        setClientACM(timeout, close, heartbeat)
        {
            this._clientACMTimeout = timeout;
            this._clientACMClose = close;
            this._clientACMHeartbeat = heartbeat;
        }

        setServerACM(timeout, close, heartbeat)
        {
            this._serverACMTimeout = timeout;
            this._serverACMClose = close;
            this._serverACMHeartbeat = heartbeat;
        }
    }

    class InvocationHeartbeatTest extends TestCase
    {
        constructor(com, out)
        {
            super("invocation heartbeat", com, out);
        }

        runTestCase(adapter, proxy)
        {
            return proxy.sleep(4).then(() =>
                {
                    test(this._heartbeat >= 2);
                });
        }
    }

    class InvocationHeartbeatOnHoldTest extends TestCase
    {
        constructor(com, out)
        {
            super("invocation with heartbeat on hold", com, out);
            // Use default ACM configuration.
        }

        runTestCase(adapter, proxy)
        {
            // When the OA is put on hold, connections shouldn't
            // send heartbeats, the invocation should therefore
            // fail.
            return proxy.sleepAndHold(10).then(
                () => test(false),
                ex => adapter.activate())
                    .then(() => proxy.interruptSleep())
                    .then(() => this.waitForClosed());
        }
    }

    class InvocationNoHeartbeatTest extends TestCase
    {
        constructor(com, out)
        {
            super("invocation with no heartbeat", com, out);
            this.setServerACM(2, 2, 0); // Disable heartbeat on invocations
        }

        runTestCase(adapter, proxy)
        {
            // Heartbeats are disabled on the server, the
            // invocation should fail since heartbeats are
            // expected.
            return proxy.sleep(10).then(
                () => test(false),
                ex =>
                {
                    return proxy.interruptSleep();
                }).then(() => this.waitForClosed())
            .then(() => {
                test(this._heartbeat === 0);
            });
        }
    }

    class InvocationHeartbeatCloseOnIdleTest extends TestCase
    {
        constructor(com, out)
        {
            super("invocation with no heartbeat and close on idle", com, out);
            this.setClientACM(1, 1, 0); // Only close on idle.
            this.setServerACM(1, 2, 0); // Disable heartbeat on invocations
        }

        runTestCase(adapter, proxy)
        {
            // No close on invocation, the call should succeed this
            // time.
            return proxy.sleep(3).then(() =>
                                       {
                                           test(this._heartbeat === 0);
                                           test(!this._closed);
                                       });
        }
    }

    class CloseOnIdleTest extends TestCase
    {
        constructor(com, out)
        {
            super("close on idle", com, out);
            this.setClientACM(1, 1, 0); // Only close on idle
        }

        runTestCase(adapter, proxy)
        {
            return Ice.Promise.delay(3000).then(() => this.waitForClosed()).then(() =>
                                                {
                                                    test(this._heartbeat === 0);
                                                    test(this._closed);
                                                });
        }
    }

    class CloseOnInvocationTest extends TestCase
    {
        constructor(com, out)
        {
            super("close on invocation", com, out);
            this.setClientACM(1, 2, 0); // Only close on invocation
        }

        runTestCase(adapter, proxy)
        {
            return Ice.Promise.delay(3000).then(() =>
                                                {
                                                    test(this._heartbeat === 0);
                                                    test(!this._closed);
                                                });
        }
    }

    class CloseOnIdleAndInvocationTest extends TestCase
    {
        constructor(com, out)
        {
            super("close on idle and invocation", com, out);
            this.setClientACM(1, 3, 0); // Only close on idle and invocation
        }

        runTestCase(adapter, proxy)
        {
            //
            // Put the adapter on hold. The server will not respond to
            // the graceful close. This allows to test whether or not
            // the close is graceful or forceful.
            //
            return adapter.hold().delay(3000).then(
                () =>
                {
                    test(this._heartbeat === 0);
                    test(!this._closed); // Not closed yet because of graceful close.
                    return adapter.activate();
                }).delay(500).then(() => this.waitForClosed()); // Connection should be closed this time.
        }
    }

    class ForcefullCloseOnIdleAndInvocationTest extends TestCase
    {
        constructor(com, out)
        {
            super("forcefull close on idle and invocation", com, out);
            this.setClientACM(1, 4, 0); // Only close on idle and invocation
        }

        runTestCase(adapter, proxy)
        {
            return adapter.hold().delay(3000).then(() => this.waitForClosed()).then(
                () =>
                {
                    test(this._heartbeat === 0);
                    test(this._closed); // Connection closed forcefully by ACM
                });
        }
    }

    class HeartbeatOnIdleTest extends TestCase
    {
        constructor(com, out)
        {
            super("heartbeat on idle", com, out);
            this.setServerACM(1, -1, 2); // Enable server heartbeats.
        }

        runTestCase(adapter, proxy)
        {
            return Ice.Promise.delay(3000).then(() => test(this._heartbeat >= 3));
        }
    }

    class HeartbeatAlwaysTest extends TestCase
    {
        constructor(com, out)
        {
            super("heartbeat always", com, out);
            this.setServerACM(1, -1, 3); // Enable server heartbeats.
        }

        runTestCase(adapter, proxy)
        {
            var p = Ice.Promise.resolve();

            // Use this function so we don't have a function defined
            // inside of a loop
            function icePing(prx)
            {
                return proxy.ice_ping();
            }

            for(var i = 0; i < 10; ++i)
            {
                p = p.then(icePing(proxy)).delay(300);
            }
            return p.then(() => test(this._heartbeat >= 3));
        }
    }

    class HeartbeatManualTest extends TestCase
    {
        constructor(com, out)
        {
            super("manual heartbeats", com, out);
            //
            // Disable heartbeats.
            //
            this.setClientACM(10, -1, 0);
            this.setServerACM(10, -1, 0);
        }

        runTestCase(adapter, proxy)
        {
            function sendHeartbeats(con)
            {
                var p = Ice.Promise.resolve();
                for(var i = 0; i < 5; ++i)
                {
                    p = p.then(con.heartbeat());
                }
                return p;
            }

            return proxy.startHeartbeatCount().then(
                () => proxy.ice_getConnection()).then(
                    con => sendHeartbeats(con)).then(
                        () => proxy.waitForHeartbeatCount(5));
        }
    }

    class SetACMTest extends TestCase
    {
        constructor(com, out)
        {
            super("setACM/getACM", com, out);
            this.setClientACM(15, 4, 0);
        }

        runTestCase(adapter, proxy)
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

            return proxy.startHeartbeatCount().then(() => proxy.waitForHeartbeatCount(2));
        }
    }

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
            tests.push(new HeartbeatManualTest(com, out));
            tests.push(new SetACMTest(com, out));
        }

        return Ice.Promise.all(tests.map(test => test.init())
            ).then(() => Ice.Promise.all(tests.map(test => test.start()))
            ).then(() => tests.forEach(test => test.join(out))
            ).then(() => Ice.Promise.all(tests.map(test => test.destroy()))
            ).then(() =>
                {
                    out.write("shutting down... ");
                    return com.shutdown();
                }
            ).then(() => out.writeLine("ok"));
    };

    var run = function(out, id)
    {
        id.properties.setProperty("Ice.Warn.Connections", "0");
        var c = Ice.initialize(id);
        return Ice.Promise.try(() => allTests(out, c)).finally(() => c.destroy());
    };
    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
