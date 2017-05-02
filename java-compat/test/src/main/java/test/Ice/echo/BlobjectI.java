// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.echo;

public class BlobjectI extends Ice.BlobjectAsync
{
    class Callback extends Ice.Callback_Object_ice_invoke
    {
        public Callback(Ice.AMD_Object_ice_invoke cb, boolean twoway)
        {
            _cb = cb;
            _twoway = twoway;
        }

        @Override
        public void response(boolean ok, byte[] encaps)
        {
            _cb.ice_response(ok, encaps);
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            _cb.ice_exception(ex);
        }

        @Override
        public void sent(boolean sync)
        {
            if(!_twoway)
            {
                _cb.ice_response(true, new byte[0]);
            }
        }

        final Ice.AMD_Object_ice_invoke _cb;
        final boolean _twoway;
    };

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
    public void
    ice_invoke_async(Ice.AMD_Object_ice_invoke amdCb, byte[] inEncaps, Ice.Current current)
    {
        boolean twoway = current.requestId > 0;
        Ice.ObjectPrx obj = current.con.createProxy(current.id);
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

            if(_batchProxy != null)
            {
                Ice.ByteSeqHolder out = new Ice.ByteSeqHolder();
                obj.ice_invoke(current.operation, current.mode, inEncaps, out, current.ctx);
                amdCb.ice_response(true, new byte[0]);
            }
            else
            {
                Callback cb = new Callback(amdCb, false);
                obj.ice_oneway().begin_ice_invoke(current.operation, current.mode, inEncaps, current.ctx, cb);
            }
        }
        else
        {
            if(!current.facet.isEmpty())
            {
                obj = obj.ice_facet(current.facet);
            }

            Callback cb = new Callback(amdCb, true);
            obj.begin_ice_invoke(current.operation, current.mode, inEncaps, current.ctx, cb);
        }
    }

    private Ice.ObjectPrx _batchProxy;
    private boolean _startBatch;
}
