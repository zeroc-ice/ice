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
require("./Throughput");

function menu()
{
    process.stdout.write(
        "usage:\n" +
        "\n" +
        "toggle type of data to send:\n" +
        "1: sequence of bytes (default)\n" +
        "2: sequence of strings (\"hello\")\n" +
        "3: sequence of structs with a string (\"hello\") and a double\n" +
        "4: sequence of structs with two ints and a double\n" +
        "\n" +
        "select test to run:\n" +
        "t: Send sequence as twoway\n" +
        "o: Send sequence as oneway\n" +
        "r: Receive sequence\n" +
        "e: Echo (send and receive) sequence\n" +
        "\n" +
        "other commands:\n" +
        "s: shutdown server\n" +
        "x: exit\n" +
        "?: help\n" +
        "\n");
};

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

//
// Initialize sequences.
//
var byteSeq = new Buffer(Demo.ByteSeqSize);
for(var i = 0; i < Demo.ByteSeqSize; ++i)
{
    byteSeq[i] = 0;
}

var stringSeq = [];
for(var i = 0; i < Demo.StringSeqSize; ++i)
{
    stringSeq[i] = "hello";
}

var structSeq = [];
for(var i = 0; i < Demo.StringDoubleSeqSize; ++i)
{
    structSeq[i] = new Demo.StringDouble();
    structSeq[i].s = "hello";
    structSeq[i].d = 3.14;
}

var fixedSeq = [];
for(var i = 0; i < Demo.FixedSeqSize; ++i)
{
    fixedSeq[i] = new Demo.Fixed();
    fixedSeq[i].i = 0;
    fixedSeq[i].j = 0;
    fixedSeq[i].d = 0;
}

var communicator;
Ice.Promise.try(
    function()
    {
        var currentType = "1";
        var repetitions = 100;
        
        var seqSize = Demo.ByteSeqSize;
        var seq = byteSeq;
        var wireSize = 1;
        
        //
        // Initialize the communicator and create a proxy
        // to the throughput object.
        //
        communicator = Ice.initialize();
        var proxy = communicator.stringToProxy("throughput:default -p 10000");
        
        //
        // Down-cast the proxy to the Demo.Throughput interface.
        //
        return Demo.ThroughputPrx.checkedCast(proxy).then(
            function(twoway)
            {
                var oneway = twoway.ice_oneway();
                menu();
                process.stdout.write("==> ");
                var keyLoop = new Ice.Promise();
                
                function processKey(key)
                {
                    if(key == "x")
                    {
                        keyLoop.succeed();
                        return;
                    }
                    
                    var proxy;
                    var operation;
                    
                    if(key == "1" || key == "2" || key == "3" || key == "4")
                    {
                        currentType = key;

                        //
                        // Select the sequence data type to use by this test.
                        //
                        switch(currentType)
                        {
                            case "1":
                            {
                                console.log("using byte sequences");
                                seqSize = Demo.ByteSeqSize;
                                seq = byteSeq;
                                wireSize = 1;
                                break;
                            }

                            case "2":
                            {
                                console.log("using string sequences");
                                seqSize = Demo.StringSeqSize;
                                seq = stringSeq;
                                wireSize = seq[0].length;
                                break;
                            }

                            case "3":
                            {
                                console.log("using variable-length struct sequences");
                                seqSize = Demo.StringDoubleSeqSize;
                                seq = structSeq;
                                wireSize = seq[0].s.length;
                                wireSize += 8; // Size of double on the wire.
                                break;
                            }

                            case "4":
                            {
                                console.log("using fixed-length struct sequences");
                                seqSize = Demo.FixedSeqSize;
                                seq = fixedSeq;
                                wireSize = 16; // Size of two ints and a double on the wire.
                                break;
                            }
                        }
                    }
                    else if(key == "t" || key == "o" || key == "r" || key == "e")
                    {
                        //
                        // Select the proxy and operation to use by this test.
                        //
                        switch(key)
                        {
                            case "t":
                            case "o":
                            {
                                proxy = key == "o" ? oneway : twoway;
                                if(currentType == 1)
                                {
                                    operation = proxy.sendByteSeq;
                                }
                                else if(currentType == 2)
                                {
                                    operation = proxy.sendStringSeq;
                                }
                                else if(currentType == 3)
                                {
                                    operation = proxy.sendStructSeq;
                                }
                                else if(currentType == 4)
                                {
                                    operation = proxy.sendFixedSeq;
                                }
                                process.stdout.write("sending");
                                break;
                            }
                            
                            case "r":
                            {
                                proxy = twoway;
                                if(currentType == 1)
                                {
                                    operation = proxy.recvByteSeq;
                                }
                                else if(currentType == 2)
                                {
                                    operation = proxy.recvStringSeq;
                                }
                                else if(currentType == 3)
                                {
                                    operation = proxy.recvStructSeq;
                                }
                                else if(currentType == 4)
                                {
                                    operation = proxy.recvFixedSeq;
                                }
                                process.stdout.write("receiving");
                                break;
                            }
                            
                            case "e":
                            {
                                proxy = twoway;
                                if(currentType == 1)
                                {
                                    operation = proxy.echoByteSeq;
                                }
                                else if(currentType == 2)
                                {
                                    operation = proxy.echoStringSeq;
                                }
                                else if(currentType == 3)
                                {
                                    operation = proxy.echoStructSeq;
                                }
                                else if(currentType == 4)
                                {
                                    operation = proxy.echoFixedSeq;
                                }
                                process.stdout.write("sending and receiving");
                                break;
                            }
                        }
                        
                        process.stdout.write(" " + repetitions);
                        switch(currentType)
                        {
                            case "1":
                            {
                                process.stdout.write(" byte");
                                break;
                            }
                            case "2":
                            {
                                process.stdout.write(" string");
                                break;
                            }
                            case "3":
                            {
                                process.stdout.write(" variable-length struct");
                                break;
                            }

                            case "4":
                            {
                                process.stdout.write(" fixed-length struct");
                                break;
                            }
                        }
                        
                        process.stdout.write(" sequences of size " + seqSize);

                        if(key == "o")
                        {
                            process.stdout.write(" as oneway");
                        }
                        console.log("...");
                        
                        var start = new Date().getTime();
                        var args = key != "r" ? [seq] : [];
                        return loop(
                            function() 
                            {
                                return operation.apply(proxy, args);
                            },
                            repetitions
                        ).then(
                            function()
                            {
                                //
                                // Write the results.
                                //
                                var total = new Date().getTime() - start;
                                console.log("time for " + repetitions + " sequences: " + total  + " ms");
                                console.log("time per sequence: " + total / repetitions + " ms");
                                
                                var mbit = repetitions * seqSize * wireSize * 8.0 / total / 1000.0;
                                if(key == "e")
                                {
                                    mbit *= 2;
                                }
                                mbit = Math.round(mbit * 100) / 100;
                                console.log("throughput: " + mbit + " Mbps");
                            });
                    }
                    else if(key == "s")
                    {
                        return twoway.shutdown();
                        process.stdout.write("==> ");
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
                                                          function(r) 
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
                                                      if(!keyLoop.completed())
                                                      {
                                                          process.stdout.write("==> ");
                                                          process.stdin.resume();
                                                      }
                                                  });
                                     data = [];
                                 });

                return keyLoop;
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
}());
