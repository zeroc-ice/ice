// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
        var bs1, bs2, batch, batch2, batch3;
        var p = new Ice.Promise();
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
                    p.reject(err);
                    throw err;
                }
            }
        };

        Ice.Promise.try(() =>
            {
                var i;
                bs1 = new Uint8Array(10 * 1024);
                for(i = 0; i < bs1.length; ++i)
                {
                    bs1[i] = 0;
                }
                return prx.opByteSOnewayCallCount();
            }
        ).then(count =>
            {
                batch = prx.ice_batchOneway();
                return batch.ice_getConnection();
            }
        ).then(() =>
            {
                test(batch.ice_flushBatchRequests().isCompleted()); // Empty flush
                test(batch.ice_flushBatchRequests().isSent()); // Empty flush
                test(batch.ice_flushBatchRequests().sentSynchronously()); // Empty flush

                var all = [];
                for(var i = 0; i < 30; ++i)
                {
                    all[i] = batch.opByteSOneway(bs1);
                }

                return Ice.Promise.all(all).then(() =>
                    {
                        var wait = function(count)
                        {
                            if(count < 27) // 3 * 9 requests auto-flushed.
                            {
                                return Ice.Promise.delay(10)
                                              .then(() => prx.opByteSOnewayCallCount())
                                              .then(n => wait(n + count));
                            }
                        };
                        return wait(0);
                    });
            }
        ).then(() =>
            {
                batch2 = prx.ice_batchOneway();
                return Ice.Promise.all([batch.ice_ping(), batch2.ice_ping()]);
            }
        ).then(count => batch.ice_flushBatchRequests()
        ).then(() => prx.opByteSOnewayCallCount()
        ).then(() => batch.ice_getConnection()
        ).then(con => bidir ? undefined : con.close(Ice.ConnectionClose.GracefullyWithWait)
        ).then(() => Ice.Promise.all([batch.ice_ping(), batch2.ice_ping()])
        ).then(() =>
            {
                var identity = Ice.stringToIdentity("invalid");
                batch3 = batch.ice_identity(identity);
                return batch3.ice_ping();
            }
        ).then(() => batch3.ice_flushBatchRequests()
        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        ).then(() => Ice.Promise.all([batch3.ice_ping(), batch.ice_ping()])
        ).then(() => batch.ice_flushBatchRequests()
        ).then(() => prx.opByteSOnewayCallCount()
        ).then(p.resolve, p.reject);
        return p;
    };

    exports.BatchOneways = { run: run };
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
