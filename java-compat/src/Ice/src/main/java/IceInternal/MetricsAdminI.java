// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class MetricsAdminI extends IceMX._MetricsAdminDisp implements Ice.PropertiesAdminUpdateCallback
{
    final static private String[] suffixes =
    {
        "Disabled",
        "GroupBy",
        "Accept.*",
        "Reject.*",
        "RetainDetached",
        "Map.*",
    };

    static void
    validateProperties(String prefix, Ice.Properties properties)
    {
        java.util.Map<String, String> props = properties.getPropertiesForPrefix(prefix);
        java.util.List<String> unknownProps = new java.util.ArrayList<String>();
        for(String prop : props.keySet())
        {
            boolean valid = false;
            for(String suffix : suffixes)
            {
                if(IceUtilInternal.StringUtil.match(prop, prefix + suffix, false))
                {
                    valid = true;
                    break;
                }
            }

            if(!valid)
            {
                unknownProps.add(prop);
            }
        }

        if(unknownProps.size() != 0 && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
        {
            StringBuffer message = new StringBuffer("found unknown IceMX properties for `");
            message.append(prefix.substring(0, prefix.length() - 1));
            message.append("':");
            for(String p : unknownProps)
            {
                message.append("\n    ");
                message.append(p);
            }
            Ice.Util.getProcessLogger().warning(message.toString());
        }
    }

    static class MetricsMapFactory<T extends IceMX.Metrics>
    {
        public MetricsMapFactory(Runnable updater, Class<T> cl)
        {
            _updater = updater;
            _class = cl;
        }

        public void
        update()
        {
            assert(_updater != null);
            _updater.run();
        }

        public MetricsMap<T>
        create(String mapPrefix, Ice.Properties properties)
        {
            return new MetricsMap<T>(mapPrefix, _class, properties, _subMaps);
        }

        public <S extends IceMX.Metrics> void
        registerSubMap(String subMap, Class<S> cl, java.lang.reflect.Field field)
        {
            _subMaps.put(subMap, new MetricsMap.SubMapFactory<S>(cl, field));
        }

        final private Runnable _updater;
        final private Class<T> _class;
        final private java.util.Map<String, MetricsMap.SubMapFactory<?>> _subMaps =
            new java.util.HashMap<String, MetricsMap.SubMapFactory<?>>();
    }

    public MetricsAdminI(Ice.Properties properties, Ice.Logger logger)
    {
        _logger = logger;
        _properties = properties;
        updateViews();
    }

    public void updateViews()
    {
        java.util.Set<MetricsMapFactory<?> > updatedMaps = new java.util.HashSet<MetricsMapFactory<?> >();
        synchronized(this)
        {
            String viewsPrefix = "IceMX.Metrics.";
            java.util.Map<String, String> viewsProps = _properties.getPropertiesForPrefix(viewsPrefix);
            java.util.Map<String, MetricsViewI> views = new java.util.HashMap<String, MetricsViewI>();
            _disabledViews.clear();
            for(java.util.Map.Entry<String, String> e : viewsProps.entrySet())
            {
                String viewName = e.getKey().substring(viewsPrefix.length());
                int dotPos = viewName.indexOf('.');
                if(dotPos > 0)
                {
                    viewName = viewName.substring(0, dotPos);
                }

                if(views.containsKey(viewName) || _disabledViews.contains(viewName))
                {
                    continue; // View already configured.
                }

                validateProperties(viewsPrefix + viewName + ".", _properties);

                if(_properties.getPropertyAsIntWithDefault(viewsPrefix + viewName + ".Disabled", 0) > 0)
                {
                    _disabledViews.add(viewName);
                    continue; // The view is disabled
                }

                //
                // Create the view or update it.
                //
                MetricsViewI v = _views.get(viewName);
                if(v == null)
                {
                    v = new MetricsViewI(viewName);
                }
                views.put(viewName, v);

                for(java.util.Map.Entry<String, MetricsMapFactory<?>> f : _factories.entrySet())
                {
                    if(v.addOrUpdateMap(_properties, f.getKey(), f.getValue(), _logger))
                    {
                        updatedMaps.add(f.getValue());
                    }
                }
            }
            java.util.Map<String, MetricsViewI> tmp = _views;
            _views = views;
            views = tmp;

            //
            // Go through removed views to collect maps to update.
            //
            for(java.util.Map.Entry<String, MetricsViewI> v : views.entrySet())
            {
                if(!_views.containsKey(v.getKey()))
                {
                    for(String n : v.getValue().getMaps())
                    {
                        updatedMaps.add(_factories.get(n));
                    }
                }
            }
        }

        //
        // Call the updaters to update the maps.
        //
        for(MetricsMapFactory<?> f : updatedMaps)
        {
            f.update();
        }
    }

    @Override
    synchronized public String[]
    getMetricsViewNames(Ice.StringSeqHolder holder, Ice.Current current)
    {
        holder.value = _disabledViews.toArray(new String[_disabledViews.size()]);
        return _views.keySet().toArray(new String[_views.size()]);
    }

    @Override
    public void
    enableMetricsView(String name, Ice.Current current)
        throws IceMX.UnknownMetricsView
    {
        synchronized(this)
        {
            getMetricsView(name); // Throws if unknown view.
            _properties.setProperty("IceMX.Metrics." + name + ".Disabled", "0");
        }
        updateViews();
    }

    @Override
    public void
    disableMetricsView(String name, Ice.Current current)
        throws IceMX.UnknownMetricsView
    {
        synchronized(this)
        {
            getMetricsView(name); // Throws if unknown view.
            _properties.setProperty("IceMX.Metrics." + name + ".Disabled", "1");
        }
        updateViews();
    }

    @Override
    synchronized public java.util.Map<String, IceMX.Metrics[]>
    getMetricsView(String viewName, Ice.LongHolder holder, Ice.Current current)
        throws IceMX.UnknownMetricsView
    {
        MetricsViewI view = getMetricsView(viewName);
        holder.value = IceInternal.Time.currentMonotonicTimeMillis();
        if(view != null)
        {
            return view.getMetrics();
        }
        return new java.util.HashMap<String, IceMX.Metrics[]>();
    }

    @Override
    synchronized public IceMX.MetricsFailures[]
    getMapMetricsFailures(String viewName, String mapName, Ice.Current current)
        throws IceMX.UnknownMetricsView
    {
        MetricsViewI view = getMetricsView(viewName);
        if(view != null)
        {
            return view.getFailures(mapName);
        }
        return new IceMX.MetricsFailures[0];
    }

    @Override
    synchronized public IceMX.MetricsFailures
    getMetricsFailures(String viewName, String mapName, String id, Ice.Current current)
        throws IceMX.UnknownMetricsView
    {
        MetricsViewI view = getMetricsView(viewName);
        if(view != null)
        {
            return view.getFailures(mapName, id);
        }
        return new IceMX.MetricsFailures();
    }

    public <T extends IceMX.Metrics> void
    registerMap(String map, Class<T> cl, Runnable updater)
    {
        boolean updated;
        MetricsMapFactory<T> factory;
        synchronized(this)
        {
            factory = new MetricsMapFactory<T>(updater, cl);
            _factories.put(map, factory);
            updated = addOrUpdateMap(map, factory);
        }
        if(updated)
        {
            factory.update();
        }
    }

    synchronized public <S extends IceMX.Metrics> void
    registerSubMap(String map, String subMap, Class<S> cl, java.lang.reflect.Field field)
    {
        boolean updated;
        MetricsMapFactory<?> factory;
        synchronized(this)
        {
            factory = _factories.get(map);
            if(factory == null)
            {
                return;
            }
            factory.registerSubMap(subMap, cl, field);
            removeMap(map);
            updated = addOrUpdateMap(map, factory);
        }
        if(updated)
        {
            factory.update();
        }
    }

    public void
    unregisterMap(String mapName)
    {
        boolean updated;
        MetricsMapFactory<?> factory;
        synchronized(this)
        {
            factory = _factories.remove(mapName);
            if(factory == null)
            {
                return;
            }
            updated = removeMap(mapName);
        }
        if(updated)
        {
            factory.update();
        }
    }

    public <T extends IceMX.Metrics> java.util.List<MetricsMap<T>>
    getMaps(String mapName, Class<T> cl)
    {
        java.util.List<MetricsMap<T>> maps = new java.util.ArrayList<MetricsMap<T>>();
        for(MetricsViewI v : _views.values())
        {
            MetricsMap<T> map = v.getMap(mapName, cl);
            if(map != null)
            {
                maps.add(map);
            }
        }
        return maps;
    }

    public Ice.Logger
    getLogger()
    {
        return _logger;
    }

    @Override
    public void
    updated(java.util.Map<String, String> props)
    {
        for(java.util.Map.Entry<String, String> e : props.entrySet())
        {
            if(e.getKey().indexOf("IceMX.") == 0)
            {
                // Udpate the metrics views using the new configuration.
                try
                {
                    updateViews();
                }
                catch(Exception ex)
                {
                    _logger.warning("unexpected exception while updating metrics view configuration:\n" +
                                    ex.toString());
                }
                return;
            }
        }
    }

    private MetricsViewI
    getMetricsView(String name)
        throws IceMX.UnknownMetricsView
    {
        MetricsViewI view = _views.get(name);
        if(view == null)
        {
            if(!_disabledViews.contains(name))
            {
                throw new IceMX.UnknownMetricsView();
            }
            return null;
        }
        return view;
    }

    private boolean
    addOrUpdateMap(String mapName, MetricsMapFactory<?> factory)
    {
        boolean updated = false;
        for(MetricsViewI v : _views.values())
        {
            updated |= v.addOrUpdateMap(_properties, mapName, factory, _logger);
        }
        return updated;
    }

    private boolean
    removeMap(String mapName)
    {
        boolean updated = false;
        for(MetricsViewI v : _views.values())
        {
            updated |= v.removeMap(mapName);
        }
        return updated;
    }

    private Ice.Properties _properties;
    final private Ice.Logger _logger;
    final private java.util.Map<String, MetricsMapFactory<?>> _factories =
        new java.util.HashMap<String, MetricsMapFactory<?>>();

    private java.util.Map<String, MetricsViewI> _views = new java.util.HashMap<String, MetricsViewI>();
    private java.util.Set<String> _disabledViews = new java.util.HashSet<String>();
}
