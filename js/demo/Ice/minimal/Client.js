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
require("./Hello");
    
var communicator;
 
Ice.Promise.try(
    function()
    {
        //
        // Initialize the communicator and create a proxy to the hello object.
        //
        communicator = Ice.initialize();
        var proxy = communicator.stringToProxy("hello:tcp -h localhost -p 10000");
        
        //
        // Down-cast the proxy to the hello object interface.
        //
        return Demo.HelloPrx.checkedCast(proxy).then(
            function(hello)
            {
                //
                // Invoke the sayHello method.
                //
                return hello.sayHello();
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
