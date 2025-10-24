// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.echo;

public class BlobjectI : Ice.BlobjectAsync
{
    public void startBatch()
    {
        Debug.Assert(_batchProxy != null);
        _startBatch = true;
    }

    public void flushBatch()
    {
        Debug.Assert(_batchProxy != null);
        _batchProxy.ice_flushBatchRequests();
        _batchProxy = null;
    }

    public override Task<Ice.Object_Ice_invokeResult>
    ice_invokeAsync(byte[] inEncaps, Ice.Current current)
    {
        bool twoway = current.requestId > 0;
        Ice.ObjectPrx obj = current.con.createProxy(current.id);
        if (!twoway)
        {
            if (_startBatch)
            {
                _startBatch = false;
                _batchProxy = obj.ice_batchOneway();
            }
            if (_batchProxy != null)
            {
                obj = _batchProxy;
            }

            if (current.facet.Length != 0)
            {
                obj = obj.ice_facet(current.facet);
            }

            if (_batchProxy != null)
            {
                obj.ice_invoke(current.operation, current.mode, inEncaps, out _, current.ctx);
                return Task.FromResult(new Ice.Object_Ice_invokeResult(true, []));
            }
            else
            {
                return obj.ice_oneway().ice_invokeAsync(current.operation, current.mode, inEncaps, current.ctx);
            }
        }
        else
        {
            if (current.facet.Length != 0)
            {
                obj = obj.ice_facet(current.facet);
            }
            return obj.ice_invokeAsync(current.operation, current.mode, inEncaps, current.ctx);
        }
    }

    private Ice.ObjectPrx _batchProxy;
    private bool _startBatch;
}
