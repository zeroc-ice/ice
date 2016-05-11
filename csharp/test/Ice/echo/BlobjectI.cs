// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

public class BlobjectI : Ice.BlobjectAsync
{

    private class Callback
    {
        public Callback(Ice.AMD_Object_ice_invoke cb, bool twoway)
        {
            _cb = cb;
            _twoway = twoway;
        }

        public void response(bool ok, byte[] encaps)
        {
            _cb.ice_response(ok, encaps);
        }

        public void exception(Ice.Exception ex)
        {
            _cb.ice_exception(ex);
        }

        public void sent(bool sync)
        {
            if(!_twoway)
            {
                _cb.ice_response(true, new byte[0]);
            }
        }

        private Ice.AMD_Object_ice_invoke _cb;
        private bool _twoway;
    }

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

    public override void
    ice_invoke_async(Ice.AMD_Object_ice_invoke amdCb, byte[] inEncaps, Ice.Current current)
    {
        bool twoway = current.requestId > 0;
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

            if(current.facet.Length != 0)
            {
                obj = obj.ice_facet(current.facet);
            }

            if(_batchProxy != null)
            {
                byte[] outEncaps;
                obj.ice_invoke(current.operation, current.mode, inEncaps, out outEncaps, current.ctx);
                amdCb.ice_response(true, new byte[0]);
            }
            else
            {
                Callback cb = new Callback(amdCb, false);
                obj.ice_oneway().begin_ice_invoke(current.operation,
                                                  current.mode,
                                                  inEncaps,
                                                  current.ctx).whenCompleted(cb.response, cb.exception)
                                                              .whenSent(cb.sent);
            }
        }
        else
        {
            if(current.facet.Length != 0)
            {
                obj = obj.ice_facet(current.facet);
            }
            Callback cb = new Callback(amdCb, true);
            obj.begin_ice_invoke(current.operation,
                                 current.mode,
                                 inEncaps,
                                 current.ctx).whenCompleted(cb.response, cb.exception).whenSent(cb.sent);
        }
    }

    private Ice.ObjectPrx _batchProxy;
    private bool _startBatch;
}
