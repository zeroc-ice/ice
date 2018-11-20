// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

/* globals Ice, Test, URI, WorkerGlobalScope */
/* eslint no-unused-vars: ["error", { "varsIgnorePattern": "isSafari|isChrome|isWorker|isWindows|runController" }] */

function isSafari()
{
    return (/^((?!chrome).)*safari/i).test(navigator.userAgent);
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
    return typeof WorkerGlobalScope !== 'undefined' && this instanceof WorkerGlobalScope;
}

function isWindows()
{
    return navigator.userAgent.indexOf("Windows") != -1;
}

class Logger extends Ice.Logger
{
    constructor(out)
    {
        super();
        this._out = out;
    }

    write(message, indent)
    {
        if(indent)
        {
            message = message.replace(/\n/g, "\n   ");
        }
        this._out.writeLine(message);
    }
}

class ProcessI extends Test.Common.Process
{
    constructor(promise, helper, output)
    {
        super();
        this._promise = promise;
        this._helper = helper;
        this._output = output;
    }

    async waitReady()
    {
        await this._helper.waitReady();
    }

    async waitSuccess()
    {
        try
        {
            await this._promise;
            this._helper.serverReady();
            return 0;
        }
        catch(ex)
        {
            this._output.writeLine(`unexpected exception while running test: ${ex.toString()}`);
            this._output.writeLine(ex.stack);
            this._helper.serverReady(ex);
            return 1;
        }
    }

    terminate(current)
    {
        current.adapter.remove(current.id);
        return this._output.get();
    }
}

class ControllerHelper
{
    constructor(exe, output)
    {
        if(exe === "Server" || exe === "ServerAMD")
        {
            this._serverReady = new Ice.Promise();
        }
        this._output = output;
    }

    serverReady(ex)
    {
        if(this._serverReady)
        {
            if(ex)
            {
                this._serverReady.reject(ex);
            }
            else
            {
                this._serverReady.resolve();
            }
        }
    }

    async waitReady()
    {
        if(this._serverReady)
        {
            await this._serverReady;
        }
    }

    write(msg)
    {
        this._output.write(msg);
    }

    writeLine(msg)
    {
        this._output.writeLine(msg);
    }
}

class ProcessControllerI extends Test.Common.BrowserProcessController
{
    constructor(clientOutput, serverOutput, useWorker, scripts)
    {
        super();
        this._clientOutput = clientOutput;
        this._serverOutput = serverOutput;
        this._useWorker = useWorker;
        this._scripts = scripts;
    }

    start(testSuite, exe, args, current)
    {
        const es5 = document.location.pathname.indexOf("/es5/") !== -1;
        let promise;
        let out;
        if(exe === "Server" || exe === "ServerAMD")
        {
            out = this._serverOutput;
        }
        else
        {
            out = this._clientOutput;
        }
        out.clear();
        const helper = new ControllerHelper(exe, out);

        if(this._useWorker)
        {
            const scripts = this._scripts;
            promise = new Promise(
                (resolve, reject) =>
                    {
                        const worker = new Worker(es5 ?
                                                "/test/es5/Common/ControllerWorker.js" :
                                                "/test/Common/ControllerWorker.js");
                        this._worker = worker;
                        worker.onmessage = function(e)
                        {
                            if(e.data.type == "write")
                            {
                                helper.write(e.data.message);
                            }
                            else if(e.data.type == "writeLine")
                            {
                                helper.writeLine(e.data.message);
                            }
                            else if(e.data.type == "ready")
                            {
                                helper.serverReady();
                            }
                            else if(e.data.type == "finished")
                            {
                                if(e.data.exception)
                                {
                                    reject(e.data.exception);
                                }
                                else
                                {
                                    resolve();
                                }
                                worker.terminate();
                            }
                        };
                        worker.postMessage({scripts: scripts, exe: exe, args: args});
                    });
        }
        else
        {
            const cls = Ice._require(exe)[exe];
            const test = new cls();
            test.setControllerHelper(helper);
            promise = test.run(args);
        }
        return Test.Common.ProcessPrx.uncheckedCast(current.adapter.addWithUUID(new ProcessI(promise, helper, out)));
    }

    getHost()
    {
        return "127.0.0.1";
    }

    redirect(url, current)
    {
        current.con.close(Ice.ConnectionClose.Gracefully);
        window.location.href = url;
    }
}

async function runController(clientOutput, serverOutput, scripts)
{
    class Output
    {
        constructor(output)
        {
            this.output = output;
        }

        write(msg)
        {
            const text = this.output.val();
            this.output.val(text + msg);
        }

        writeLine(msg)
        {
            this.write(msg + "\n");
            this.output.scrollTop(this.output.get(0).scrollHeight);
        }

        get()
        {
            return this.output.val();
        }

        clear()
        {
            this.output.val("");
        }
    }

    const out = new Output(clientOutput);
    const serverOut = new Output(serverOutput);

    const uri = new URI(document.location.href);
    const protocol = uri.protocol() === "http" ? "ws" : "wss";
    const query = uri.search(true);
    const port = query.port || 15002;
    const worker = query.worker === "True";

    const initData = new Ice.InitializationData();
    initData.logger = new Logger(out);
    initData.properties = Ice.createProperties();
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");

    async function registerProcessController(adapter, registry, processController)
    {
        try
        {
            await registry.ice_ping();
            const connection = registry.ice_getCachedConnection();
            connection.setAdapter(adapter);
            connection.setACM(5, Ice.ACMClose.CloseOff, Ice.ACMHeartbeat.HeartbeatAlways);
            connection.setCloseCallback(
                () => out.writeLine("connection with process controller registry closed"));
            await registry.setProcessController(Test.Common.ProcessControllerPrx.uncheckedCast(processController));
        }
        catch(ex)
        {
            if(ex instanceof Ice.ConnectFailedException)
            {
                setTimeout(() => registerProcessController(adapter, registry, processController), 2000);
            }
            else
            {
                out.writeLine("unexpected exception while connecting to process controller registry:\n" + ex.toString());
            }
        }
    }

    let comm;
    try
    {
        comm = Ice.initialize(initData);
        const str = `Util/ProcessControllerRegistry:${protocol} -h ${document.location.hostname} -p ${port}`;
        const registry = Test.Common.ProcessControllerRegistryPrx.uncheckedCast(comm.stringToProxy(str));
        const adapter = await comm.createObjectAdapter("");
        const ident = new Ice.Identity("ProcessController", "Browser");
        const processController = adapter.add(new ProcessControllerI(out, serverOut, worker, scripts), ident);
        adapter.activate();
        registerProcessController(adapter, registry, processController);
    }
    catch(ex)
    {
        out.writeLine("unexpected exception while creating controller:\n" + ex.toString());
        comm.destroy();
    }
}
