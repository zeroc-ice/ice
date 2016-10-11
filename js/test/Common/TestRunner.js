// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


 /* global
    __runEchoServerOptions__ : false,
    __test__ : false,
    Test : false,
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

function runTest(name, language, defaultHost, protocol, configurations, out)
{
    //
    // This logger is setup to work with Web Workers and normal scripts using
    // the received out object. With some browser like Safari using console.log
    // method doesn't work when running inside a web worker.
    //
    var Logger =
    {
        print(message)
        {
            out.writeLine(message, false);
        },
        trace:function(category, message)
        {
            var s = [];
            var d = new Date();
            s.push("-- ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push(category);
            s.push(": ");
            s.push(message);
            out.writeLine(s.join(""), true);
        },
        warning:function(message)
        {
            var s = [];
            var d = new Date();
            s.push("-! ");
            s.push(this.timestamp());
            s.push(' ');
            s.push(this._prefix);
            s.push("warning: ");
            s.push(message);
            out.writeLine(s.join(""), true);
        },
        error:function(message)
        {
            var s = [];
            var d = new Date();
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
        timestamp:function()
        {
            var d = new Date();
            return d.toLocaleString("en-US", this._dateformat) + "." + d.getMilliseconds();
        }
    };
    
    var server, communicator;
    var id = new Ice.InitializationData();
    id.logger = Logger;
    id.properties = Ice.createProperties();
    id.properties.setProperty("Ice.Default.Host", defaultHost);
    id.properties.setProperty("Ice.Default.Protocol", protocol);
    //id.properties.setProperty("Ice.Trace.Protocol", "1");
    //id.properties.setProperty("Ice.Trace.Network", "3");

    return Ice.Promise.try(
        function()
        {
            if(typeof(__runServer__) !== "undefined" || typeof(__runEchoServer__) !== "undefined")
            {
                communicator = Ice.initialize();
                var str = protocol == "ws" ? "controller:ws -h " + defaultHost + " -p 15002" :
                                             "controller:wss -h " + defaultHost + " -p 15003";

                var controller = Test.Common.ControllerPrx.uncheckedCast(communicator.stringToProxy(str));

                var options = [];
                var srv = typeof(__runEchoServer__) !== "undefined" ? "Ice/echo" : name;
                if(typeof(__runEchoServerOptions__) !== "undefined")
                {
                    options = options.concat(__runEchoServerOptions__);
                }

                if(configurations === undefined)
                {
                    configurations = [ { configName: "", desc: "default configuration" } ];
                }

                var prev = Ice.Promise.resolve();
                configurations.forEach(
                    function(config)
                    {
                        if(config.langs && config.langs.indexOf(language) == -1)
                        {
                            return prev;
                        }
                        prev = prev.then(
                            function()
                            {
                                out.write("starting " + srv + " server... ");
                                return controller.runServer(language, srv, protocol, defaultHost, false,
                                                            config.configName, options).then(
                                    function(proxy)
                                    {
                                        var ref = proxy.ice_getIdentity().name + ":" + protocol + " -h " +
                                            defaultHost + " -p " + (protocol == "ws" ? "15002" : "15003");
                                        out.writeLine("ok");
                                        server = Test.Common.ServerPrx.uncheckedCast(communicator.stringToProxy(ref));
                                        out.writeLine("Running test with " + config.desc + ".");
                                        return server.waitForServer().then(
                                            function()
                                            {
                                                var initData = id.clone();
                                                if(config.args !== undefined)
                                                {
                                                    initData.properties =
                                                        Ice.createProperties(config.args, id.properties);
                                                }
                                                return __test__(out, initData);
                                            });
                                    },
                                    function(ex)
                                    {
                                        out.writeLine("failed! (" + ex + ")");
                                        throw ex;
                                    }
                                ).then(
                                    function()
                                    {
                                        if(server)
                                        {
                                            return server.waitTestSuccess();
                                        }
                                    }
                                ).catch(
                                    function(ex)
                                    {
                                        if(server)
                                        {
                                            return server.terminate().then(
                                                function()
                                                {
                                                    throw ex;
                                                },
                                                function()
                                                {
                                                    throw ex;
                                                });
                                        }
                                        else
                                        {
                                            throw ex;
                                        }
                                    });
                            });
                    });
                return prev;
            }
            else
            {
                return __test__(out, id);
            }
        }
    ).finally(
        function()
        {
            if(communicator)
            {
                return communicator.destroy();
            }
        }
    ).then(
        function()
        {
            return true;
        },
        function(ex)
        {
            out.writeLine("");
            if(ex instanceof Test.Common.ServerFailedException)
            {
                out.writeLine("Server failed to start:\n");
                out.writeLine(ex.reason);
            }
            else
            {
                out.writeLine(ex.toString());
                if(ex.stack)
                {
                    out.writeLine(ex.stack);
                }
            }
            return false;
        });
}
