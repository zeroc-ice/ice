// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

import IceInternal.MetricsMap;

public class Observer<T extends Metrics> extends IceUtilInternal.StopWatch implements Ice.Instrumentation.Observer
{
    public interface MetricsUpdate<T>
    {
        void update(T m);
    };
    
    public void
    attach()
    {
        start();
    }

    public void
    detach()
    {
        long lifetime = stop();
        for(MetricsMap<T>.Entry e : _objects)
        {
            e.detach(lifetime);
        }
    }

    public void
    failed(String exceptionName)
    {
        for(MetricsMap<T>.Entry e : _objects)
        {
            e.failed(exceptionName);
        }
    }
    
    public void
    forEach(MetricsUpdate<T> u)
    {
        for(MetricsMap<T>.Entry e : _objects)
        {
            e.execute(u);
        }
    }

    public void
    init(MetricsHelper<T> helper, java.util.List<MetricsMap<T>.Entry> objects)
    {
        assert(_objects == null);
        _objects = objects;
        java.util.Collections.sort(_objects);
        for(MetricsMap<T>.Entry e : _objects)
        {
            e.attach(helper);
        }
    }

    public void
    init(MetricsHelper<T> helper, java.util.List<MetricsMap<T>.Entry> objects, Observer<T> previous)
    {
        _objects = new java.util.LinkedList<MetricsMap<T>.Entry>(previous._objects);
        java.util.Collections.sort(objects);
        java.util.ListIterator<MetricsMap<T>.Entry> p = objects.listIterator();
        java.util.ListIterator<MetricsMap<T>.Entry> q = _objects.listIterator();
        while(p.hasNext())
        {
            MetricsMap<T>.Entry pe = p.next();
            MetricsMap<T>.Entry qe;
            int comp = 0;
            if(q.hasNext())
            {
                qe = q.next();
                comp = pe.compareTo(qe);
            }
            else
            {
                qe = null;
            }

            if(qe == null || comp < 0) // New metrics object
            {
                q.add(pe);
                pe.attach(helper);
            }
            else if(comp == 0) // Same metrics object
            {
                // Nothing to do.
            }
            else // Removed metrics object
            {
                qe.detach(delay());
                q.remove();
                p.previous();
            }
        }
        while(q.hasNext())
        {
            MetricsMap<T>.Entry qe = q.next();
            q.remove();
            qe.detach(delay());
        }
    }

    public <S extends Metrics, ObserverImpl extends Observer<S>> ObserverImpl
    getObserver(String mapName, MetricsHelper<S> helper, Class<S> mcl, Class<ObserverImpl> ocl)
    {
        java.util.List<MetricsMap<S>.Entry> metricsObjects = new java.util.LinkedList<MetricsMap<S>.Entry>();
        for(MetricsMap<T>.Entry entry : _objects)
        {
            MetricsMap<S>.Entry e = entry.getMatching(mapName, helper, mcl);
            if(e != null)
            {
                metricsObjects.add(e);
            }
        }

        if(metricsObjects.isEmpty())
        {
            return null;
        }

        try
        {
            ObserverImpl obsv = ocl.newInstance();
            obsv.init(helper, metricsObjects);
            return obsv;
        }
        catch(Exception ex)
        {
            assert(false);
            return null;
        }
    }
    
    private java.util.List<MetricsMap<T>.Entry> _objects;
};
