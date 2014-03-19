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

function menu()
{
    process.stdout.write(
        "usage:\n" +
            "t: send greeting as twoway\n" +
            "o: send greeting as oneway\n" +
            "O: send greeting as batch oneway\n" +
            "f: flush all batch requests\n" +
            "T: set a timeout\n" +
            "P: set a server delay\n" +
            //"S: switch secure mode on/off\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n" +
            "\n");
}

var communicator;
Ice.Promise.try(
    function() 
    {
        communicator = Ice.initialize();
        var proxy = communicator.stringToProxy("hello:default -p 10000").ice_twoway().ice_timeout(-1).ice_secure(false);
        var secure = false;
        var timeout = -1;
        var delay = 0;
        
        return Demo.HelloPrx.checkedCast(proxy).then(
            function(twoway)
            {
                var oneway = twoway.ice_oneway();
                var batchOneway = twoway.ice_batchOneway();

                menu();
                process.stdout.write("==> ");
                var loop = new Ice.Promise();
                function processKey(key)
                {
                    if(key == "x")
                    {
                        loop.succeed();
                        return;
                    }
                    
                    if(key == "t")
                    {
                        return twoway.sayHello(delay);
                    }
                    else if(key == "o")
                    {
                        return oneway.sayHello(delay);
                    }
                    else if(key == "O")
                    {
                        return batchOneway.sayHello(delay);
                    }
                    else if(key == "f")
                    {
                        return communicator.flushBatchRequests();
                    }
                    else if(key == "T")
                    {
                        if(timeout == -1)
                        {
                            timeout = 2000;
                        }
                        else
                        {
                            timeout = -1;
                        }

                        twoway = twoway.ice_timeout(timeout);
                        oneway = oneway.ice_timeout(timeout);
                        batchOneway = batchOneway.ice_timeout(timeout);

                        if(timeout == -1)
                        {
                            console.log("timeout is now switched off");
                        }
                        else
                        {
                            console.log("timeout is now set to 2000ms");
                        }
                    }
                    else if(key == "P")
                    {
                        if(delay === 0)
                        {
                            delay = 2500;
                        }
                        else
                        {
                            delay = 0;
                        }

                        if(delay === 0)
                        {
                            console.log("server delay is now deactivated");
                        }
                        else
                        {
                            console.log("server delay is now set to 2500ms");
                        }
                    }
                    else if(key == "s")
                    {
                        return twoway.shutdown();
                    }
                    else if(key == "?")
                    {
                        process.stdout.write("\n");
                        menu();
                    }
                    else
                    {
                        console.log("unknown command `" + key + "'");
                        process.stdout.write("\n");
                        menu();
                    }
                }

                //
                // Process keys sequentially. We chain the promise objects
                // returned by processKey(). Once we have process all the
                // keys we print the prompt and resume the standard input.
                //
                process.stdin.resume();
                var promise = new Ice.Promise().succeed();
                process.stdin.on("data", 
                                 function(buffer)
                                 {
                                     process.stdin.pause();
                                     var data = buffer.toString("utf-8").trim().split("");
                                     // Process each key
                                     data.forEach(function(key)
                                                  {
                                                      promise = promise.then(
                                                          function() 
                                                          { 
                                                              return processKey(key); 
                                                          }
                                                      ).exception(
                                                          function(ex)
                                                          {
                                                              console.log(ex.toString());
                                                          });
                                                  });
                                     // Once we're done, print the prompt
                                     promise.then(function() 
                                                  {
                                                      if(!loop.completed())
                                                      {
                                                          process.stdout.write("==> ");
                                                          process.stdin.resume();
                                                      }
                                                  });
                                     data = [];
                                 });

                return loop;
            });
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
        process.exit(0);
    },
    function(ex)
    {
        console.log(ex.toString());
        process.exit(1);
    });
}());
