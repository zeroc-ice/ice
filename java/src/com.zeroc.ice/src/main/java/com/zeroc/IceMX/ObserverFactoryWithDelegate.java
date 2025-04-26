// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

import com.zeroc.Ice.Instrumentation.Observer;
import com.zeroc.Ice.MetricsAdminI;

public class ObserverFactoryWithDelegate<
    T extends Metrics,
    OImpl extends ObserverWithDelegate<T, O>,
    O extends Observer>
    extends ObserverFactory<T, OImpl> {
    public ObserverFactoryWithDelegate(
            MetricsAdminI metrics, String name, Class<T> cl) {
        super(metrics, name, cl);
    }

    @SuppressWarnings("unchecked")
    public O getObserver(MetricsHelper<T> helper, Class<OImpl> cl, O delegate) {
        OImpl o = super.getObserver(helper, cl);
        if (o != null) {
            o.setDelegate(delegate);
            return (O) o;
        }
        return delegate;
    }

    @SuppressWarnings("unchecked")
    public O getObserver(MetricsHelper<T> helper, Object observer, Class<OImpl> cl, O delegate) {
        OImpl o = super.getObserver(helper, observer, cl);
        if (o != null) {
            o.setDelegate(delegate);
            return (O) o;
        }
        return delegate;
    }
}
