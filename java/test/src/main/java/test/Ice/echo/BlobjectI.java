// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.echo;

import com.zeroc.Ice.InvocationFuture;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Current;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

public class BlobjectI implements com.zeroc.Ice.BlobjectAsync
{
    public void startBatch()
    {
        assert(_batchProxy == null);
        _startBatch = true;
    }

    public void flushBatch()
    {
        assert(_batchProxy != null);
        _batchProxy.ice_flushBatchRequests();
        _batchProxy = null;
    }

    @Override
    public CompletionStage<com.zeroc.Ice.Object.Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current)
    {
        boolean twoway = current.requestId > 0;
        ObjectPrx obj = current.con.createProxy(current.id);
        if(!twoway)
        {
            if(_startBatch)
            {
                _startBatch = false;
                _batchProxy = obj.ice_batchOneway();
            }
            if(_batchProxy != null)
            {
                obj = _batchProxy;
            }

            if(!current.facet.isEmpty())
            {
                obj = obj.ice_facet(current.facet);
            }

            final com.zeroc.Ice.Object.Ice_invokeResult success =
                new com.zeroc.Ice.Object.Ice_invokeResult(true, new byte[0]);

            if(_batchProxy != null)
            {
                obj.ice_invoke(current.operation, current.mode, inEncaps, current.ctx);
                return CompletableFuture.completedFuture(success);
            }
            else
            {
                final CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> future =
                    new CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult>();
                CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> r =
                    obj.ice_oneway().ice_invokeAsync(current.operation, current.mode, inEncaps, current.ctx);
                com.zeroc.Ice.Util.getInvocationFuture(r).whenSent((sentSynchronously, ex) ->
                    {
                        if(ex != null)
                        {
                            future.completeExceptionally(ex);
                        }
                        else
                        {
                            future.complete(success);
                        }
                    });
                return future;
            }
        }
        else
        {
            if(!current.facet.isEmpty())
            {
                obj = obj.ice_facet(current.facet);
            }

            final CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> future =
                new CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult>();
            obj.ice_invokeAsync(current.operation, current.mode, inEncaps, current.ctx).whenComplete((result, ex) ->
                {
                    if(ex != null)
                    {
                        future.completeExceptionally(ex);
                    }
                    else
                    {
                        future.complete(result);
                    }
                });
            return future;
        }
    }

    private ObjectPrx _batchProxy;
    private boolean _startBatch;
}
