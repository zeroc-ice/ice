// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(){

var Promise = Ice.Promise;
var CallbackSenderPrx = Demo.CallbackSenderPrx;

//
// Define a servant class that implements the Demo.CallbackReceiver
// interface.
//
var CallbackReceiverI = Ice.Class(Demo.CallbackReceiver, {
    callback: function(num, current)
    {
        writeLine("received callback #" + num);
    }
});

var id = new Ice.InitializationData();
id.properties = Ice.createProperties();
//
// Client-side ACM must be disabled for bidirectional connections.
//
id.properties.setProperty("Ice.ACM.Client", "0");

//
// Initialize the communicator
//
var communicator = Ice.initialize(id);

var connection;

var start = function()
{
    //
    // Create a proxy to the sender object.
    //
    var hostname = document.location.hostname || "127.0.0.1";
    var proxy = communicator.stringToProxy("sender:ws -p 10002 -h " + hostname);
    
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
                    // Set the connection adapter and remember the connection.
                    //
                    connection = proxy.ice_getCachedConnection();
                    connection.setAdapter(adapter);
                    
                    //
                    // Register the client with the bidir server.
                    //
                    return server.addClient(r.ice_getIdentity());
                });
        });
};

var stop = function()
{
    //
    // Close the connection, the server will unregister the client
    // when it tries to invoke on the bi-dir proxy.
    // 
    return connection.close(false);
}

//
// Setup button click handlers
//
$("#start").click(
    function()
    {
        if(isDisconnected())
        {
            setState(State.Connecting);
            Promise.try(
                function()
                {
                    return start().then(function()
                                        {
                                            setState(State.Connected);
                                        });
                }
            ).exception(
                function(ex)
                {
                    $("#output").val(ex.toString());
                    setState(State.Disconnected);
                }
            );
        }
        return false;
    });

$("#stop").click(
    function()
    {
        if(isConnected())
        {
            setState(State.Disconnecting);
            Promise.try(
                function()
                {
                    return stop();
                }
            ).exception(
                function(ex)
                {
                    $("#output").val(ex.toString());
                }
            ).finally(
                function()
                {
                    setState(State.Disconnected);
                }
            );
        }
        return false;
    });

//
// Handle client state
//
var State = {
    Disconnected: 0,
    Connecting: 1,
    Connected: 2,
    Disconnecting: 3
};

var isConnected = function()
{
    return state == State.Connected;
};

var isDisconnected = function()
{
    return state == State.Disconnected;
};

var writeLine = function(msg)
{
    $("#output").val($("#output").val() + msg + "\n");
    $("#output").scrollTop($("#output").get(0).scrollHeight);
}

var state;

var setState = function(s)
{
    if(state == s)
    {
        return;
    }
    state = s;
    switch(s)
    {
        case State.Disconnected:
        {
            $("#start").removeClass("disabled");

            $("#progress").hide();
            $("body").removeClass("waiting");
            break;
        }
        case State.Connecting:
        {
            $("#output").val("");
            $("#start").addClass("disabled");

            $("#progress .message").text("Connecting...");
            $("#progress").show();
            $("body").addClass("waiting");
            break;
        }
        case State.Connected:
        {
            $("#stop").removeClass("disabled");

            $("#progress").hide();
            $("body").removeClass("waiting");
            break;
        }
        case State.Disconnecting:
        {
            $("#stop").addClass("disabled");

            $("#progress .message").text("Disconnecting...");
            $("#progress").show();
            $("body").addClass("waiting");
            break;
        }
        default:
        {
            break;
        }
    }
};

setState(State.Disconnected);

}());
