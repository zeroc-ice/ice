// Copyright (c) ZeroC, Inc.

package test.Ice.echo;

import com.zeroc.Ice.BlobjectAsync;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Util;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public class BlobjectI implements BlobjectAsync {
    public void startBatch() {
        assert (_batchProxy == null);
        _startBatch = true;
    }

    public void flushBatch() {
        assert (_batchProxy != null);
        _batchProxy.ice_flushBatchRequests();
        _batchProxy = null;
    }

    @Override
    public CompletionStage<Object.Ice_invokeResult> ice_invokeAsync(
            byte[] inEncaps, Current current) {
        boolean twoway = current.requestId > 0;
        ObjectPrx obj = current.con.createProxy(current.id);
        if (!twoway) {
            if (_startBatch) {
                _startBatch = false;
                _batchProxy = obj.ice_batchOneway();
            }
            if (_batchProxy != null) {
                obj = _batchProxy;
            }

            if (!current.facet.isEmpty()) {
                obj = obj.ice_facet(current.facet);
            }

            final Object.Ice_invokeResult success =
                new Object.Ice_invokeResult(true, new byte[0]);

            if (_batchProxy != null) {
                obj.ice_invoke(current.operation, current.mode, inEncaps, current.ctx);
                return CompletableFuture.completedFuture(success);
            } else {
                final CompletableFuture<Object.Ice_invokeResult> future =
                    new CompletableFuture<Object.Ice_invokeResult>();
                CompletableFuture<Object.Ice_invokeResult> r =
                    obj.ice_oneway()
                        .ice_invokeAsync(
                            current.operation, current.mode, inEncaps, current.ctx);
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            if (ex != null) {
                                future.completeExceptionally(ex);
                            } else {
                                future.complete(success);
                            }
                        });
                return future;
            }
        } else {
            if (!current.facet.isEmpty()) {
                obj = obj.ice_facet(current.facet);
            }

            final CompletableFuture<Object.Ice_invokeResult> future =
                new CompletableFuture<Object.Ice_invokeResult>();
            obj.ice_invokeAsync(current.operation, current.mode, inEncaps, current.ctx)
                .whenComplete(
                    (result, ex) -> {
                        if (ex != null) {
                            future.completeExceptionally(ex);
                        } else {
                            future.complete(result);
                        }
                    });
            return future;
        }
    }

    private ObjectPrx _batchProxy;
    private boolean _startBatch;
}
