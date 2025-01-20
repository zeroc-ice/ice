// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class CommunicatorFlushBatch extends InvocationFuture<Void> {
    public CommunicatorFlushBatch(Communicator communicator, Instance instance) {
        super(communicator, instance, "flushBatchRequests");

        //
        // _useCount is initialized to 1 to prevent premature callbacks.
        // The caller must invoke ready() after all flush requests have
        // been initiated.
        //
        _useCount = 1;
    }

    @Override
    protected void markCompleted() {
        complete(null);
    }

    public void flushConnection(final ConnectionI con, final CompressBatch compressBatch) {
        class FlushBatch extends OutgoingAsyncBase<Void> {
            public FlushBatch() {
                super(
                        CommunicatorFlushBatch.this.getCommunicator(),
                        CommunicatorFlushBatch.this._instance,
                        CommunicatorFlushBatch.this.getOperation());
            }

            @Override
            protected void markCompleted() {
                assert (false);
            }

            @Override
            public boolean sent() {
                if (_childObserver != null) {
                    _childObserver.detach();
                    _childObserver = null;
                }
                doCheck(false);
                return false;
            }

            @Override
            public boolean completed(LocalException ex) {
                if (_childObserver != null) {
                    _childObserver.failed(ex.ice_id());
                    _childObserver.detach();
                    _childObserver = null;
                }
                doCheck(false);
                return false;
            }

            @Override
            protected com.zeroc.Ice.Instrumentation.InvocationObserver getObserver() {
                return CommunicatorFlushBatch.this._observer;
            }
        }

        synchronized (this) {
            ++_useCount;
        }

        try {
            final FlushBatch flushBatch = new FlushBatch();
            final BatchRequestQueue.SwapResult r =
                    con.getBatchRequestQueue().swap(flushBatch.getOs());
            if (r == null) {
                flushBatch.sent();
            } else {
                boolean comp = false;
                if (compressBatch == CompressBatch.Yes) {
                    comp = true;
                } else if (compressBatch == CompressBatch.No) {
                    comp = false;
                } else {
                    comp = r.compress;
                }
                con.sendAsyncRequest(flushBatch, comp, false, r.batchRequestNum);
            }
        } catch (RetryException ex) {
            doCheck(false);
            throw ex.get();
        } catch (LocalException ex) {
            doCheck(false);
            throw ex;
        }
    }

    public void invoke(CompressBatch compressBatch) {
        _observer = ObserverHelper.get(_instance, "flushBatchRequests");
        _instance.outgoingConnectionFactory().flushAsyncBatchRequests(compressBatch, this);
        _instance.objectAdapterFactory().flushAsyncBatchRequests(compressBatch, this);
        doCheck(true);
    }

    public void waitForResponse() {
        try {
            get();
        } catch (InterruptedException ex) {
            throw new OperationInterruptedException(ex);
        } catch (java.util.concurrent.ExecutionException ee) {
            try {
                throw ee.getCause().fillInStackTrace();
            } catch (RuntimeException ex) // Includes LocalException
            {
                throw ex;
            } catch (Throwable ex) {
                throw new UnknownException(ex);
            }
        }
    }

    private void doCheck(boolean userThread) {
        synchronized (this) {
            assert (_useCount > 0);
            if (--_useCount > 0) {
                return;
            }
        }

        if (sent(true)) {
            if (userThread) {
                _sentSynchronously = true;
                invokeSent();
            } else {
                invokeSentAsync();
            }
        }
    }

    private int _useCount;
}
