// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* global
    isSafari : false,
    isWorker : false
*/

(function(module, require, exports)
{
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;
    const ArrayUtil = Ice.ArrayUtil;

    const isBrowser = (typeof window !== 'undefined' || typeof WorkerGlobalScope !== 'undefined');
    const isConnectionFailed = ex =>
          (!isBrowser && ex instanceof Ice.ConnectionRefusedException) ||
          (isBrowser && ex instanceof Ice.ConnectFailedException) ||
          (ex instanceof Ice.ConnectTimeoutException);

    async function allTests(out, initData)
    {
        function test(value, ex)
        {
            if(!value)
            {
                let message = "test failed\n";
                if(ex)
                {
                    message += ex.toString();
                }
                throw new Error(message);
            }
        }

        async function createTestIntfPrx(adapters)
        {
            let endpoints = [];
            let obj = null;
            for(let adapter of adapters)
            {
                obj = await adapter.getTestIntf();
                endpoints = endpoints.concat(obj.ice_getEndpoints());
            }
            return Test.TestIntfPrx.uncheckedCast(obj.ice_endpoints(endpoints));
        }

        async function deactivate(communicator, adapters)
        {
            for(let adapter of adapters)
            {
                await communicator.deactivateObjectAdapter(adapter);
            }
        }

        let communicator;
        try
        {
            communicator = Ice.initialize(initData);

            out.write("testing stringToProxy... ");
            ref = "communicator:default -p 12010";
            com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));
            test(com !== null);
            out.writeLine("ok");

            out.write("testing binding with single endpoint... ");
            {
                let adapter = await com.createObjectAdapter("Adapter", "default");

                let test1 = await adapter.getTestIntf();
                let test2 = await adapter.getTestIntf();
                test(await test1.ice_getConnection() === await test2.ice_getConnection());

                await test1.ice_ping();
                await test2.ice_ping();

                await com.deactivateObjectAdapter(adapter);

                let test3 = Test.TestIntfPrx.uncheckedCast(test1);
                test(await test3.ice_getConnection() === await test1.ice_getConnection());
                test(await test3.ice_getConnection() === await test2.ice_getConnection());

                try
                {
                    await test3.ice_ping();
                    test(false);
                }
                catch(ex)
                {
                    test(isConnectionFailed(ex), ex);
                }

            }
            out.writeLine("ok")

            await communicator.destroy();
            communicator = Ice.initialize(initData);
            com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            out.write("testing binding with multiple endpoints... ");
            {
                let adapters = await Promise.all([com.createObjectAdapter("Adapter11", "default"),
                                                  com.createObjectAdapter("Adapter12", "default"),
                                                  com.createObjectAdapter("Adapter13", "default")]);

                //
                // Ensure that when a connection is opened it's reused for new
                // proxies and that all endpoints are eventually tried.
                //
                let names = ["Adapter11", "Adapter12", "Adapter13"];
                while(names.length > 0)
                {
                    let adpts = ArrayUtil.clone(adapters);
                    let test1 = await createTestIntfPrx(adpts);
                    ArrayUtil.shuffle(adpts)
                    let test2 = await createTestIntfPrx(adpts);
                    ArrayUtil.shuffle(adpts)
                    let test3 = await createTestIntfPrx(adpts);
                    await test1.ice_ping();
                    test(await test1.ice_getConnection() == await test2.ice_getConnection());
                    test(await test2.ice_getConnection() == await test3.ice_getConnection());

                    let name = await test1.getAdapterName();
                    let index = names.indexOf(name);
                    if(index !== -1)
                    {
                        names.splice(index, 1);
                    }

                    let conn = await test1.ice_getConnection();
                    await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                }
                //
                // Ensure that the proxy correctly caches the connection (we
                // always send the request over the same connection.)
                //
                {
                    for(let adpt of adapters)
                    {
                        let prx = await adpt.getTestIntf();
                        await prx.ice_ping();
                    }

                    let t = await createTestIntfPrx(adapters);
                    let name = await t.getAdapterName();
                    let nRetry = 10;
                    let i;
                    for(i = 0; i < nRetry && await t.getAdapterName() == name; i++);
                    test(i == nRetry);

                    for(let adpt of adapters)
                    {
                        let prx = await adpt.getTestIntf();
                        let conn = await prx.ice_getConnection();
                        await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                    }
                }
                //
                // Deactivate an adapter and ensure that we can still
                // establish the connection to the remaining adapters.
                //
                await com.deactivateObjectAdapter(adapters[0]);
                names = ["Adapter12", "Adapter13"];
                while(names.length > 0)
                {
                    let adpts = ArrayUtil.clone(adapters);

                    let test1 = await createTestIntfPrx(adpts);
                    ArrayUtil.shuffle(adpts);
                    let test2 = await createTestIntfPrx(adpts);
                    ArrayUtil.shuffle(adpts);
                    let test3 = await createTestIntfPrx(adpts);

                    test(await test1.ice_getConnection() == await test2.ice_getConnection());
                    test(await test2.ice_getConnection() == await test3.ice_getConnection());

                    let name = await test1.getAdapterName();
                    let index = names.indexOf(name);
                    if(index !== -1)
                    {
                        names.splice(index, 1);
                    }
                    let conn = await test1.ice_getConnection();
                    await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                }
                //
                // Deactivate an adapter and ensure that we can still
                // establish the connection to the remaining adapter.
                //
                await com.deactivateObjectAdapter(adapters[2]);
                let obj = await createTestIntfPrx(adapters);
                test(await obj.getAdapterName() == "Adapter12");

                await deactivate(com, adapters);
            }
            out.writeLine("ok");

            await communicator.destroy();
            communicator = Ice.initialize(initData);
            com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            //
            // Skip this test with IE it open too many connections IE doesn't allow more
            // than 6 connections. Firefox has a configuration that causes failed connections
            // to be delayed on retry (network.websocket.delay-failed-reconnects=true by
            // default), so we prefer to also disable this test with Firefox.
            //
            if(typeof(navigator) === "undefined" ||
               ["MSIE", "Trident/7.0", "Firefox", "Edge"].every(value => navigator.userAgent.indexOf(value) === -1))
            {
                out.write("testing binding with multiple random endpoints... ");

                let adapters = await Promise.all([
                    com.createObjectAdapter("AdapterRandom11", "default"),
                    com.createObjectAdapter("AdapterRandom12", "default"),
                    com.createObjectAdapter("AdapterRandom13", "default"),
                    com.createObjectAdapter("AdapterRandom14", "default"),
                    com.createObjectAdapter("AdapterRandom15", "default")]);

                let count = 20;
                let adapterCount = adapters.length;
                while(--count > 0)
                {
                    let proxies = [];
                    if(count == 1)
                    {
                        await com.deactivateObjectAdapter(adapters[4]);
                        --adapterCount;
                    }
                    proxies = new Array(!0);

                    let i;
                    for(i = 0; i < proxies.length; ++i)
                    {
                        let adpts = new Array(Math.floor(Math.random() * adapters.length));
                        if(adpts.length == 0)
                        {
                            adpts = new Array(1);
                        }
                        for(let j = 0; j < adpts.length; ++j)
                        {
                            adpts[j] = adapters[Math.floor(Math.random() * adapters.length)];
                        }
                        proxies[i] = await createTestIntfPrx(ArrayUtil.clone(adpts));
                    }

                    for(i = 0; i < proxies.length; i++)
                    {
                        await proxies[i].getAdapterName();
                    }

                    for(i = 0; i < proxies.Length; i++)
                    {
                        try
                        {
                            await proxies[i].ice_ping();
                        }
                        catch(ex)
                        {
                            test(ex instanceof Ice.LocalException, ex);
                        }
                    }

                    let connections = [];
                    for(i = 0; i < proxies.length; i++)
                    {
                        if(proxies[i].ice_getCachedConnection() !== null)
                        {
                            if(connections.indexOf(proxies[i].ice_getCachedConnection()) === -1)
                            {
                                connections.push(proxies[i].ice_getCachedConnection());
                            }
                        }
                    }
                    test(connections.length <= adapterCount);

                    for(let a of adapters)
                    {
                        try
                        {
                            let prx = await a.getTestIntf();
                            let conn = await prx.ice_getConnection();
                            await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                        }
                        catch(ex)
                        {
                            // Expected if adapter is down.
                            test(ex instanceof Ice.LocalException, ex);
                        }
                    }
                }
                out.writeLine("ok");
            }

            await communicator.destroy();
            communicator = Ice.initialize(initData);
            com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            out.write("testing random endpoint selection... ");
            {
                let adapters = await Promise.all([
                    com.createObjectAdapter("Adapter21", "default"),
                    com.createObjectAdapter("Adapter22", "default"),
                    com.createObjectAdapter("Adapter23", "default")]);

                let obj = await createTestIntfPrx(adapters);
                test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

                let names = ["Adapter21", "Adapter22", "Adapter23"];
                while(names.length > 0)
                {
                    let index = names.indexOf(await obj.getAdapterName());
                    if(index !== -1)
                    {
                        names.splice(index, 1);
                    }
                    let conn = await obj.ice_getConnection();
                    await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                }

                obj = Test.TestIntfPrx.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Random));
                test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

                names = ["Adapter21", "Adapter22", "Adapter23"]
                while(names.length > 0)
                {
                    let index = names.indexOf(await obj.getAdapterName());
                    if(index !== -1)
                    {
                        names.splice(index, 1);
                    }
                    let conn = await obj.ice_getConnection();
                    await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                }

                await deactivate(com, adapters);
            }
            out.writeLine("ok");

            await communicator.destroy();
            communicator = Ice.initialize(initData);
            com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            out.write("testing ordered endpoint selection... ");
            {
                let adapters = await Promise.all([
                    com.createObjectAdapter("Adapter31", "default"),
                    com.createObjectAdapter("Adapter32", "default"),
                    com.createObjectAdapter("Adapter33", "default")]);

                let obj = await createTestIntfPrx(adapters);
                obj = Test.TestIntfPrx.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
                test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
                let nRetry = 3;
                let i;

                //
                // Ensure that endpoints are tried in order by deactiving the adapters
                // one after the other.
                //
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter31"; i++);
                test(i == nRetry);
                await com.deactivateObjectAdapter(adapters[0]);
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter32"; i++);
                test(i == nRetry);
                await com.deactivateObjectAdapter(adapters[1]);
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter33"; i++);
                test(i == nRetry);
                await com.deactivateObjectAdapter(adapters[2]);

                try
                {
                    await obj.getAdapterName();
                }
                catch(ex)
                {
                    test(isConnectionFailed(ex), ex);
                }

                let endpoints = obj.ice_getEndpoints();

                adapters = [];

                //
                // Now, re-activate the adapters with the same endpoints in the opposite
                // order.
                //
                adapters.push(await com.createObjectAdapter("Adapter36", endpoints[2].toString()));
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter36"; i++);
                test(i == nRetry);
                let conn = await obj.ice_getConnection();
                await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                adapters.push(await com.createObjectAdapter("Adapter35", endpoints[1].toString()));
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter35"; i++);
                test(i == nRetry);
                conn = await obj.ice_getConnection();
                await conn.close(Ice.ConnectionClose.GracefullyWithWait);
                adapters.push(await com.createObjectAdapter("Adapter34", endpoints[0].toString()));
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter34"; i++);
                test(i == nRetry);

                await deactivate(com, adapters);
            }
            out.writeLine("ok");

            await communicator.destroy();
            communicator = Ice.initialize(initData);
            com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            out.write("testing per request binding with single endpoint... ");
            {
                let adapter = await com.createObjectAdapter("Adapter41", "default");

                let test1 = Test.TestIntfPrx.uncheckedCast((await adapter.getTestIntf()).ice_connectionCached(false));
                let test2 = Test.TestIntfPrx.uncheckedCast((await adapter.getTestIntf()).ice_connectionCached(false));
                test(!test1.ice_isConnectionCached());
                test(!test2.ice_isConnectionCached());
                test(await test1.ice_getConnection() !== null && await test2.ice_getConnection() !== null);
                test(await test1.ice_getConnection() == await test2.ice_getConnection());

                await test1.ice_ping();

                await com.deactivateObjectAdapter(adapter);

                let test3 = Test.TestIntfPrx.uncheckedCast(test1);
                try
                {
                    test(await test3.ice_getConnection() == await test1.ice_getConnection());
                    test(false);
                }
                catch(ex)
                {
                    test(isConnectionFailed(ex), ex);
                }
            }
            out.writeLine("ok");

            await communicator.destroy();
            communicator = Ice.initialize(initData);
            com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            out.write("testing per request binding with multiple endpoints... ");
            {
                let adapters = await Promise.all([
                    com.createObjectAdapter("Adapter51", "default"),
                    com.createObjectAdapter("Adapter52", "default"),
                    com.createObjectAdapter("Adapter53", "default")]);

                let obj = Test.TestIntfPrx.uncheckedCast((await createTestIntfPrx(adapters)).ice_connectionCached(false));
                test(!obj.ice_isConnectionCached());

                let names = ["Adapter51", "Adapter52", "Adapter53"];
                while(names.length > 0)
                {
                    const name = await obj.getAdapterName();
                    const index = names.indexOf(name);
                    if(index !== -1)
                    {
                        names.splice(index, 1);
                    }
                }

                await com.deactivateObjectAdapter(adapters[0]);

                names = ["Adapter52", "Adapter53"];
                while(names.length > 0)
                {
                    const name = await obj.getAdapterName();
                    const index = names.indexOf(name);
                    if(index !== -1)
                    {
                        names.splice(index, 1);
                    }
                }

                await com.deactivateObjectAdapter(adapters[2]);

                test(await obj.getAdapterName() == "Adapter52");

                await deactivate(com, adapters);
            }
            out.writeLine("ok");

            if(typeof(navigator) === "undefined" ||
               ["Firefox", "MSIE", "Trident/7.0"].every(value => navigator.userAgent.indexOf(value) === -1))
            {
                //
                // Firefox adds a delay on websocket failed reconnects that causes this test to take too
                // much time to complete.
                //
                // You can set network.websocket.delay-failed-reconnects to false in Firefox about:config
                // to disable this behaviour
                //

                await communicator.destroy();
                communicator = Ice.initialize(initData);
                com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

                out.write("testing per request binding and ordered endpoint selection... ");
                let adapters = await Promise.all([
                    com.createObjectAdapter("Adapter61", "default"),
                    com.createObjectAdapter("Adapter62", "default"),
                    com.createObjectAdapter("Adapter63", "default")]);

                let obj = await createTestIntfPrx(adapters);
                obj = Test.TestIntfPrx.uncheckedCast(obj.ice_endpointSelection(Ice.EndpointSelectionType.Ordered));
                test(obj.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
                obj = Test.TestIntfPrx.uncheckedCast(obj.ice_connectionCached(false));
                test(!obj.ice_isConnectionCached());
                let nRetry = 3;
                let i;

                //
                // Ensure that endpoints are tried in order by deactiving the adapters
                // one after the other.
                //
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter61"; i++);
                test(i == nRetry);
                await com.deactivateObjectAdapter(adapters[0]);
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter62"; i++);
                test(i == nRetry);
                await com.deactivateObjectAdapter(adapters[1]);
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter63"; i++);
                test(i == nRetry);
                com.deactivateObjectAdapter(adapters[2]);

                try
                {
                    await obj.getAdapterName();
                }
                catch(ex)
                {
                    test(isConnectionFailed(ex), ex);
                }

                let endpoints = obj.ice_getEndpoints();

                adapters = [];

                //
                // Now, re-activate the adapters with the same endpoints in the opposite
                // order.
                //
                adapters.push(await com.createObjectAdapter("Adapter66", endpoints[2].toString()));
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter66"; i++);
                test(i == nRetry);
                adapters.push(await com.createObjectAdapter("Adapter65", endpoints[1].toString()));
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter65"; i++);
                test(i == nRetry);
                adapters.push(await com.createObjectAdapter("Adapter64", endpoints[0].toString()));
                for(i = 0; i < nRetry && await obj.getAdapterName() == "Adapter64"; i++);
                test(i == nRetry);

                await deactivate(com, adapters);
                out.writeLine("ok");
            }

            await com.shutdown();
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }

    async function run(out, initData)
    {
        if(typeof(navigator) !== 'undefined' && isSafari() && isWorker())
        {
            let communicator;
            try
            {
                communicator = Ice.initialize(initData);
                //
                // BUGFIX:
                //
                // With Safari 9.1 and WebWorkers, this test hangs in communicator destruction. The
                // web socket send() method never returns for the sending of close connection message.
                //
                out.writeLine("Test not supported with Safari web workers.");
                let obj = communicator.stringToProxy("communicator:default -p 12010");
                let prx = await Test.RemoteCommunicatorPrx.uncheckedCast(obj);
                await prx.shutdown();
            }
            finally
            {
                if(communicator)
                {
                    await communicator.destroy();
                }
            }
        }
        else
        {
            await allTests(out, initData);
        }
    }

    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
