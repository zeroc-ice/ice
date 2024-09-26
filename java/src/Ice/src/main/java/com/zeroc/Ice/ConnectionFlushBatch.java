//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.util.concurrent.Callable;

class ConnectionFlushBatch extends OutgoingAsyncBase<Void> {
    public ConnectionFlushBatch(ConnectionI con, Communicator communicator, Instance instance) {
        super(communicator, instance, "flushBatchRequests");
        _connection = con;
    }

    @Override
    public Connection getConnection() {
        return _connection;
    }

    @Override
    protected void markCompleted() {
        complete(null);
    }

    public void invoke(CompressBatch compressBatch) {
        _observer = ObserverHelper.get(_instance, "flushBatchRequests");
        try {
            final BatchRequestQueue.SwapResult r = _connection.getBatchRequestQueue().swap(_os);
            int status;
            if (r == null) {
                status = AsyncStatus.Sent;
                if (sent()) {
                    status |= AsyncStatus.InvokeSentCallback;
                }
            } else if (_instance.queueRequests()) {
                status =
                        _instance
                                .getQueueExecutor()
                                .execute(
                                        new Callable<Integer>() {
                                            @Override
                                            public Integer call() throws RetryException {
                                                boolean comp = false;
                                                if (compressBatch == CompressBatch.Yes) {
                                                    comp = true;
                                                } else if (compressBatch == CompressBatch.No) {
                                                    comp = false;
                                                } else {
                                                    comp = r.compress;
                                                }
                                                return _connection.sendAsyncRequest(
                                                        ConnectionFlushBatch.this,
                                                        comp,
                                                        false,
                                                        r.batchRequestNum);
                                            }
                                        });
            } else {
                boolean comp = false;
                if (compressBatch == CompressBatch.Yes) {
                    comp = true;
                } else if (compressBatch == CompressBatch.No) {
                    comp = false;
                } else {
                    comp = r.compress;
                }
                status = _connection.sendAsyncRequest(this, comp, false, r.batchRequestNum);
            }

            if ((status & AsyncStatus.Sent) > 0) {
                _sentSynchronously = true;
                if ((status & AsyncStatus.InvokeSentCallback) > 0) {
                    invokeSent();
                }
            }
        } catch (RetryException ex) {
            if (completed(ex.get())) {
                invokeCompletedAsync();
            }
        } catch (LocalException ex) {
            if (completed(ex)) {
                invokeCompletedAsync();
            }
        }
    }

    private ConnectionI _connection;
}