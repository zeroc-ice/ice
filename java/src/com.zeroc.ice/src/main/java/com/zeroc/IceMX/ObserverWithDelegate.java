// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

/**
 * Observer implementation that supports delegation to another observer.
 *
 * @param <T> the metrics type
 * @param <O> the delegate observer type
 */
public class ObserverWithDelegate<
    T extends Metrics, O extends com.zeroc.Ice.Instrumentation.Observer>
    extends Observer<T> {
    @Override
    public void attach() {
        super.attach();
        if (_delegate != null) {
            _delegate.attach();
        }
    }

    @Override
    public void detach() {
        super.detach();
        if (_delegate != null) {
            _delegate.detach();
        }
    }

    @Override
    public void failed(String exceptionName) {
        super.failed(exceptionName);
        if (_delegate != null) {
            _delegate.failed(exceptionName);
        }
    }

    /**
     * Gets the delegate observer.
     *
     * @return the delegate observer, or null if none is set
     */
    public O getDelegate() {
        return _delegate;
    }

    /**
     * Sets the delegate observer.
     *
     * @param del the delegate observer to set
     */
    public void setDelegate(O del) {
        _delegate = del;
    }

    /**
     * Gets a sub-observer with a delegate for the specified metrics type.
     *
     * @param <S> the sub-metrics type
     * @param <ObserverImpl> the observer implementation type that supports delegation
     * @param <Obs> the delegate observer type
     * @param mapName the name of the metrics map
     * @param helper the metrics helper for the sub-metrics type
     * @param mcl the metrics class
     * @param ocl the observer class
     * @param delegate the delegate observer
     * @return the sub-observer with delegate set, or the delegate if no matching entries are found
     */
    @SuppressWarnings("unchecked")
    public <
        S extends Metrics,
        ObserverImpl extends ObserverWithDelegate<S, Obs>,
        Obs extends com.zeroc.Ice.Instrumentation.Observer>
        Obs getObserver(
                    String mapName,
                    MetricsHelper<S> helper,
                    Class<S> mcl,
                    Class<ObserverImpl> ocl,
                    Obs delegate) {
        ObserverImpl obsv = super.getObserver(mapName, helper, mcl, ocl);
        if (obsv != null) {
            obsv.setDelegate(delegate);
            return (Obs) obsv;
        }
        return delegate;
    }

    /** The delegate observer. */
    protected O _delegate;
}
