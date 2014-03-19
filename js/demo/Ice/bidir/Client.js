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
require("./Callback");

var Demo = global.Demo;
var CallbackSenderPrx = Demo.CallbackSenderPrx;

//
// Define a servant class that implements Demo.CallbackReceiver
// interface.
//
var CallbackReceiverI = Ice.Class(Demo.CallbackReceiver, {
    callback: function(num, current)
    {
        console.log("received callback #" + num);
    }
});

var id = new Ice.InitializationData();
id.properties = Ice.createProperties();
//
// Client-side ACM must be disabled for bidirectional connections.
//
id.properties.setProperty("Ice.ACM.Client", "0");

var communicator = Ice.initialize(id);

//
// Exit on SIGINT
//
process.on("SIGINT", function() {
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
        // Initialize the communicator and create a proxy to the sender object.
        //
        var proxy = communicator.stringToProxy("sender:tcp -p 10000");
        
        //
        // Down-cast the proxy to the Demo.CallbackSender interface.
        //
        return CallbackSenderPrx.checkedCast(proxy).then(
            function(server)
            {
                //
                // Create the client object adapter.
                //
                return communicator.createObjectAdapter("").then(
                    function(adapter)
                    {
                        //
                        // Create a callback receiver servant and add it to
                        // the object adapter.
                        //
                        var r = adapter.addWithUUID(new CallbackReceiverI());
                        
                        //
                        // Set the connection adapter.
                        //
                        proxy.ice_getCachedConnection().setAdapter(adapter);

                        //
                        // Register the client with the bidir server.
                        //
                        return server.addClient(r.ice_getIdentity());
                    });
            });
    }
).exception(
    function(ex)
    {
        console.log(ex.toString());
        Promise.try(
            function()
            {
                if(communicator)
                {
                    return communicator.destroy();
                }
            }
        ).finally(
            function()
            {
                process.exit(1);
            });
    });

}());
