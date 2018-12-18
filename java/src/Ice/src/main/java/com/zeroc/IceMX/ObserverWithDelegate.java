// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceMX;

public class ObserverWithDelegate<T extends Metrics, O extends com.zeroc.Ice.Instrumentation.Observer>
    extends Observer<T>
{
    @Override
    public void
    attach()
    {
        super.attach();
        if(_delegate != null)
        {
            _delegate.attach();
        }
    }

    @Override
    public void
    detach()
    {
        super.detach();
        if(_delegate != null)
        {
            _delegate.detach();
        }
    }

    @Override
    public void
    failed(String exceptionName)
    {
        super.failed(exceptionName);
        if(_delegate != null)
        {
            _delegate.failed(exceptionName);
        }
    }

    public O
    getDelegate()
    {
        return _delegate;
    }

    public void
    setDelegate(O del)
    {
        _delegate = del;
    }

    @SuppressWarnings("unchecked")
    public <S extends Metrics, ObserverImpl extends ObserverWithDelegate<S, Obs>,
        Obs extends com.zeroc.Ice.Instrumentation.Observer> Obs
    getObserver(String mapName, MetricsHelper<S> helper, Class<S> mcl, Class<ObserverImpl> ocl, Obs delegate)
    {
        ObserverImpl obsv = super.getObserver(mapName, helper, mcl, ocl);
        if(obsv != null)
        {
            obsv.setDelegate(delegate);
            return (Obs)obsv;
        }
        return delegate;
    }

    protected O _delegate;
}
