// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;
    var Promise = Ice.Promise;

    function allTests(communicator, out)
    {
        var promise = new Ice.Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    promise.reject(err);
                    throw err;
                }
            }
        };

        var getConnectionBatchProxy = function(proxy, connectionId)
        {
            if(!connectionId)
            {
                connectionId = "";
            }
            var p = proxy;
            return p.ice_connectionId(connectionId).ice_getConnection().then(c =>
                {
                    p = p.constructor.uncheckedCast(c.createProxy(proxy.ice_getIdentity())).ice_batchOneway();
                    return p.ice_getConnection();
                }
            ).then(c =>
                {
                    test(p.ice_getCachedConnection() === c);
                    return p;
                });
        };

        var result = null;
        var p = Test.TestIntfPrx.uncheckedCast(communicator.stringToProxy("test:default -p 12010"));
        var testController =
            Test.TestIntfControllerPrx.uncheckedCast(communicator.stringToProxy("testController:default -p 12011"));
        var r = null;
        var b1 = null;
        var b2 = null;

        Promise.try(() =>
            {
                out.write("testing batch requests with proxy... ");
                return p.opBatchCount().then(count =>
                    {
                        test(count === 0);
                        b1 = p.ice_batchOneway();
                        test(b1.opBatch());
                        test(b1.opBatch());
                        return b1.ice_flushBatchRequests();
                    }
                ).then(() => p.waitForBatch(2)
                ).then(() => b1.ice_flushBatchRequests()
                ).then(() => out.writeLine("ok"));
            }
        ).then(() =>
            {
                out.write("testing batch requests with connection... ");
                return p.opBatchCount().then(count =>
                    {
                        test(count === 0);
                        return getConnectionBatchProxy(p).then(prx =>
                            {
                                b1 = prx;
                                var connection = b1.ice_getCachedConnection();
                                test(b1.opBatch());
                                test(b1.opBatch());
                                return connection.flushBatchRequests();
                            });
                    }
                ).then(() => p.waitForBatch(2)
                ).then(() => b1.ice_getConnection().then(connection => connection.flushBatchRequests())
                ).then(() => out.writeLine("ok"));
            }
        ).then(() =>
            {
                out.write("testing batch requests with communicator... ");
                return p.opBatchCount().then(count =>
                    {
                        test(count === 0);
                        test(b1.opBatch());
                        test(b1.opBatch());
                        return communicator.flushBatchRequests().then(() => p.waitForBatch(2))
                                                                .then(() => p.opBatchCount());
                    }
                ).then(batchCount =>
                    {
                        //
                        // AsyncResult exception - 1 connection.
                        //
                        test(batchCount === 0);
                        b1.opBatch();
                        b1.ice_getCachedConnection().close(false);
                        return communicator.flushBatchRequests().then(() => p.opBatchCount());
                    }
                ).then(batchCount =>
                    {
                        //
                        // AsyncResult exception - 2 connections
                        //
                        test(batchCount === 0);
                        return getConnectionBatchProxy(p).then(prx =>
                            {
                                b1 = prx;
                                return getConnectionBatchProxy(p, "2");
                            }
                        ).then(prx => // Ensure connection is established.
                            {
                                b2 = prx;
                                b1.opBatch();
                                b1.opBatch();
                                b2.opBatch();
                                b2.opBatch();
                                return communicator.flushBatchRequests();
                            }
                        ).then(() => p.waitForBatch(4))
                         .then(() => p.opBatchCount());
                    }
                ).then(batchCount =>
                    {
                        //
                        // AsyncResult exception - 2 connections - 1 failure.
                        //
                        // All connections should be flushed even if there are failures on some connections.
                        // Exceptions should not be reported.
                        //
                        test(batchCount === 0);
                        return getConnectionBatchProxy(p).then(prx =>
                            {
                                b1 = prx;
                                return getConnectionBatchProxy(p, "2");
                            }
                        ).then(prx => // Ensure connection is established.
                            {
                                b2 = prx;
                                b1.opBatch();
                                b2.opBatch();
                                b1.ice_getCachedConnection().close(false);
                                return communicator.flushBatchRequests();
                            }
                        ).then(() => p.waitForBatch(1)
                        ).then(() => p.opBatchCount());
                    }
                ).then(batchCount =>
                    {
                        //
                        // AsyncResult exception - 2 connections - 2 failures.
                        //
                        // All connections should be flushed even if there are failures on some connections.
                        // Exceptions should not be reported.
                        //
                        test(batchCount === 0);
                        return getConnectionBatchProxy(p).then(prx =>
                            {
                                b1 = prx;
                                return getConnectionBatchProxy(p, "2");
                            }
                        ).then(prx => // Ensure connection is established.
                            {
                                b2 = prx;
                                b1.opBatch();
                                b2.opBatch();
                                b1.ice_getCachedConnection().close(false);
                                b2.ice_getCachedConnection().close(false);
                                return communicator.flushBatchRequests();
                            }
                        ).then(() => p.opBatchCount());
                    }
                ).then(batchCount =>
                    {
                        test(batchCount === 0);
                        out.writeLine("ok");
                    });
            }
        ).then(() =>
            {
                out.write("testing AsyncResult operations... ");

                var indirect = Test.TestIntfPrx.uncheckedCast(p.ice_adapterId("dummy"));
                
                return indirect.op().catch(ex => test(ex instanceof Ice.NoEndpointException)
                ).then(() => testController.holdAdapter()
                ).then(() =>
                    {
                        var r1 = p.op();
                        var r2 = null;
                        var seq = Ice.Buffer.createNative(new Array(100000));

                        while((r2 = p.opWithPayload(seq)).sentSynchronously());
                        test(r1.sentSynchronously() && r1.isSent() && !r1.isCompleted() ||
                             !r1.sentSynchronously() && !r1.isCompleted());

                        test(!r2.sentSynchronously() && !r2.isCompleted());

                        testController.resumeAdapter();

                        test(r1.operation === "op");
                        test(r2.operation === "opWithPayload");

                        return r1.then(() =>
                            {
                                test(r1.isSent());
                                test(r1.isCompleted());
                                return r2;
                            }
                        ).then(() =>
                            {
                                test(r2.isSent());
                                test(r2.isCompleted());
                            });
                    }
                ).then(() =>
                    {
                        r = p.ice_ping();
                        test(r.operation === "ice_ping");
                        test(r.connection === null); // Expected
                        test(r.communicator == communicator);
                        test(r.proxy == p);

                        //
                        // Oneway
                        //
                        var p2 = p.ice_oneway();
                        r = p2.ice_ping();
                        test(r.operation === "ice_ping");
                        test(r.connection === null); // Expected
                        test(r.communicator == communicator);
                        test(r.proxy == p2);

                        //
                        // Batch request via proxy
                        //
                        p2 = p.ice_batchOneway();
                        p2.ice_ping();
                        r = p2.ice_flushBatchRequests();
                        test(r.operation === "ice_flushBatchRequests");
                        test(r.connection === null); // Expected
                        test(r.communicator == communicator);
                        test(r.proxy == p2);

                        var con = p.ice_getCachedConnection();
                        p2 = p.ice_batchOneway();
                        p2.ice_ping();
                        r = con.flushBatchRequests();
                        test(r.operation === "flushBatchRequests");
                        test(r.connection == con);
                        test(r.communicator == communicator);
                        test(r.proxy === null);

                        p2 = p.ice_batchOneway();
                        p2.ice_ping();
                        r = communicator.flushBatchRequests();
                        test(r.operation === "flushBatchRequests");
                        test(r.connection === null);
                        test(r.communicator == communicator);
                        test(r.proxy === null);
                    }
                ).then(() => testController.holdAdapter()
                ).then(() =>
                    {
                        var seq = Ice.Buffer.createNative(new Array(100000));
                        while((r = p.opWithPayload(seq)).sentSynchronously());
                        test(!r.isSent());

                        var r1 = p.ice_ping();
                        r1.then(
                            () => test(false),
                            (ex) => test(ex instanceof Ice.InvocationCanceledException));

                        var r2 = p.ice_id();
                        r2.then(
                            () => test(false),
                            (ex) => test(ex instanceof Ice.InvocationCanceledException));
                    
                        r1.cancel();
                        r2.cancel();

                        return testController.resumeAdapter()
                            .then(() => p.ice_ping())
                            .then(() =>
                                {
                                    test(!r1.isSent() && r1.isCompleted());
                                    test(!r2.isSent() && r2.isCompleted());
                                });
                    }
                ).then(() => testController.holdAdapter()
                ).then(() =>
                    {
                        var r1 = p.op();
                        var r2 = p.ice_id();
                        return p.ice_oneway().ice_ping().then(() =>
                            {
                                r1.cancel();
                                r1.then(
                                    () => test(false),
                                    (ex) => test(ex instanceof Ice.InvocationCanceledException));

                                r2.cancel();
                                r2.then(
                                    () => test(false),
                                    (ex) => test(ex instanceof Ice.InvocationCanceledException));

                                return testController.resumeAdapter();
                            });
                    }
                ).then(() => out.writeLine("ok"));
            }
        ).then(
            () => p.shutdown(),
            ex =>
            {
                console.log("unexpected exception:\n" + ex);
                test(false);
            }
        ).then(promise.resolve, promise.reject);
        return promise;
    }

    exports.__test__ = function(out, id)
    {
        var communicator = Ice.initialize(id);
        return Promise.try(() =>
            {
                if(typeof(navigator) !== 'undefined' && isSafari() && isWorker())
                {
                    out.writeLine("Test not supported with Safari web workers.");
                    return Test.TestIntfPrx.uncheckedCast(
                        communicator.stringToProxy("test:default -p 12010")).shutdown();
                }
                else
                {
                    return allTests(communicator, out);
                }
            }).finally(() => communicator.destroy());
    };
    exports.__runServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
