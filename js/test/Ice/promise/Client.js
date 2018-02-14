// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Promise = Ice.Promise;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    //
    // Create a new promise object and call function fn with
    // the promise as its first argument, then return the new
    // promise.
    //
    var deferred = function(fn)
    {
        var promise = new Promise();
        fn.call(null, promise);
        return promise;
    };

    var run = function(out)
    {
        var p = new Promise();

        deferred(
            function(promise)
            {
                out.write("Creating a promise object that is resolved and succeed... ");
                var promise1 = new Promise().succeed(1024);
                promise1.then(
                    function(i)
                    {
                        test(i === 1024);
                        test(promise1.succeeded());
                        out.writeLine("ok");
                        promise.succeed();
                    },
                    function(e)
                    {
                        promise.fail();
                        test(false, e);
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Creating a promise object that is resolved and failed... ");
                        var promise1 = new Promise().fail("promise.fail");
                        promise1.then(
                            function(i)
                            {
                                promise.fail();
                                test(false, i);
                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                test(promise1.failed());
                                out.writeLine("ok");
                                promise.succeed();
                            });
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Creating a promise object that is resolved and succeed with multiple arguments... ");
                        var promise1 = new Promise().succeed(1024, "Hello World!");
                        promise1.then(
                            function(i, msg)
                            {
                                test(i === 1024);
                                test(msg === "Hello World!");
                                test(promise1.succeeded());
                                out.writeLine("ok");
                                promise.succeed();
                            },
                            function(e)
                            {
                                promise.fail(e);
                                test(false, e);
                            });
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Creating a promise with a callback that returns a new value... ");
                        var promise1 = new Promise().succeed(1024);
                        promise1.then(
                            function(i)
                            {
                                test(i === 1024);
                                test(promise1.succeeded());
                                return "Hello World!";
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            })
                        .then(
                            function(msg)
                            {
                                test(msg === "Hello World!");
                                out.writeLine("ok");
                                promise.succeed();
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            });
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Creating a promise object that recovers from a failure... ");
                        var promise1 = new Promise().fail("promise.fail");
                        promise1.then(
                            function(i)
                            {
                                promise.fail();
                                test(false, "Succeed called.failed expected");
                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                test(promise1.failed());
                                return "Hello World!";
                            })
                        .then(
                            function(msg)
                            {
                                test(msg === "Hello World!");
                                out.writeLine("ok");
                                promise.succeed();
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            });
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Creating a promise object that rethrow a.failure... ");
                        var promise1 = new Promise().fail("promise.fail");
                        promise1.then(
                            function(i)
                            {
                                promise.fail();
                                test(false, i);
                            },
                            function(e)
                            {
                                throw e;
                            })
                        .then(
                            function(msg)
                            {
                                promise.fail();
                                test(false, "Succeed called.failed expected");

                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                test(promise1.failed());
                                out.writeLine("ok");
                                promise.succeed();
                            });
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("A second call to then should produce the same results... ");
                        var promise1 = new Promise().succeed(1024);
                        promise1.then(
                            function(i)
                            {
                                test(i === 1024);
                                test(promise1.succeeded());
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            });
                        promise1.then(
                            function(i)
                            {
                                test(i === 1024);
                                test(promise1.succeeded());
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            });

                        promise1 = new Promise().fail("promise.fail");
                        promise1.then(
                            function(i)
                            {
                                promise.fail();
                                test(false, i);
                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                test(promise1.failed());
                            });
                        promise1.then(
                            function(i)
                            {
                                promise.fail();
                                test(false, i);
                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                test(promise1.failed());
                                out.writeLine("ok");
                                promise.succeed();
                            });
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Create a promise that is not yet resolved, but will succeed... ");
                        var promise1 = new Promise();
                        test(!promise1.completed());
                        promise1.then(
                            function(i)
                            {
                                test(i === 1024);
                                out.writeLine("ok");
                                promise.succeed();
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            }
                        );
                        promise1.succeed(1024);
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Create a promise that is not yet resolved, but will.fail... ");
                        var promise1 = new Promise();
                        test(!promise1.completed());
                        promise1.then(
                            function(i)
                            {
                                promise.fail();
                                test(false, "Succeed called.failed expected");
                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                test(promise1.failed());
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        );
                        promise1.fail("promise.fail");
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Create a promise chain that is not yet resolved, but will succeed... ");
                        var promise1 = new Promise();
                        var promise2 = new Promise();
                        var promise3 = new Promise();
                        promise1.then(
                            function(i)
                            {
                                test(i === 1);
                                return promise2;
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            }
                        ).then(
                            function(i)
                            {
                                test(i === 2);
                                return promise3;
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            }
                        ).then(
                            function(i)
                            {
                                test(i === 3);
                                return "Hello World!";
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            }
                        ).then(
                            function(msg)
                            {
                                test(promise1.succeeded() && promise2.succeeded() && promise3.succeeded());
                                test(msg === "Hello World!");
                                out.writeLine("ok");
                                promise.succeed();
                            },
                            function(e)
                            {
                                promise.fail();
                                test(false, e);
                            }
                        );
                        test(!promise1.completed() && !promise2.completed() && !promise3.completed());

                        promise1.succeed(1);
                        promise2.succeed(2);
                        promise3.succeed(3);
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Use exception method on a Promise that will.fail... ");
                        var promise1 = new Promise().fail("promise.fail");
                        promise1.exception(
                            function(e)
                            {
                                test(e === "promise.fail");
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        );
                    });
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Promise exception propagation in succeed callback... ");
                        var promise1 = new Promise().fail("promise.fail");
                        promise1.then(
                            function()
                            {
                                promise.fail();
                                test(false, "response callback called but exception expected");
                            }
                        ).then(
                            function()
                            {
                                promise.fail();
                                test(false, "response callback called but exception expected");
                            }
                        ).exception(
                            function(e)
                            {
                                //
                                // since no exception handler was passed to the first `.then`, the error propagates.
                                //
                                test(e === "promise.fail");
                                out.writeLine("ok");
                                promise.succeed();
                            });
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Promise exception propagation in exception callback... ");
                        var promise1 = new Promise().fail("promise.fail");
                        promise1.then(
                            function()
                            {
                                promise.fail();
                                test(false, "response callback called but exception expected");
                            },
                            function(ex)
                            {
                                throw "promise.fail";
                            }
                        ).then(
                            function()
                            {
                                promise.fail();
                                test(false, "response callback called but exception expected");
                            }
                        ).then(
                            function()
                            {
                                promise.fail();
                                test(false, "response callback called but exception expected");
                            }
                        ).exception(
                            function(e)
                            {
                                //
                                // since no exception handler was passed to the first `.then`, the error propagates.
                                //
                                test(e === "promise.fail");
                                out.writeLine("ok");
                                promise.succeed();
                            });
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Use Promise.all to wait for several promises and all succeed... ");
                        var promise1 = new Promise();
                        var promise2 = new Promise();
                        var promise3 = new Promise();

                        Promise.all(promise1, promise2, promise3).then(
                            function(r1, r2, r3)
                            {
                                test(r1.length === 1);
                                test(r1[0] === 1024);

                                test(r2.length === 2);
                                test(r2[0] === 1024);
                                test(r2[1] === 2048);

                                test(r3.length === 3);
                                test(r3[0] === 1024);
                                test(r3[1] === 2048);
                                test(r3[2] === 4096);

                                promise.succeed();
                            },
                            function()
                            {
                                promise.fail();
                                test(false);
                            }
                        );

                        //
                        // Now resolve the promise in the reverse order, all succeed callback
                        // will get the result in the right order.
                        //
                        promise3.succeed(1024, 2048, 4096);
                        promise2.succeed(1024, 2048);
                        promise1.succeed(1024);
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        //
                        // Now try the same using an array of promises
                        //
                        var promise1 = new Promise();
                        var promise2 = new Promise();
                        var promise3 = new Promise();

                        Promise.all([promise1, promise2, promise3]).then(
                            function(r1, r2, r3)
                            {
                                test(r1.length === 1);
                                test(r1[0] === 1024);

                                test(r2.length === 2);
                                test(r2[0] === 1024);
                                test(r2[1] === 2048);

                                test(r3.length === 3);
                                test(r3[0] === 1024);
                                test(r3[1] === 2048);
                                test(r3[2] === 4096);

                                out.writeLine("ok");
                                promise.succeed();
                            },
                            function()
                            {
                                promise.fail();
                                test(false);
                            }
                        );

                        //
                        // Now resolve the promise in the reverse order, all succeed callback
                        // will get the result in the right order.
                        //
                        promise3.succeed(1024, 2048, 4096);
                        promise2.succeed(1024, 2048);
                        promise1.succeed(1024);
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Use Promise.all to wait for several promises and one fails... ");
                        var promise1 = new Promise();
                        var promise2 = new Promise();
                        var promise3 = new Promise();

                        Promise.all(promise1, promise2, promise3).then(
                            function(r1, r2, r3)
                            {
                                promise.fail(new Error());
                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                promise.succeed();
                            }
                        );

                        //
                        // Now resolve the promise in the reverse order.
                        //
                        promise3.succeed(1024, 2048, 4096);
                        promise2.succeed(1024, 2048);
                        promise1.fail("promise.fail");
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        //
                        // Same as before but using an array of promises.
                        var promise1 = new Promise();
                        var promise2 = new Promise();
                        var promise3 = new Promise();

                        Promise.all([promise1, promise2, promise3]).then(
                            function(r1, r2, r3)
                            {
                                promise.fail(new Error());
                            },
                            function(e)
                            {
                                test(e === "promise.fail");
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        );

                        //
                        // Now resolve the promise in the reverse order.
                        //
                        promise3.succeed(1024, 2048, 4096);
                        promise2.succeed(1024, 2048);
                        promise1.fail("promise.fail");
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Test finally on a succeed promise... ");

                        var p = new Promise().succeed(1024);
                        var called = false;
                        p.finally(
                            function(i)
                            {
                                called = true;
                                test(i == 1024);
                                return 1025;
                            }
                        ).then(
                            function(i)
                            {
                                test(i == 1024);
                                test(called);
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        ).exception(
                            function(ex)
                            {
                                promise.fail(ex);
                            });
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Test finally on a failed promise... ");

                        var p = new Promise().fail("promise.failed");
                        var called = false;
                        p.finally(
                            function(e)
                            {
                                called = true;
                                test(e == "promise.failed");
                                return "foo";
                            }
                        ).then(
                            function(i)
                            {
                                promise.fail(new Error());
                            }
                        ).exception(
                            function(e)
                            {
                                test(called);
                                test(e == "promise.failed");
                                out.writeLine("ok");
                                promise.succeed();
                            });
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Test finally return a succeed promise... ");

                        var p = new Promise().succeed(1024);
                        var called = false;
                        p.finally(
                            function(e)
                            {
                                called = true;
                                return new Promise().succeed(2048);
                            }
                        ).then(
                            function(i)
                            {
                                test(called);
                                test(i == 1024);
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        ).exception(
                            function(ex)
                            {
                                promise.fail(ex);
                            });
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Test finally return a fail promise... ");

                        var p = new Promise().succeed(1024);
                        var called = false;
                        p.finally(
                            function(e)
                            {
                                called = true;
                                return new Promise().fail(new Error("error"));
                            }
                        ).then(
                            function(i)
                            {
                                test(called);
                                test(i == 1024);
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        ).exception(
                            function(ex)
                            {
                                promise.fail(ex);
                            });
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Test finally throw an exception... ");

                        var p = new Promise().succeed(1024);
                        var called = false;
                        p.finally(
                            function(e)
                            {
                                called = true;
                                throw new Error("error");
                            }
                        ).then(
                            function(i)
                            {
                                test(called);
                                test(i == 1024);
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        ).exception(
                            function(ex)
                            {
                                promise.fail(ex);
                            });
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Test Promise.try... ");

                        Promise.try(
                            function()
                            {
                                out.writeLine("ok");
                                promise.succeed();
                            }
                        ).exception(
                            function()
                            {
                                promise.fail(new Error("test failed"));
                            }
                        );
                    }
                );
            })
        .then(
            function()
            {
                return deferred(
                    function(promise)
                    {
                        out.write("Test promise delay... ");

                        var p = new Promise();

                        var start = Date.now();
                        p = p.succeed(10).delay(500).then(
                            function(i)
                            {
                                test(i == 10);
                                test(Date.now() - start >= 450);
                                test(Date.now() - start <= 900);
                            }
                        ).then(
                            function()
                            {
                                start = Date.now();
                                return Promise.delay(10, 500).then(
                                    function(i)
                                    {
                                        test(i == 10);
                                        test(Date.now() - start >= 450);
                                        test(Date.now() - start <= 900);
                                    });
                            }
                        ).then(
                            function()
                            {
                                var f = new Promise();
                                start = Date.now();
                                return f.fail("failed").delay(500).then(
                                    function(i)
                                    {
                                        test(false);
                                    },
                                    function(ex)
                                    {
                                        test(ex == "failed");
                                        test(Date.now() - start >= 450);
                                        test(Date.now() - start <= 900);
                                        out.writeLine("ok");
                                        promise.succeed();
                                    });
                            }
                        ).exception(
                            function(ex)
                            {
                                promise.fail(ex);
                            });
                    }
                );
            })
        .then(
            function(){
                p.succeed();
            },
            function(ex){
                p.fail(ex);
            }
        );
        return p;
    };
    exports.__test__ = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
