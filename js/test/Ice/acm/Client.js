// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    function test(value)
    {
        if(!value)
        {
            throw new Error("test failed");
        }
    }

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

        async init()
        {
            const initData = new Ice.InitializationData();
            initData.properties = this._com.ice_getCommunicator().getProperties().clone();
            initData.logger = this._logger;
            initData.properties.setProperty("Ice.ACM.Timeout", "2");
            if(this._clientACMTimeout >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Timeout", String(this._clientACMTimeout));
            }
            if(this._clientACMClose >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Close", String(this._clientACMClose));
            }
            if(this._clientACMHeartbeat >= 0)
            {
                initData.properties.setProperty("Ice.ACM.Client.Heartbeat", String(this._clientACMHeartbeat));
            }
            this._communicator = Ice.initialize(initData);

            this._adapter = await this._com.createObjectAdapter(this._serverACMTimeout,
                                                                this._serverACMClose,
                                                                this._serverACMHeartbeat);
        }

        async destroy()
        {
            await this._adapter.deactivate();
            await this._communicator.destroy();
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

        async start()
        {
            try
            {
                let prx = await this._adapter.getTestIntf();
                prx = Test.TestIntfPrx.uncheckedCast(this._communicator.stringToProxy(prx.toString()));
                const con = await prx.ice_getConnection();
                con.setCloseCallback(connection =>
                                     {
                                         this._closed = true;
                                     });
                con.setHeartbeatCallback(connection => ++this._heartbeat);
                await this.runTestCase(this._adapter, prx);
            }
            catch(ex)
            {
                this._msg = "unexpected exception:\n" + ex.toString() + "\n" + ex.stack;
            }
        }

        async waitForClosed()
        {
            const now = Date.now();
            while(!this._closed)
            {
                await Ice.Promise.delay(100);
                if(Date.now() - now >= 2000)
                {
                    test(false); // Waited for more than 2s for close, something's wrong.
                }
            }
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
            this.setServerACM(1, -1, -1); // Faster ACM to make sure we receive enough ACM heartbeats
        }

        async runTestCase(adapter, proxy)
        {
            await proxy.sleep(4);
            test(this._heartbeat >= 4);
        }
    }

    class InvocationHeartbeatOnHoldTest extends TestCase
    {
        constructor(com, out)
        {
            super("invocation with heartbeat on hold", com, out);
            // Use default ACM configuration.
        }

        async runTestCase(adapter, proxy)
        {
            // When the OA is put on hold, connections shouldn't
            // send heartbeats, the invocation should therefore
            // fail.
            try
            {
                await proxy.sleepAndHold(10);
                test(false);
            }
            catch(ex)
            {
                await adapter.activate();
                await proxy.interruptSleep();
                await this.waitForClosed();
            }
        }
    }

    class InvocationNoHeartbeatTest extends TestCase
    {
        constructor(com, out)
        {
            super("invocation with no heartbeat", com, out);
            this.setServerACM(2, 2, 0); // Disable heartbeat on invocations
        }

        async runTestCase(adapter, proxy)
        {
            // Heartbeats are disabled on the server, the
            // invocation should fail since heartbeats are
            // expected.
            try
            {
                await proxy.sleep(10);
                test(false);
            }
            catch(ex)
            {
                await proxy.interruptSleep();
                await this.waitForClosed();
                test(this._heartbeat === 0);
            }
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

        async runTestCase(adapter, proxy)
        {
            // No close on invocation, the call should succeed this
            // time.
            await proxy.sleep(3);
            test(this._heartbeat === 0);
            test(!this._closed);
        }
    }

    class CloseOnIdleTest extends TestCase
    {
        constructor(com, out)
        {
            super("close on idle", com, out);
            this.setClientACM(1, 1, 0); // Only close on idle
        }

        async runTestCase(adapter, proxy)
        {
            await Ice.Promise.delay(3000);
            await this.waitForClosed();
            test(this._heartbeat === 0);
        }
    }

    class CloseOnInvocationTest extends TestCase
    {
        constructor(com, out)
        {
            super("close on invocation", com, out);
            this.setClientACM(1, 2, 0); // Only close on invocation
        }

        async runTestCase(adapter, proxy)
        {
            await Ice.Promise.delay(3000);
            test(this._heartbeat === 0);
            test(!this._closed);
        }
    }

    class CloseOnIdleAndInvocationTest extends TestCase
    {
        constructor(com, out)
        {
            super("close on idle and invocation", com, out);
            this.setClientACM(1, 3, 0); // Only close on idle and invocation
        }

        async runTestCase(adapter, proxy)
        {
            //
            // Put the adapter on hold. The server will not respond to
            // the graceful close. This allows to test whether or not
            // the close is graceful or forceful.
            //
            await adapter.hold();
            await Ice.Promise.delay(3000);
            test(this._heartbeat === 0);
            test(!this._closed); // Not closed yet because of graceful close.
            await adapter.activate();
            await Ice.Promise.delay(1000);
            await this.waitForClosed(); // Connection should be closed this time.
        }
    }

    class ForcefullCloseOnIdleAndInvocationTest extends TestCase
    {
        constructor(com, out)
        {
            super("forcefull close on idle and invocation", com, out);
            this.setClientACM(1, 4, 0); // Only close on idle and invocation
        }

        async runTestCase(adapter, proxy)
        {
            await adapter.hold();
            await Ice.Promise.delay(3000);
            await this.waitForClosed();

            test(this._heartbeat === 0);
        }
    }

    class HeartbeatOnIdleTest extends TestCase
    {
        constructor(com, out)
        {
            super("heartbeat on idle", com, out);
            this.setServerACM(1, -1, 2); // Enable server heartbeats.
        }

        async runTestCase(adapter, proxy)
        {
            await Ice.Promise.delay(3000);
            return this._heartbeat >= 3;
        }
    }

    class HeartbeatAlwaysTest extends TestCase
    {
        constructor(com, out)
        {
            super("heartbeat always", com, out);
            this.setServerACM(1, -1, 3); // Enable server heartbeats.
        }

        async runTestCase(adapter, proxy)
        {
            for(let i = 0; i < 10; i++)
            {
                await proxy.ice_ping();
                await Ice.Promise.delay(300);
            }

            test(this._heartbeat >= 3);
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

        async runTestCase(adapter, proxy)
        {
            await proxy.startHeartbeatCount();
            const con = await proxy.ice_getConnection();
            await con.heartbeat();
            await con.heartbeat();
            await con.heartbeat();
            await con.heartbeat();
            await con.heartbeat();
            await proxy.waitForHeartbeatCount(5);
        }
    }

    class SetACMTest extends TestCase
    {
        constructor(com, out)
        {
            super("setACM/getACM", com, out);
            this.setClientACM(15, 4, 0);
        }

        async runTestCase(adapter, proxy)
        {
            const con = proxy.ice_getCachedConnection();

            try
            {
                con.setACM(-19, undefined, undefined);
                test(false);
            }
            catch(ex)
            {
            }

            let acm = con.getACM();
            test(acm.timeout === 15);
            test(acm.close === Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat === Ice.ACMHeartbeat.HeartbeatOff);

            con.setACM(undefined, undefined, undefined);
            acm = con.getACM();
            test(acm.timeout === 15);
            test(acm.close === Ice.ACMClose.CloseOnIdleForceful);
            test(acm.heartbeat === Ice.ACMHeartbeat.HeartbeatOff);

            con.setACM(1, Ice.ACMClose.CloseOnInvocationAndIdle, Ice.ACMHeartbeat.HeartbeatAlways);
            acm = con.getACM();
            test(acm.timeout === 1);
            test(acm.close === Ice.ACMClose.CloseOnInvocationAndIdle);
            test(acm.heartbeat === Ice.ACMHeartbeat.HeartbeatAlways);

            await proxy.startHeartbeatCount();
            await proxy.waitForHeartbeatCount(2);

            await new Promise(
                (resolve, reject) =>
                    {
                        con.setCloseCallback(() => resolve());
                        con.close(Ice.ConnectionClose.Gracefully);
                    });

            await new Promise(
                (resolve, reject) =>
                    {
                        con.setCloseCallback(() => resolve());
                    });
            con.setHeartbeatCallback(c => test(false));
        }
    }

    async function allTests(out, communicator)
    {
        const ref = "communicator:default -p 12010";
        const com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

        const tests = [];
        //
        // Skip some tests with IE it opens too many connections and
        // IE doesn't allow more than 6 connections.
        //
        if(typeof navigator !== "undefined" &&
           ["MSIE", "Trident/7.0", "Edge/12", "Edge/13"].some(value => navigator.userAgent.indexOf(value) !== -1))
        {
            tests.push(new HeartbeatOnIdleTest(com, out));
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

        await Promise.all(tests.map(test => test.init()));
        await Promise.all(tests.map(test => test.start()));
        for(const test of tests)
        {
            test.join(out);
        }
        await Promise.all(tests.map(test => test.destroy()));

        out.write("shutting down... ");
        await com.shutdown();
        out.writeLine("ok");
    }

    async function run(out, initData)
    {
        let communicator;
        try
        {
            initData.properties.setProperty("Ice.Warn.Connections", "0");
            communicator = Ice.initialize(initData);
            await allTests(out, communicator);
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    exports._test = run;
    exports._runServer = true;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require : this.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports : this));
