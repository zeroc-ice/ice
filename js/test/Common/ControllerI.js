// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

class ProcessI extends Test.Common.Process
{
    constructor(promise, output)
    {
        super();
        this._promise = promise;
        this._output = output;
    }

    waitReady(timeout, current)
    {
    }

    waitSuccess(timeout, current)
    {
        return this._promise.then(function() { return 0; }, function(ex) { console.log(ex); return 1; });
    }

    terminate(current)
    {
        current.adapter.remove(current.id);
        return this._output.get();
    }
};

class ProcessControllerI extends Test.Common.ProcessController
{
    constructor(output, logger)
    {
        super();
        this._output = output;
        this._logger = logger;
    }

    start(testSuite, exe, args, current)
    {
        let promise;
        let initData = new Ice.InitializationData();
        initData.logger = this._logger;
        initData.properties = Ice.createProperties(args);
        if(exe === "ClientBidir")
        {
            promise = _testBidir(this._output, initData);
        }
        else
        {
            promise = _test(this._output, initData);
        }
        promise = promise.catch(ex => {
            this._output.writeLine("unexpected exception while running test: " + ex.toString() + "\nstack = " + ex.stack);
            throw ex;
        });
        return Test.Common.ProcessPrx.uncheckedCast(current.adapter.addWithUUID(new ProcessI(promise, this._output)));
    }

    getHost(protocol, ipv6, current)
    {
        return "127.0.0.1"
    }
};

function runController(output)
{
    let out =
    {
        write: function(msg)
        {
            let text = output.val();
            output.val((text === "") ? msg : (text + msg));
        },
        writeLine: function(msg)
        {
            out.write(msg + "\n");
            output.scrollTop(output.get(0).scrollHeight);
        },
        get: function()
        {
            return output.val()
        }
    };

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

    //
    // This logger is setup to work with Web Workers and normal scripts using
    // the received out object. With some browser like Safari using console.log
    // method doesn't work when running inside a web worker.
    //
    let logger =
    {
        print: function(message)
        {
            out.writeLine(message, false);
        },
        trace: function(category, message)
        {
            let s = [];
            let d = new Date();
            s.push("-- ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push(category);
            s.push(": ");
            s.push(message);
            out.writeLine(s.join(""), true);
        },
        warning: function(message)
        {
            let s = [];
            let d = new Date();
            s.push("-! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("warning: ");
            s.push(message);
            out.writeLine(s.join(""), true);
        },
        error: function(message)
        {
            let s = [];
            let d = new Date();
            s.push("!! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("error: ");
            s.push(message);
            out.writeLine(s.join(""), true);
        },
        getPrefix: function()
        {
            return "";
        },
        cloneWithPrefix: function(prefix)
        {
            return Logger;
        },
        timestamp: function()
        {
            let d = new Date();
            return d.toLocaleString("en-US", this._dateformat) + "." + d.getMilliseconds();
        }
    };

    let uri = new URI(document.location.href)
    let initData = new Ice.InitializationData();
    let protocol = uri.protocol() === "http" ? "ws" : "wss";
    query = uri.search(true)
    let port = query["port"]
    let worker = query["worker"]
    initData.logger = logger;

    let registerProcessController = function(adapter, registry, processController) {
        registry.setProcessController(Test.Common.ProcessControllerPrx.uncheckedCast(processController)).then(
        () => {
            let connection = registry.ice_getCachedConnection();
            connection.setAdapter(adapter)
            connection.setACM(5, Ice.ACMClose.CloseOff, Ice.ACMHeartbeat.HeartbeatAlways);
            connection.setCloseCallback(connection => {
                logger.print("connection with process controller registry closed");
            });
        },
        ex => {
            if(ex instanceof Ice.ConnectFailedException)
            {
                setTimeout(() => registerProcessController(adapter, registry, processController), 2000);
            }
            else
            {
                logger.error("unexpected exception while connecting to process controller registry:\n" + ex.toString())
            }
        });
    };

    let comm = Ice.initialize();
    let str = "Util/ProcessControllerRegistry:" + protocol + " -h 127.0.0.1 -p " + port;
    let registry = Test.Common.ProcessControllerRegistryPrx.uncheckedCast(comm.stringToProxy(str));
    comm.createObjectAdapter("").then(adapter => {
        let ident = new Ice.Identity("ProcessController", "Browser");
        let processController = adapter.add(new ProcessControllerI(out, logger), ident);
        adapter.activate();
        registerProcessController(adapter, registry, processController);
    }).catch(ex => {
        logger.error("unexpected exception while creating controller:\n" + ex.toString());
        comm.destroy();
    });
}