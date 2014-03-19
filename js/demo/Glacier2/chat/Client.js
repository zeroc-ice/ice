// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(){

require("Ice");
require("Glacier2");
require("./Chat");

//
// Servant that implements the ChatCallback interface,
// the message operation just writes the received data
// to stdout.
//
var ChatCallbackI = Ice.Class(Demo.ChatCallback, {
    message: function(data)
    {
        console.log(data);
    }
});

var communicator;

//
// Destroy communicator on SIGINT so application
// exit cleanly.
//
process.once("SIGINT", function() {
    if(communicator)
    {
        communicator.destroy().finally(
            function()
            {
                process.exit(0);
            });
    }
});

Ice.Promise.try(
    function() 
    {
        //
        // Initialize the communicator with Ice.Default.Router property
        // set to the chat demo Glacier2 router.
        //
        var id = new Ice.InitializationData();
        id.properties = Ice.createProperties();
        id.properties.setProperty("Ice.Default.Router", "DemoGlacier2/router:tcp -p 4063 -h localhost");
        communicator = Ice.initialize(id);
        
        function createSession()
        {
            return Ice.Promise.try(
                function()
                {
                    //
                    // Get a proxy to the default rotuer and down-cast it to Glacier2.Router
                    // interface to ensure Glacier2 server is available.
                    //
                    var router = communicator.getDefaultRouter();
                    var id;

                    return Glacier2.RouterPrx.checkedCast(router).then(
                        function(proxy)
                        {
                            router = proxy;
                            console.log("This demo accepts any user-id / password combination.");
                            process.stdout.write("user id: ");
                            return getline();
                        }
                    ).then(
                        function(str)
                        {
                            id = str;
                            process.stdout.write("password: ");
                            return getline();
                        }
                    ).then(
                        function(password)
                        {
                            return router.createSession(id, password);
                        }
                    ).then(
                        function(session)
                        {
                            return runWithSession(router, Demo.ChatSessionPrx.uncheckedCast(session));
                        },
                        function(ex)
                        {
                            if(ex instanceof Glacier2.PermissionDeniedException)
                            {
                                console.log("permission denied:\n" + ex.reason);
                                return createSession();
                            }
                            else if(ex instanceof Glacier2.CannotCreateSessionException)
                            {
                                console.log("cannot create session:\n" + ex.reason);
                                return createSession();
                            }
                            else
                            {
                                throw ex;
                            }
                        });
                });
        };
        
        function runWithSession(router, session)
        {
            var p = new Ice.Promise();

            //
            // Get the session timeout, the router client category and
            // create the client object adapter.
            //
            // Use Ice.Promise.all to wait for the completion of all the
            // calls.
            //
            Ice.Promise.all(
                router.getSessionTimeout(),
                router.getCategoryForClient(),
                communicator.createObjectAdapterWithRouter("", router)
            ).then(
                function(timeoutA, categoryA, adapterA)
                {
                    var timeout = timeoutA[0];
                    var category = categoryA[0];
                    var adapter = adapterA[0];
                    
                    //
                    // Call refreshSession in a loop to keep the 
                    // session alive.
                    //
                    var refreshSession = function()
                    {
                        router.refreshSession().exception(
                            function(ex)
                            {
                                p.fail(ex);
                            }
                        ).delay(timeout.toNumber() * 500).then(
                            function()
                            {
                                if(!p.completed())
                                {
                                    refreshSession();
                                }
                            });
                    };
                    refreshSession();
                    
                    //
                    // Create the ChatCallback servant and add it to the ObjectAdapter.
                    //
                    var callback = Demo.ChatCallbackPrx.uncheckedCast(
                        adapter.add(new ChatCallbackI(), new Ice.Identity("callback", category)));
                    
                    //
                    // Set the chat session callback.
                    //
                    return session.setCallback(callback);
                }
            ).then(
                function()
                {
                    //
                    // The chat function sequantially reads stdin messages
                    // and send it to server using the session say method.
                    //
                    function chat()
                    {
                        process.stdout.write("==> ");
                        return getline().then(
                            function(msg)
                            {
                                if(msg == "/quit")
                                {
                                    p.succeed();
                                }
                                else if(msg.indexOf("/") == 0)
                                {
                                    console.log("enter /quit to exit.");
                                }
                                else
                                {
                                    return session.say(msg);
                                }
                            }
                        ).then(
                            function()
                            {
                                if(!p.completed())
                                {
                                    return chat();
                                }
                            }
                        ).exception(
                            function(ex)
                            {
                                p.fail(ex);
                            });
                    }
                    
                    //
                    // Start the chat loop
                    //
                    return chat();
                }
            ).finally(
                function()
                {
                    //
                    // Destroy the session.
                    //
                    return router.destroySession();
                }
            ).exception(
                function(ex)
                {
                    p.fail(ex);
                });
            return p;
        }
        return createSession();
    }
).finally(
    function()
    {
        //
        // Destroy the communicator if required.
        //
        if(communicator)
        {
            return communicator.destroy();
        }
    }
).then(
    function()
    {
        process.exit(0);
    },
    function(ex)
    {
        //
        // Handle any exceptions above.
        //
        console.log(ex.toString());
        process.exit(1);
    });

//
// Asynchonously process stdin lines using a promise
//
var getline = function()
{
    var p = new Ice.Promise();
    process.stdin.resume();
    process.stdin.once("data", 
        function(buffer)
        {
            process.stdin.pause();
            p.succeed(buffer.toString("utf-8").trim());
        });
    return p;
};

}());
