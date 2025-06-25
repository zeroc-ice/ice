// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

import com.zeroc.Ice.MetricsAdminI;
import com.zeroc.Ice.MetricsMap;

import java.util.ArrayList;
import java.util.List;

/**
 * Factory for creating observer instances for metrics collection.
 *
 * @param <T> the metrics type
 * @param <O> the observer type
 */
public class ObserverFactory<T extends Metrics, O extends Observer<T>> {
    /**
     * Constructs an ObserverFactory.
     *
     * @param metrics the metrics admin instance
     * @param name the name of the metrics map
     * @param cl the metrics class
     */
    public ObserverFactory(MetricsAdminI metrics, String name, Class<T> cl) {
        _metrics = metrics;
        _name = name;
        _class = cl;
        _metrics.registerMap(
            name,
            _class,
            () -> {
                update();
            });
    }

    /**
     * Destroys this observer factory and unregisters the metrics map.
     */
    public void destroy() {
        if (_metrics != null) {
            _metrics.unregisterMap(_name);
        }
    }

    /**
     * Gets an observer instance for the specified helper.
     *
     * @param helper the metrics helper
     * @param cl the observer class
     * @return the observer instance, or null if no metrics maps are enabled
     */
    public O getObserver(MetricsHelper<T> helper, Class<O> cl) {
        return getObserver(helper, null, cl);
    }

    /**
     * Gets an observer instance for the specified helper and previous observer.
     *
     * @param helper the metrics helper
     * @param observer the previous observer, or null
     * @param cl the observer class
     * @return the observer instance, or null if no metrics maps are enabled
     */
    @SuppressWarnings("unchecked")
    public synchronized O getObserver(MetricsHelper<T> helper, Object observer, Class<O> cl) {
        O old = null;
        try {
            old = (O) observer;
        } catch (ClassCastException ex) {}
        List<MetricsMap<T>.Entry> metricsObjects = null;
        for (MetricsMap<T> m : _maps) {
            MetricsMap<T>.Entry e = m.getMatching(helper, old != null ? old.getEntry(m) : null);
            if (e != null) {
                if (metricsObjects == null) {
                    metricsObjects = new ArrayList<>(_maps.size());
                }
                metricsObjects.add(e);
            }
        }

        if (metricsObjects == null) {
            if (old != null) {
                old.detach();
            }
            return null;
        }

        O obsv;
        try {
            obsv = cl.getDeclaredConstructor().newInstance();
        } catch (Exception ex) {
            assert false;
            return null;
        }
        obsv.init(helper, metricsObjects, old);
        return obsv;
    }

    /**
     * Registers a sub-map for this observer factory.
     *
     * @param <S> the sub-metrics type
     * @param subMap the name of the sub-map
     * @param cl the sub-metrics class
     * @param field the field to associate with the sub-map
     */
    public <S extends Metrics> void registerSubMap(
            String subMap, Class<S> cl, java.lang.reflect.Field field) {
        _metrics.registerSubMap(_name, subMap, cl, field);
    }

    /**
     * Checks whether this observer factory is enabled.
     *
     * @return true if enabled (has active metrics maps), false otherwise
     */
    public boolean isEnabled() {
        return _enabled;
    }

    /**
     * Updates the metrics maps and enabled state.
     */
    public void update() {
        Runnable updater;
        synchronized (this) {
            _maps.clear();
            for (MetricsMap<T> m : _metrics.getMaps(_name, _class)) {
                _maps.add(m);
            }
            _enabled = !_maps.isEmpty();
            updater = _updater;
        }

        if (updater != null) {
            updater.run();
        }
    }

    /**
     * Sets an updater to be called when the factory is updated.
     *
     * @param updater the updater to call, or null
     */
    public synchronized void setUpdater(Runnable updater) {
        _updater = updater;
    }

    private final MetricsAdminI _metrics;
    private final String _name;
    private final Class<T> _class;
    private final List<MetricsMap<T>> _maps = new ArrayList<>();
    private volatile boolean _enabled;
    private Runnable _updater;
}
