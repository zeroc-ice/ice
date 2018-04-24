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
    Test : false,
    WorkerGlobalScope : false,
    _server: false,
    _serveramd: false,
    URI: false
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
    constructor(promise, output, ready)
    {
        super();
        this._promise = promise;
        this._output = output;
        this._ready = ready;
    }

    async waitReady(timeout, current)
    {
        if(this._ready)
        {
            await this._ready;
        }
    }

    async waitSuccess(timeout, current)
    {
        try
        {
            await this._promise;
            return 0;
        }
        catch(ex)
        {
            this._output.writeLine(`unexpected exception while running test: ${ex.toString()}`);
            return 1;
        }
    }

    terminate(current)
    {
        current.adapter.remove(current.id);
        return this._output.get();
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
        let es5 = document.location.pathname.indexOf("/es5/") !== -1;
        let promise;
        let ready = null;
        let out;
        if(exe === "Server" || exe === "ServerAMD")
        {
            ready = new Ice.Promise();
            out = this._serverOutput;
        }
        else
        {
            out = this._clientOutput;
        }
        out.clear();

        if(this._useWorker)
        {
            let scripts = this._scripts;
            promise = new Promise(
                (resolve, reject) =>
                    {
                        let worker = new Worker(es5 ?
                                                "/test/es5/Common/ControllerWorker.js" :
                                                "/test/Common/ControllerWorker.js");
                        this._worker = worker;
                        worker.onmessage = function(e)
                        {
                            if(e.data.type == "write")
                            {
                                out.write(e.data.message);
                            }
                            else if(e.data.type == "writeLine")
                            {
                                out.writeLine(e.data.message);
                            }
                            else if(e.data.type == "ready" && (exe === "Server" || exe === "ServerAMD"))
                            {
                                ready.resolve();
                            }
                            else if(e.data.type == "finished")
                            {
                                if(e.data.exception)
                                {
                                    reject(e.data.exception);
                                    if(ready)
                                    {
                                        ready.reject(e.data.exception);
                                    }
                                }
                                else
                                {
                                    resolve();
                                    if(ready)
                                    {
                                        ready.resolve();
                                    }
                                }
                                worker.terminate();
                            }
                        };
                        worker.postMessage({ scripts:scripts, exe:exe, args:args });
                    });
        }
        else
        {
            let initData = new Ice.InitializationData();
            initData.properties = Ice.createProperties(args);
            if(exe === "Server" || exe === "ServerAMD")
            {
                initData.logger = new Logger(this._serverOutput);
                let test = exe === "Server" ? _server : _serveramd;
                promise = test(this._serverOutput, initData, ready, args);
            }
            else
            {
                initData.logger = new Logger(this._clientOutput);
                promise = _test(this._clientOutput, initData, args);
            }
        }
        return Test.Common.ProcessPrx.uncheckedCast(current.adapter.addWithUUID(new ProcessI(promise, out, ready)));
    }

    getHost(protocol, ipv6, current)
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

    let out = new Output(clientOutput);
    let serverOut = new Output(serverOutput);

    let uri = new URI(document.location.href);
    let protocol = uri.protocol() === "http" ? "ws" : "wss";
    let query = uri.search(true);
    let port = query.port || 15002;
    let worker = query.worker === "True";

    let initData = new Ice.InitializationData();
    initData.logger = new Logger(out);
    initData.properties = Ice.createProperties();
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");

    async function registerProcessController(adapter, registry, processController)
    {
        try
        {
            await registry.ice_ping();
            let connection = registry.ice_getCachedConnection();
            connection.setAdapter(adapter);
            connection.setACM(5, Ice.ACMClose.CloseOff, Ice.ACMHeartbeat.HeartbeatAlways);
            connection.setCloseCallback(
                connection => out.writeLine("connection with process controller registry closed"));
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
        let str = `Util/ProcessControllerRegistry:${protocol} -h ${document.location.hostname} -p ${port}`;
        let registry = Test.Common.ProcessControllerRegistryPrx.uncheckedCast(comm.stringToProxy(str));
        let adapter = await comm.createObjectAdapter("");
        let ident = new Ice.Identity("ProcessController", "Browser");
        let processController = adapter.add(new ProcessControllerI(out, serverOut, worker, scripts), ident);
        adapter.activate();
        registerProcessController(adapter, registry, processController);
    }
    catch(ex)
    {
        out.writeLine("unexpected exception while creating controller:\n" + ex.toString());
        comm.destroy();
    }
}
