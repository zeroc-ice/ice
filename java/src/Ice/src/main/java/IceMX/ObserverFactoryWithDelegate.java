// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

public class ObserverFactoryWithDelegate<T extends Metrics,
                                         OImpl extends ObserverWithDelegate<T, O>,
                                         O extends Ice.Instrumentation.Observer>
    extends ObserverFactory<T, OImpl>
{
    public
    ObserverFactoryWithDelegate(IceInternal.MetricsAdminI metrics, String name, Class<T> cl)
    {
        super(metrics, name, cl);
    }

    @SuppressWarnings("unchecked")
    public O
    getObserver(MetricsHelper<T> helper, Class<OImpl> cl, O delegate)
    {
        OImpl o = super.getObserver(helper, cl);
        if(o != null)
        {
            o.setDelegate(delegate);
            return (O)o;
        }
        return delegate;
    }

    @SuppressWarnings("unchecked")
    public O
    getObserver(MetricsHelper<T> helper, Object observer, Class<OImpl> cl, O delegate)
    {
        OImpl o = super.getObserver(helper, observer, cl);
        if(o != null)
        {
            o.setDelegate(delegate);
            return (O)o;
        }
        return delegate;
    }
}
