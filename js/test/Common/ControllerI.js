// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

var process = { argv : [] };

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

    waitReady(timeout, current)
    {
        if(this._ready)
        {
            return this._ready;
        }
    }

    waitSuccess(timeout, current)
    {
        let out = this._output;
        return this._promise.then(function() {
            return 0;
        }, function(ex) {
            out.writeLine("unexpected exception while running test: " + ex.toString() + "\nstack = " + ex.stack);
            return 1;
        });
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
            promise = new Promise((resolve, reject) =>
                {
                    let worker;
                    if(document.location.pathname.indexOf("/es5/") !== -1)
                    {
                        worker = new Worker("/test/es5/Common/ControllerWorker.js");
                    }
                    else
                    {
                        worker = new Worker("/test/Common/ControllerWorker.js");
                    }
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
                            }
                            else
                            {
                                resolve();
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
            process.argv = args;
            if(exe === "Server" || exe === "ServerAMD")
            {
                initData.logger = new Logger(this._serverOutput);
                let test = exe === "Server" ? _server : _serveramd;
                promise = test(this._serverOutput, initData, ready);
            }
            else
            {
                initData.logger = new Logger(this._clientOutput);
                promise = _test(this._clientOutput, initData);
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

function runController(clientOutput, serverOutput, scripts)
{
    function wrapOutput(output)
    {
        return {
            write: function(msg)
            {
                let text = output.val();
                output.val((text === "") ? msg : (text + msg));
            },
            writeLine: function(msg)
            {
                msg = msg + "\n";
                let text = output.val();
                output.val((text === "") ? msg : (text + msg));
                output.scrollTop(output.get(0).scrollHeight);
            },
            get: function()
            {
                return output.val();
            },
            clear : function()
            {
                output.val("");
            }
        };
    }
    let out = wrapOutput(clientOutput);
    let serverOut = wrapOutput(serverOutput);

    window.onerror = function(msg, url, line, column, err)
    {
        let e = msg + " at " + url + ":" + line + ":" + column;
        if(err)
        {
            e += "\n" + err.stack;
        }
        out.writeLine(e);
        return false;
    };

    let uri = new URI(document.location.href);
    let protocol = uri.protocol() === "http" ? "ws" : "wss";
    let query = uri.search(true);
    let port = query.port || 15002;
    let worker = query.worker === "True";

    let initData = new Ice.InitializationData();
    initData.logger = new Logger(out);
    initData.properties = Ice.createProperties();
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "1000");

    let registerProcessController = function(adapter, registry, processController) {
        registry.ice_ping().then(
            () =>
            {
                let connection = registry.ice_getCachedConnection();
                connection.setAdapter(adapter);
                connection.setACM(5, Ice.ACMClose.CloseOff, Ice.ACMHeartbeat.HeartbeatAlways);
                connection.setCloseCallback(
                    connection => out.writeLine("connection with process controller registry closed"));
                return registry.setProcessController(Test.Common.ProcessControllerPrx.uncheckedCast(processController));
            }).catch(
            ex =>
            {
                if(ex instanceof Ice.ConnectFailedException)
                {
                    setTimeout(() => registerProcessController(adapter, registry, processController), 2000);
                }
                else
                {
                    out.writeLine("unexpected exception while connecting to process controller registry:\n" + ex.toString());
                }
            });
    };

    let comm = Ice.initialize(initData);
    let str = "Util/ProcessControllerRegistry:" + protocol + " -h " + document.location.hostname + " -p " + port;
    let registry = Test.Common.ProcessControllerRegistryPrx.uncheckedCast(comm.stringToProxy(str));
    comm.createObjectAdapter("").then(
        adapter =>
        {
            let ident = new Ice.Identity("ProcessController", "Browser");
            let processController = adapter.add(new ProcessControllerI(out, serverOut, worker, scripts), ident);
            adapter.activate();
            registerProcessController(adapter, registry, processController);
        }).catch(
        ex =>
        {
            out.writeLine("unexpected exception while creating controller:\n" + ex.toString());
            comm.destroy();
        });
}
