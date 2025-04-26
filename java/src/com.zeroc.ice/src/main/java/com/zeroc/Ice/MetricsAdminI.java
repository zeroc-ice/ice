// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.IceMX.Metrics;
import com.zeroc.IceMX.MetricsAdmin;
import com.zeroc.IceMX.MetricsFailures;
import com.zeroc.IceMX.UnknownMetricsView;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.function.Consumer;

/**
 * @hidden Public because it's used by IceMX.
 */
public class MetricsAdminI
    implements MetricsAdmin, Consumer<Map<String, String>> {
    private static final String[] suffixes = {
        "Disabled", "GroupBy", "Accept.*", "Reject.*", "RetainDetached", "Map.*",
    };

    static void validateProperties(String prefix, Properties properties) {
        Map<String, String> props = properties.getPropertiesForPrefix(prefix);
        List<String> unknownProps = new ArrayList<>();
        for (String prop : props.keySet()) {
            boolean valid = false;
            for (String suffix : suffixes) {
                if (StringUtil.match(prop, prefix + suffix, false)) {
                    valid = true;
                    break;
                }
            }

            if (!valid) {
                unknownProps.add(prop);
            }
        }

        if (unknownProps.size() > 0) {
            throw new PropertyException(
                "found unknown IceMX properties for:"
                    + ": '"
                    + prefix
                    + "'\n    "
                    + String.join("\n    ", unknownProps));
        }
    }

    static class MetricsMapFactory<T extends Metrics> {
        public MetricsMapFactory(Runnable updater, Class<T> cl) {
            _updater = updater;
            _class = cl;
        }

        public void update() {
            assert (_updater != null);
            _updater.run();
        }

        public MetricsMap<T> create(String mapPrefix, Properties properties) {
            return new MetricsMap<T>(mapPrefix, _class, properties, _subMaps);
        }

        public <S extends Metrics> void registerSubMap(
                String subMap, Class<S> cl, java.lang.reflect.Field field) {
            _subMaps.put(subMap, new MetricsMap.SubMapFactory<S>(cl, field));
        }

        private final Runnable _updater;
        private final Class<T> _class;
        private final Map<String, MetricsMap.SubMapFactory<?>> _subMaps =
            new HashMap<>();
    }

    public MetricsAdminI(Properties properties, Logger logger) {
        _logger = logger;
        _properties = properties;
        updateViews();
    }

    public void updateViews() {
        Set<MetricsMapFactory<?>> updatedMaps = new HashSet<>();
        synchronized (this) {
            String viewsPrefix = "IceMX.Metrics.";
            Map<String, String> viewsProps =
                _properties.getPropertiesForPrefix(viewsPrefix);
            Map<String, MetricsViewI> views = new HashMap<>();
            _disabledViews.clear();
            for (Map.Entry<String, String> e : viewsProps.entrySet()) {
                String viewName = e.getKey().substring(viewsPrefix.length());
                int dotPos = viewName.indexOf('.');
                if (dotPos > 0) {
                    viewName = viewName.substring(0, dotPos);
                }

                if (views.containsKey(viewName) || _disabledViews.contains(viewName)) {
                    continue; // View already configured.
                }

                validateProperties(viewsPrefix + viewName + '.', _properties);

                if (_properties.getPropertyAsIntWithDefault(viewsPrefix + viewName + ".Disabled", 0)
                    > 0) {
                    _disabledViews.add(viewName);
                    continue; // The view is disabled
                }

                //
                // Create the view or update it.
                //
                MetricsViewI v = _views.get(viewName);
                if (v == null) {
                    v = new MetricsViewI(viewName);
                }
                views.put(viewName, v);

                for (Map.Entry<String, MetricsMapFactory<?>> f : _factories.entrySet()) {
                    if (v.addOrUpdateMap(_properties, f.getKey(), f.getValue(), _logger)) {
                        updatedMaps.add(f.getValue());
                    }
                }
            }
            Map<String, MetricsViewI> tmp = _views;
            _views = views;
            views = tmp;

            //
            // Go through removed views to collect maps to update.
            //
            for (Map.Entry<String, MetricsViewI> v : views.entrySet()) {
                if (!_views.containsKey(v.getKey())) {
                    for (String n : v.getValue().getMaps()) {
                        updatedMaps.add(_factories.get(n));
                    }
                }
            }
        }

        //
        // Call the updaters to update the maps.
        //
        for (MetricsMapFactory<?> f : updatedMaps) {
            f.update();
        }
    }

    @Override
    public synchronized MetricsAdmin.GetMetricsViewNamesResult getMetricsViewNames(
            Current current) {
        MetricsAdmin.GetMetricsViewNamesResult r = new MetricsAdmin.GetMetricsViewNamesResult();
        r.disabledViews = _disabledViews.toArray(new String[_disabledViews.size()]);
        r.returnValue = _views.keySet().toArray(new String[_views.size()]);
        return r;
    }

    @Override
    public void enableMetricsView(String name, Current current) throws UnknownMetricsView {
        synchronized (this) {
            getMetricsView(name); // Throws if unknown view.
            _properties.setProperty("IceMX.Metrics." + name + ".Disabled", "0");
        }
        updateViews();
    }

    @Override
    public void disableMetricsView(String name, Current current) throws UnknownMetricsView {
        synchronized (this) {
            getMetricsView(name); // Throws if unknown view.
            _properties.setProperty("IceMX.Metrics." + name + ".Disabled", "1");
        }
        updateViews();
    }

    @Override
    public synchronized MetricsAdmin.GetMetricsViewResult getMetricsView(
            String viewName, Current current) throws UnknownMetricsView {
        MetricsAdmin.GetMetricsViewResult r = new MetricsAdmin.GetMetricsViewResult();
        MetricsViewI view = getMetricsView(viewName);
        r.timestamp = Time.currentMonotonicTimeMillis();
        if (view != null) {
            r.returnValue = view.getMetrics();
        } else {
            r.returnValue = new HashMap<>();
        }
        return r;
    }

    @Override
    public synchronized MetricsFailures[] getMapMetricsFailures(
            String viewName, String mapName, Current current) throws UnknownMetricsView {
        MetricsViewI view = getMetricsView(viewName);
        if (view != null) {
            return view.getFailures(mapName);
        }
        return new MetricsFailures[0];
    }

    @Override
    public synchronized MetricsFailures getMetricsFailures(
            String viewName, String mapName, String id, Current current) throws UnknownMetricsView {
        MetricsViewI view = getMetricsView(viewName);
        if (view != null) {
            return view.getFailures(mapName, id);
        }
        return new MetricsFailures();
    }

    public <T extends Metrics> void registerMap(String map, Class<T> cl, Runnable updater) {
        boolean updated;
        MetricsMapFactory<T> factory;
        synchronized (this) {
            factory = new MetricsMapFactory<T>(updater, cl);
            _factories.put(map, factory);
            updated = addOrUpdateMap(map, factory);
        }
        if (updated) {
            factory.update();
        }
    }

    public synchronized <S extends Metrics> void registerSubMap(
            String map, String subMap, Class<S> cl, java.lang.reflect.Field field) {
        boolean updated;
        MetricsMapFactory<?> factory;
        synchronized (this) {
            factory = _factories.get(map);
            if (factory == null) {
                return;
            }
            factory.registerSubMap(subMap, cl, field);
            removeMap(map);
            updated = addOrUpdateMap(map, factory);
        }
        if (updated) {
            factory.update();
        }
    }

    public void unregisterMap(String mapName) {
        boolean updated;
        MetricsMapFactory<?> factory;
        synchronized (this) {
            factory = _factories.remove(mapName);
            if (factory == null) {
                return;
            }
            updated = removeMap(mapName);
        }
        if (updated) {
            factory.update();
        }
    }

    public <T extends Metrics> List<MetricsMap<T>> getMaps(String mapName, Class<T> cl) {
        List<MetricsMap<T>> maps = new ArrayList<>();
        for (MetricsViewI v : _views.values()) {
            MetricsMap<T> map = v.getMap(mapName, cl);
            if (map != null) {
                maps.add(map);
            }
        }
        return maps;
    }

    public Logger getLogger() {
        return _logger;
    }

    @Override
    public void accept(Map<String, String> props) {
        for (Map.Entry<String, String> e : props.entrySet()) {
            if (e.getKey().indexOf("IceMX.") == 0) {
                // Update the metrics views using the new configuration.
                try {
                    updateViews();
                } catch (Exception ex) {
                    _logger.warning(
                        "unexpected exception while updating metrics view configuration:\n"
                            + ex.toString());
                }
                return;
            }
        }
    }

    private MetricsViewI getMetricsView(String name) throws UnknownMetricsView {
        MetricsViewI view = _views.get(name);
        if (view == null) {
            if (!_disabledViews.contains(name)) {
                throw new UnknownMetricsView();
            }
            return null;
        }
        return view;
    }

    private boolean addOrUpdateMap(String mapName, MetricsMapFactory<?> factory) {
        boolean updated = false;
        for (MetricsViewI v : _views.values()) {
            updated |= v.addOrUpdateMap(_properties, mapName, factory, _logger);
        }
        return updated;
    }

    private boolean removeMap(String mapName) {
        boolean updated = false;
        for (MetricsViewI v : _views.values()) {
            updated |= v.removeMap(mapName);
        }
        return updated;
    }

    private final Properties _properties;
    private final Logger _logger;
    private final Map<String, MetricsMapFactory<?>> _factories =
        new HashMap<>();

    private Map<String, MetricsViewI> _views = new HashMap<>();
    private final Set<String> _disabledViews = new HashSet<>();
}
