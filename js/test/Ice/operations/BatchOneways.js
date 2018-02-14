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
    var Test = require("Test").Test;

    var run = function(communicator, prx, Test, bidir)
    {
        var Promise = Ice.Promise;
        var bs1, bs2, batch, batch2, batch3;
        var p = new Promise();
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
                    p.fail(err);
                    throw err;
                }
            }
        };

        Promise.try(
            function()
            {
                var i;
                bs1 = Ice.Buffer.createNative(new Array(10 * 1024));
                for(i = 0; i < bs1.length; ++i)
                {
                    bs1[i] = 0;
                }
                return prx.opByteSOnewayCallCount();
            }
        ).then(
            function(count)
            {
                batch = prx.ice_batchOneway();
                return batch.ice_getConnection();
            }
        ).then(
            function()
            {
                test(batch.ice_flushBatchRequests().isCompleted()); // Empty flush
                test(batch.ice_flushBatchRequests().isSent()); // Empty flush
                test(batch.ice_flushBatchRequests().sentSynchronously()); // Empty flush

                var all = [];
                for(var i = 0; i < 30; ++i)
                {
                    all[i] = batch.opByteSOneway(bs1);
                }

                return Promise.all(all).then(
                    function()
                    {
                        var wait = function(count)
                        {
                            if(count < 27) // 3 * 9 requests auto-flushed.
                            {
                                return Promise.delay(10).then(
                                    function()
                                    {
                                        return prx.opByteSOnewayCallCount();
                                    }
                                ).then(
                                    function(n)
                                    {
                                        return wait(n + count);
                                    }
                                );
                            }
                        };
                        return wait(0);
                    }
                );
            }
        ).then(
            function()
            {
                batch2 = prx.ice_batchOneway();
                return Promise.all(batch.ice_ping(), batch2.ice_ping());
            }
        ).then(
            function(count)
            {
                return batch.ice_flushBatchRequests();
            }
        ).then(
            function()
            {
                return prx.opByteSOnewayCallCount();
            }
        ).then(
            function()
            {
                return batch.ice_getConnection();
            }
        ).then(
            function(con)
            {
                if(!bidir)
                {
                    return con.close(false);
                }
            }
        ).then(
            function()
            {
                return Promise.all(batch.ice_ping(), batch2.ice_ping());
            }
        ).then(
            function()
            {
                var identity = communicator.stringToIdentity("invalid");
                batch3 = batch.ice_identity(identity);
                return batch3.ice_ping();
            }
        ).then(
            function()
            {
                return batch3.ice_flushBatchRequests();
            }
        ).then(
            function()
            {
                // Make sure that a bogus batch request doesn't cause troubles to other ones.
                return Promise.all(batch3.ice_ping(), batch.ice_ping());
            }
        ).then(
            function()
            {
                return batch.ice_flushBatchRequests();
            }
        ).then(
            function()
            {
                return prx.opByteSOnewayCallCount();
            }
        ).then(
            function(count)
            {
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            });
        return p;
    };

    exports.BatchOneways = { run: run };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
