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
require("./Latency");

var communicator;

//
// Asynchronous loop, each call to the given function returns a
// promise that when fulfilled runs the next iteration.
//    
function loop(fn, repetitions)
{
    var i = 0;
    var next = function() 
    {
        if(i++ < repetitions)
        {
            return fn.call().then(next);
        }
    };
    return next();
}

Ice.Promise.try(
    function()
    {
        //
        // Initialize the communicator and create a proxy to the 
        // ping object.
        //
        communicator = Ice.initialize();
        var repetitions = 10000;
        var proxy = communicator.stringToProxy("ping:default -p 10000");
        
        //
        // Down-cast the proxy to the Demo.Ping interface.
        //
        return Demo.PingPrx.checkedCast(proxy).then(
            function(obj)
            {
                console.log("pinging server " + repetitions + " times (this may take a while)");
                start = new Date().getTime();
                return loop(
                    function() 
                    {
                        return obj.ice_ping();
                    },
                    repetitions
                ).then(
                    function()
                    {
                        //
                        // Write the results.
                        //
                        total = new Date().getTime() - start;
                        console.log("time for " + repetitions + " pings: " + total + "ms");
                        console.log("time per ping: " + (total / repetitions) + "ms");
                    });
            });
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
).exception(
    function(ex)
    {
        //
        // Handle any exceptions above.
        //
        console.log(ex.toString());
        process.exit(1);
    });
}());
