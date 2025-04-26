// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

import com.zeroc.Ice.MetricsMap;

import java.util.ArrayList;
import java.util.List;

public class Observer<T extends Metrics> extends StopWatch
    implements com.zeroc.Ice.Instrumentation.Observer {
    public interface MetricsUpdate<T> {
        void update(T m);
    }

    @Override
    public void attach() {
        if (!isStarted()) {
            start();
        }
    }

    @Override
    public void detach() {
        long lifetime = _previousDelay + stop();
        for (MetricsMap<T>.Entry e : _objects) {
            e.detach(lifetime);
        }
    }

    @Override
    public void failed(String exceptionName) {
        for (MetricsMap<T>.Entry e : _objects) {
            e.failed(exceptionName);
        }
    }

    public void forEach(MetricsUpdate<T> u) {
        for (MetricsMap<T>.Entry e : _objects) {
            e.execute(u);
        }
    }

    public void init(
            MetricsHelper<T> helper,
            List<MetricsMap<T>.Entry> objects,
            Observer<T> previous) {
        _objects = objects;

        if (previous == null) {
            return;
        }

        _previousDelay = previous._previousDelay + previous.delay();

        //
        // Detach entries from previous observer which are no longer
        // attached to this new observer.
        //
        for (MetricsMap<T>.Entry p : previous._objects) {
            if (!_objects.contains(p)) {
                p.detach(_previousDelay);
            }
        }
    }

    public <S extends Metrics, ObserverImpl extends Observer<S>> ObserverImpl getObserver(
            String mapName, MetricsHelper<S> helper, Class<S> mcl, Class<ObserverImpl> ocl) {
        List<MetricsMap<S>.Entry> metricsObjects = null;
        for (MetricsMap<T>.Entry entry : _objects) {
            MetricsMap<S>.Entry e = entry.getMatching(mapName, helper, mcl);
            if (e != null) {
                if (metricsObjects == null) {
                    metricsObjects = new ArrayList<>(_objects.size());
                }
                metricsObjects.add(e);
            }
        }

        if (metricsObjects == null) {
            return null;
        }

        try {
            ObserverImpl obsv = ocl.getDeclaredConstructor().newInstance();
            obsv.init(helper, metricsObjects, null);
            return obsv;
        } catch (Exception ex) {
            assert false;
            return null;
        }
    }

    public MetricsMap<T>.Entry getEntry(MetricsMap<?> map) {
        for (MetricsMap<T>.Entry e : _objects) {
            if (e.getMap() == map) {
                return e;
            }
        }
        return null;
    }

    private List<MetricsMap<T>.Entry> _objects;
    private long _previousDelay;
}
