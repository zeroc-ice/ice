// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

 /* global
    _test : false,
    _runServer : false,
    _server: false,
    Test : false,
    WorkerGlobalScope: false
*/

function isSafari()
{
    return /^((?!chrome).)*safari/i.test(navigator.userAgent);
}

function isChrome()
{
    //
    // We need to check for Edge browser as it might include Chrome in its user agent.
    //
    return navigator.userAgent.indexOf("Edge/") === -1 &&
           navigator.userAgent.indexOf("Chrome/") !== -1;
}

function isWorker()
{
    return typeof(WorkerGlobalScope) !== 'undefined' && this instanceof WorkerGlobalScope;
}

function isWindows()
{
    return navigator.userAgent.indexOf("Windows") != -1;
}

async function runTest(testsuite, language, host, protocol, testcases, out)
{
    //
    // This logger is setup to work with Web Workers and normal scripts using
    // the received out object. With some browser like Safari using console.log
    // method doesn't work when running inside a web worker.
    //
    class Logger
    {
        print(message)
        {
            out.writeLine(message, false);
        }

        trace(category, message)
        {
            const s = [];
            const d = new Date();
            s.push("-- ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push(category);
            s.push(": ");
            s.push(message);
            out.writeLine(s.join(""), true);
        }

        warning(message)
        {
            const s = [];
            const d = new Date();
            s.push("-! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("warning: ");
            s.push(message);
            out.writeLine(s.join(""), true);
        }

        error(message)
        {
            const s = [];
            const d = new Date();
            s.push("!! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("error: ");
            s.push(message);
            out.writeLine(s.join(""), true);
        }

        getPrefix()
        {
            return "";
        }

        cloneWithPrefix(prefix)
        {
            return Logger;
        }

        timestamp()
        {
            const d = new Date();
            return d.toLocaleString("en-US", this._dateformat) + "." + d.getMilliseconds();
        }
    }

    let communicator;
    try
    {
        let initData = new Ice.InitializationData();
        let port = protocol == "ws" ? 15002 : 15003;
        initData.logger = new Logger();
        initData.properties = Ice.createProperties();
        initData.properties.setProperty("Ice.Default.Host", host);
        initData.properties.setProperty("Ice.Default.Protocol", protocol);
        //initData.properties.setProperty("Ice.Trace.Protocol", "1");
        //initData.properties.setProperty("Ice.Trace.Network", "3");

        if(typeof(_runServer) === "undefined")
        {
            await _test(out, initData);
        }
        else
        {
            communicator = Ice.initialize();
            let controller = Test.Common.ControllerPrx.uncheckedCast(
                communicator.stringToProxy(`controller:${protocol} -h ${host} -p ${port}`));

            testcases = testcases || [ { name: "client/server" } ];

            async function run(testcase, client)
            {
                let serverTestCase;
                try
                {
                    if(testcase.langs && testcase.langs.indexOf(language) == -1)
                    {
                        return;
                    }

                    out.writeLine(`[ running ${testcase.name} test]`);
                    out.write("starting server side... ");
                    serverTestCase = (language === "js") ?
                        await controller.runTestCase("cpp", "Ice/echo", "server", "") :
                        await controller.runTestCase("js", testsuite, testcase.name, language);

                    serverTestCase = Test.Common.TestCasePrx.uncheckedCast(
                        controller.ice_getCachedConnection().createProxy(serverTestCase.ice_getIdentity()));

                    let config = new Test.Common.Config();
                    config.protocol = protocol;
                    await serverTestCase.startServerSide(config);
                    out.writeLine("ok");

                    let server;
                    if(language === "js")
                    {
                        let id = initData.clone();
                        if(testcase.args !== undefined)
                        {
                            id.properties = Ice.createProperties(testcase.args, initData.properties);
                        }
                        let ready = new Ice.Promise();
                        server = _server(out, id, ready, testcase.args);
                        await ready;
                    }

                    {
                        let id = initData.clone();
                        if(testcase.args !== undefined)
                        {
                            initData.properties = Ice.createProperties(testcase.args, id.properties);
                        }
                        await client(out, id, testcase.args);
                    }

                    if(server)
                    {
                        await server; // Wait for server to terminate
                    }

                    serverTestCase.stopServerSide(true);
                }
                catch(ex)
                {
                    out.writeLine("failed! (" + ex + ")");
                    throw ex;
                }
                finally
                {
                    if(serverTestCase)
                    {
                        await serverTestCase.destroy();
                    }
                }
            }

            for(let testcase of testcases)
            {
                await run(testcase, _test);
            }
        }
    }
    catch(ex)
    {
        out.writeLine("");
        if(ex instanceof Test.Common.TestCaseFailedException)
        {
            out.writeLine("Server test case failed to start:\n");
            out.writeLine(ex.output);
        }
        else
        {
            out.writeLine(ex.toString());
            if(ex.stack)
            {
                out.writeLine(ex.stack);
            }
        }
        return false
    }
    finally
    {
        if(communicator)
        {
            await communicator.destroy();
        }
    }
    return true;
}
