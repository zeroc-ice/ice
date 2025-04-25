// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.IceMX.Metrics;
import com.zeroc.IceMX.MetricsFailures;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

class MetricsViewI {
    MetricsViewI(String name) {
        _name = name;
    }

    public boolean addOrUpdateMap(
            Properties properties,
            String mapName,
            MetricsAdminI.MetricsMapFactory<?> factory,
            Logger logger) {
        //
        // Add maps to views configured with the given map.
        //
        String viewPrefix = "IceMX.Metrics." + _name + ".";
        String mapsPrefix = viewPrefix + "Map.";
        Map<String, String> mapsProps = properties.getPropertiesForPrefix(mapsPrefix);

        String mapPrefix;
        Map<String, String> mapProps = new HashMap<>();
        if (!mapsProps.isEmpty()) {
            mapPrefix = mapsPrefix + mapName + ".";
            mapProps = properties.getPropertiesForPrefix(mapPrefix);
            if (mapProps.isEmpty()) {
                // This map isn't configured for this view.
                return _maps.remove(mapName) != null;
            }
        } else {
            mapPrefix = viewPrefix;
            mapProps = properties.getPropertiesForPrefix(mapPrefix);
        }

        if (properties.getPropertyAsInt(mapPrefix + "Disabled") > 0) {
            // This map is disabled for this view.
            return _maps.remove(mapName) != null;
        }

        MetricsMap<?> m = _maps.get(mapName);
        if (m != null && m.getProperties().equals(mapProps)) {
            return false; // The map configuration didn't change, no need to re-create.
        }

        try {
            _maps.put(mapName, factory.create(mapPrefix, properties));
        } catch (Exception ex) {
            logger.warning("unexpected exception while creating metrics map:\n" + ex);
            _maps.remove(mapName);
        }
        return true;
    }

    public boolean removeMap(String mapName) {
        return _maps.remove(mapName) != null;
    }

    public Map<String, Metrics[]> getMetrics() {
        Map<String, Metrics[]> metrics = new HashMap<>();
        for (Map.Entry<String, MetricsMap<?>> e : _maps.entrySet()) {
            metrics.put(e.getKey(), e.getValue().getMetrics());
        }
        return metrics;
    }

    public MetricsFailures[] getFailures(String mapName) {
        MetricsMap<?> m = _maps.get(mapName);
        if (m != null) {
            return m.getFailures();
        }
        return null;
    }

    public MetricsFailures getFailures(String mapName, String id) {
        MetricsMap<?> m = _maps.get(mapName);
        if (m != null) {
            return m.getFailures(id);
        }
        return null;
    }

    public Collection<String> getMaps() {
        return _maps.keySet();
    }

    @SuppressWarnings("unchecked")
    public <T extends Metrics> MetricsMap<T> getMap(String mapName, Class<T> cl) {
        return (MetricsMap<T>) _maps.get(mapName);
    }

    private final String _name;
    private final Map<String, MetricsMap<?>> _maps = new HashMap<>();
}
