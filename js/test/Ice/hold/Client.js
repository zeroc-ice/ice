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
    const TestHelper = require("TestHelper").TestHelper;
    const test = TestHelper.test;

    class Client extends TestHelper
    {
        async allTests()
        {
            class Condition
            {
                constructor(value)
                {
                    this.value = value;
                }
            }

            class SetCB
            {
                constructor(condition, expected)
                {
                    this.condition = condition;
                    this.expected = expected;
                }

                response(value)
                {
                    if(value != this.expected)
                    {
                        this.condition.value = false;
                    }
                }
            }

            const communicator = this.communicator();
            const out = this.getWriter();

            const ref = "hold:" + this.getTestEndpoint();
            const hold = await Test.HoldPrx.checkedCast(communicator.stringToProxy(ref));
            test(hold !== null);
            const holdOneway = Test.HoldPrx.uncheckedCast(hold.ice_oneway());

            const refSerialized = "hold:" + this.getTestEndpoint(1);
            const holdSerialized = await Test.HoldPrx.checkedCast(communicator.stringToProxy(refSerialized));
            test(holdSerialized !== null);
            const holdSerializedOneway = Test.HoldPrx.uncheckedCast(holdSerialized.ice_oneway());

            out.write("changing state between active and hold rapidly... ");

            for(let i = 0; i < 100; ++i)
            {
                await hold.putOnHold(0);
            }
            for(let i = 0; i < 100; ++i)
            {
                await holdOneway.putOnHold(0);
            }
            for(let i = 0; i < 100; ++i)
            {
                await holdSerialized.putOnHold(0);
            }
            for(let i = 0; i < 1; ++i)
            {
                await holdSerializedOneway.putOnHold(0);
            }
            out.writeLine("ok");

            out.write("testing without serialize mode... ");
            {
                const condition = new Condition(true);
                let value = 0;
                let result;
                const results = [];
                while(condition.value)
                {
                    const cb = new SetCB(condition, value);
                    result = hold.set(++value, value < 500 ? Math.floor((Math.random() * 5)) : 0);
                    results.push(
                        result.then(
                            v =>
                                {
                                    cb.response(v);
                                },
                            () =>
                                {
                                    // Ignore exception
                                }));

                    if(value % 100 === 0)
                    {
                        while(true)
                        {
                            if(result.isSent())
                            {
                                break;
                            }
                            else if(result.isCompleted())
                            {
                                await result; // This should throw the failure if the call wasn't sent but done.
                                test(result.isSent());
                            }
                            await Ice.Promise.delay(10);
                        }
                    }

                    if(value > 10000)
                    {
                        // Don't continue, it's possible that out-of-order dispatch doesn't occur
                        // after 100000 iterations and we don't want the test to last for too long
                        // when this occurs.
                        break;
                    }
                }
                test(value > 10000 || !condition.value);
                while(true)
                {
                    if(result.isSent())
                    {
                        break;
                    }
                    else if(result.isCompleted())
                    {
                        await result; // This should throw the failure if the call wasn't sent but done.
                        test(result.isSent());
                    }
                    await Ice.Promise.delay(10);
                }
                await Promise.all(results);
            }
            out.writeLine("ok");

            out.write("testing with serialize mode... ");
            {
                const condition = new Condition(true);
                let value = 0;
                let result;
                const results = [];
                while(value < 3000 && condition.value)
                {
                    const cb = new SetCB(condition, value);
                    result = holdSerialized.set(++value, 0);
                    results.push(
                        result.then(
                            v =>
                                {
                                    cb.response(v);
                                },
                            () =>
                                {
                                    // Ignore exceptions
                                }));

                    if(value % 100 === 0)
                    {
                        while(true)
                        {
                            if(result.isSent())
                            {
                                break;
                            }
                            else if(result.isCompleted())
                            {
                                await result; // This should throw the failure if the call wasn't sent but done.
                                test(result.isSent());
                            }
                            await Ice.Promise.delay(10);
                        }
                    }
                }
                await result;
                test(condition.value);

                for(let i = 0; i < 10000; ++i)
                {
                    await holdSerializedOneway.setOneway(value + 1, value);
                    ++value;
                    if((i % 100) == 0)
                    {
                        await holdSerializedOneway.putOnHold(1);
                    }
                }
                await Promise.all(results);
            }
            out.writeLine("ok");

            out.write("testing serialization... ");
            {
                let value = 0;
                let result;
                const results = [];
                await holdSerialized.set(value, 0);

                for(let i = 0; i < 10000; ++i)
                {
                    // Create a new proxy for each request
                    result = holdSerialized.ice_oneway().setOneway(value + 1, value);
                    results.push(result);
                    ++value;
                    if((i % 100) === 0)
                    {
                        while(true)
                        {
                            if(result.isSent())
                            {
                                break;
                            }
                            else if(result.isCompleted())
                            {
                                await result; // This should throw the failure if the call wasn't sent but done.
                                test(result.isSent());
                            }
                            await Ice.Promise.delay(10);
                        }
                        await holdSerialized.ice_ping(); // Ensure everything's dispatched.
                        const conn = await holdSerialized.ice_getConnection();
                        await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                    }
                }
                await Promise.all(results);
            }
            out.writeLine("ok");

            out.write("testing waitForHold... ");
            await hold.waitForHold();
            await hold.waitForHold();
            for(let i = 0; i < 1000; ++i)
            {
                await holdOneway.ice_ping();
                if((i % 20) == 0)
                {
                    await hold.putOnHold(0);
                }
            }
            await hold.putOnHold(-1);
            await hold.ice_ping();
            await hold.putOnHold(-1);
            await hold.ice_ping();
            out.writeLine("ok");

            out.write("changing state to hold and shutting down server... ");
            await hold.shutdown();
            out.writeLine("ok");
        }

        async run(args)
        {
            let communicator;
            try
            {
                const [properties] = this.createTestProperties(args);
                //
                // For this test, we want to disable retries.
                //
                properties.setProperty("Ice.RetryIntervals", "-1");

                //
                // We don't want connection warnings because of the timeout
                //
                properties.setProperty("Ice.Warn.Connections", "0");

                [communicator] = this.initialize(properties);
                await this.allTests();
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
    }
    exports.Client = Client;
}(typeof global !== "undefined" && typeof global.process !== "undefined" ? module : undefined,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? require :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self.Ice._require : window.Ice._require,
  typeof global !== "undefined" && typeof global.process !== "undefined" ? exports :
  (typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) ? self : window));
