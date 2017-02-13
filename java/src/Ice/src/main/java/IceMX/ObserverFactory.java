// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

import IceInternal.MetricsMap;

public class ObserverFactory<T extends Metrics, O extends Observer<T>>
{
    public
    ObserverFactory(IceInternal.MetricsAdminI metrics, String name, Class<T> cl)
    {
        _metrics = metrics;
        _name = name;
        _class = cl;
        _metrics.registerMap(name, _class, new Runnable()
            {
                @Override
                public void
                run()
                {
                    update();
                }
            });
    }

    public void
    destroy()
    {
        if(_metrics != null)
        {
            _metrics.unregisterMap(_name);
        }
    }

    public O
    getObserver(MetricsHelper<T> helper, Class<O> cl)
    {
        return getObserver(helper, null, cl);
    }

    @SuppressWarnings("unchecked")
    public synchronized O
    getObserver(MetricsHelper<T> helper, Object observer, Class<O> cl)
    {
        O old = null;
        try
        {
            old = (O)observer;
        }
        catch(ClassCastException ex)
        {
        }
        java.util.List<MetricsMap<T>.Entry> metricsObjects = null;
        for(MetricsMap<T> m : _maps)
        {
            MetricsMap<T>.Entry e = m.getMatching(helper, old != null ? old.getEntry(m) : null);
            if(e != null)
            {
                if(metricsObjects == null)
                {
                    metricsObjects = new java.util.ArrayList<MetricsMap<T>.Entry>(_maps.size());
                }
                metricsObjects.add(e);
            }
        }

        if(metricsObjects == null)
        {
            if(old != null)
            {
                old.detach();
            }
            return null;
        }

        O obsv;
        try
        {
            obsv = cl.newInstance();
        }
        catch(Exception ex)
        {
            assert(false);
            return null;
        }
        obsv.init(helper, metricsObjects, old);
        return obsv;
    }

    public <S extends IceMX.Metrics> void
    registerSubMap(String subMap, Class<S> cl, java.lang.reflect.Field field)
    {
        _metrics.registerSubMap(_name, subMap, cl, field);
    }

    public boolean
    isEnabled()
    {
        return _enabled;
    }

    public void
    update()
    {
        Runnable updater;
        synchronized(this)
        {
            _maps.clear();
            for(MetricsMap<T> m : _metrics.getMaps(_name, _class))
            {
                _maps.add(m);
            }
            _enabled = !_maps.isEmpty();
            updater = _updater;
        }

        if(updater != null)
        {
            updater.run();
        }
    }

    public synchronized void
    setUpdater(Runnable updater)
    {
        _updater = updater;
    }

    private final IceInternal.MetricsAdminI _metrics;
    private final String _name;
    private final Class<T> _class;
    private java.util.List<MetricsMap<T>> _maps = new java.util.ArrayList<MetricsMap<T>>();
    private volatile boolean _enabled;
    private Runnable _updater;
}