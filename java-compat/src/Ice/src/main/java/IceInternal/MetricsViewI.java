// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class MetricsViewI
{
    MetricsViewI(String name)
    {
        _name = name;
    }

    public boolean
    addOrUpdateMap(Ice.Properties properties, String mapName, MetricsAdminI.MetricsMapFactory<?> factory,
                   Ice.Logger logger)
    {
        //
        // Add maps to views configured with the given map.
        //
        String viewPrefix = "IceMX.Metrics." + _name + ".";
        String mapsPrefix = viewPrefix + "Map.";
        java.util.Map<String, String> mapsProps = properties.getPropertiesForPrefix(mapsPrefix);

        String mapPrefix;
        java.util.Map<String, String> mapProps = new java.util.HashMap<String, String>();
        if(!mapsProps.isEmpty())
        {
            mapPrefix = mapsPrefix + mapName + ".";
            mapProps = properties.getPropertiesForPrefix(mapPrefix);
            if(mapProps.isEmpty())
            {
                // This map isn't configured for this view.
                return _maps.remove(mapName) != null;
            }
        }
        else
        {
            mapPrefix = viewPrefix;
            mapProps = properties.getPropertiesForPrefix(mapPrefix);
        }

        if(properties.getPropertyAsInt(mapPrefix + "Disabled") > 0)
        {
            // This map is disabled for this view.
            return _maps.remove(mapName) != null;
        }

        MetricsMap<?> m = _maps.get(mapName);
        if(m != null && m.getProperties().equals(mapProps))
        {
            return false; // The map configuration didn't change, no need to re-create.
        }

        try
        {
            _maps.put(mapName, factory.create(mapPrefix, properties));
        }
        catch(Exception ex)
        {
            logger.warning("unexpected exception while creating metrics map:\n" + ex);
            _maps.remove(mapName);
        }
        return true;
    }

    public boolean
    removeMap(String mapName)
    {
        return _maps.remove(mapName) != null;
    }

    public java.util.Map<String, IceMX.Metrics[]>
    getMetrics()
    {
        java.util.Map<String, IceMX.Metrics[]> metrics = new java.util.HashMap<String, IceMX.Metrics[]>();
        for(java.util.Map.Entry<String, MetricsMap<?>> e : _maps.entrySet())
        {
            metrics.put(e.getKey(), e.getValue().getMetrics());
        }
        return metrics;
    }

    public IceMX.MetricsFailures[]
    getFailures(String mapName)
    {
        MetricsMap<?> m = _maps.get(mapName);
        if(m != null)
        {
            return m.getFailures();
        }
        return null;
    }

    public IceMX.MetricsFailures
    getFailures(String mapName, String id)
    {
        MetricsMap<?> m = _maps.get(mapName);
        if(m != null)
        {
            return m.getFailures(id);
        }
        return null;
    }

    public java.util.Collection<String>
    getMaps()
    {
        return _maps.keySet();
    }

    @SuppressWarnings("unchecked")
    public <T extends IceMX.Metrics> MetricsMap<T>
    getMap(String mapName, Class<T> cl)
    {
        return (MetricsMap<T>)_maps.get(mapName);
    }

    final private String _name;
    final private java.util.Map<String, MetricsMap<?>> _maps = new java.util.HashMap<String, MetricsMap<?>>();
}
