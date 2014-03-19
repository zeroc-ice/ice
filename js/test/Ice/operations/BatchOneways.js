// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var Ice = global.Ice;

    var run = function(communicator, prx, Test, bidir)
    {
        var Promise = Ice.Promise;
        var bs1, bs2, bs3, batch, batch2, batch3;
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
                bs1 = Ice.Buffer.createNative(new Array(10 * 1024));
                for(var i = 0; i < bs1.length; ++i)
                {
                    bs1[i] = 0;
                }
                bs2 = Ice.Buffer.createNative(new Array(99 * 1024));
                for(var i = 0; i < bs2.length; ++i)
                {
                    bs2[i] = 0;
                }
                bs3 = Ice.Buffer.createNative(new Array(100 * 1024));
                for(var i = 0; i < bs3.length; ++i)
                {
                    bs3[i] = 0;
                }

                return prx.opByteSOneway(bs1);
            }
        ).then(
            function()
            {
                return prx.opByteSOneway(bs2);
            }
        ).then(
            function()
            {
                return prx.opByteSOneway(bs3);
            }
        ).then(
            function()
            {
                test(false);
            },
            function(ex)
            {
                test(ex instanceof Ice.MemoryLimitException);

                batch = prx.ice_batchOneway();

                var all = [];
                for(var i = 0; i < 30; ++i)
                {
                    all[i] = batch.opByteSOneway(bs1);
                }

                return Promise.all(all).then(
                    function()
                    {
                        return batch.ice_getConnection();
                    }
                ).then(
                    function(con)
                    {
                        return con.flushBatchRequests();
                    }
                ).then(
                    function()
                    {
                        return prx;
                    });
            }
        ).then(
            function(prx)
            {
                batch2 = prx.ice_batchOneway();

                return Promise.all(batch.ice_ping(), batch2.ice_ping());
            }
        ).then(
            function()
            {
                return batch.ice_flushBatchRequests();
            }
        ).then(
            function()
            {
                return batch.ice_getConnection();
            }
        ).then(
            function(con)
            {
                return con.close(false);
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
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            });
        return p;
    };

    global.BatchOneways = { run: run };
}(typeof (global) === "undefined" ? window : global));
