// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

import com.zeroc.Ice.Instrumentation.Observer;
import com.zeroc.Ice.MetricsAdminI;

/**
 * Factory for creating observer instances that support delegation.
 *
 * @param <T> the metrics type
 * @param <OImpl> the observer implementation type that supports delegation
 * @param <O> the delegate observer type
 */
public class ObserverFactoryWithDelegate<
    T extends Metrics,
    OImpl extends ObserverWithDelegate<T, O>,
    O extends Observer>
    extends ObserverFactory<T, OImpl> {
    /**
     * Constructs an ObserverFactoryWithDelegate.
     *
     * @param metrics the metrics admin instance
     * @param name the name of the metrics map
     * @param cl the metrics class
     */
    public ObserverFactoryWithDelegate(
            MetricsAdminI metrics, String name, Class<T> cl) {
        super(metrics, name, cl);
    }

    /**
     * Gets an observer instance with a delegate.
     *
     * @param helper the metrics helper
     * @param cl the observer implementation class
     * @param delegate the delegate observer
     * @return the observer instance with delegate set, or the delegate if no metrics maps are enabled
     */
    @SuppressWarnings("unchecked")
    public O getObserver(MetricsHelper<T> helper, Class<OImpl> cl, O delegate) {
        OImpl o = super.getObserver(helper, cl);
        if (o != null) {
            o.setDelegate(delegate);
            return (O) o;
        }
        return delegate;
    }

    /**
     * Gets an observer instance with a delegate and previous observer.
     *
     * @param helper the metrics helper
     * @param observer the previous observer, or null
     * @param cl the observer implementation class
     * @param delegate the delegate observer
     * @return the observer instance with delegate set, or the delegate if no metrics maps are enabled
     */
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
