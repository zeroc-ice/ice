// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    var server, communicator;
    var id = new Ice.InitializationData();
    id.properties = Ice.createProperties();
    id.properties.setProperty("Ice.Default.Host", defaultHost);
    id.properties.setProperty("Ice.Default.Protocol", protocol);

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
                out.write("starting " + srv + " server... ");
                return controller.runServer(language, srv, protocol, defaultHost, false, options).then(
                    function(proxy)
                    {
                        var ref = proxy.ice_getIdentity().name + ":" + protocol + " -h " + defaultHost + " -p " +
                            (protocol == "ws" ? "15002" : "15003");
                        out.writeLine("ok");
                        server = Test.Common.ServerPrx.uncheckedCast(communicator.stringToProxy(ref));

                        if(configurations === undefined)
                        {
                            return server.waitForServer().then(
                                function()
                                {
                                    return __test__(out, id);
                                });
                        }
                        else
                        {
                            var prev = new Ice.Promise().succeed();
                            configurations.forEach(
                                function(configuration)
                                {
                                    if(configuration.langs && configuration.langs.indexOf(language) == -1)
                                    {
                                        return prev;
                                    }
                                    prev = prev.then(
                                        function()
                                        {
                                            out.writeLine("Running test with " + configuration.name + ".");
                                            return server.waitForServer().then(
                                                function()
                                                {
                                                    var initData = id.clone();
                                                    if(configuration.args !== undefined)
                                                    {
                                                        initData.properties =
                                                            Ice.createProperties(configuration.args, id.properties);
                                                    }
                                                    return __test__(out, initData);
                                                });
                                        });
                                });
                            return prev;
                        }
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
                ).exception(
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
        function(ex, r)
        {
            out.writeLine("");
            if(r instanceof Ice.AsyncResult)
            {
                out.writeLine("exception occurred in call to " + r.operation);
            }
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
