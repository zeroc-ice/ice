// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.IceMX.Metrics;
import com.zeroc.Ice.IceMX.MetricsFailures;
import com.zeroc.Ice.IceMX.MetricsHelper;
import com.zeroc.Ice.IceMX.Observer;
import com.zeroc.Ice.MetricsMap.SubMap;

import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * @hidden Public because it's used by IceMX.
 */
public class MetricsMap<T extends Metrics> {
    public class Entry {
        Entry(T obj) {
            _object = obj;
        }

        public void failed(String exceptionName) {
            synchronized (MetricsMap.this) {
                ++_object.failures;
                if (_failures == null) {
                    _failures = new HashMap<>();
                }
                Integer count = _failures.get(exceptionName);
                _failures.put(exceptionName, Integer.valueOf(count == null ? 1 : count + 1));
            }
        }

        @SuppressWarnings("unchecked")
        public <S extends Metrics> MetricsMap<S>.Entry getMatching(
                String mapName, MetricsHelper<S> helper, Class<S> cl) {
            SubMap<S> m;
            synchronized (MetricsMap.this) {
                m = _subMaps != null ? (SubMap<S>) _subMaps.get(mapName) : null;
                if (m == null) {
                    m = createSubMap(mapName, cl);
                    if (m == null) {
                        return null;
                    }
                    if (_subMaps == null) {
                        _subMaps = new HashMap<>();
                    }
                    _subMaps.put(mapName, m);
                }
            }
            return m.getMatching(helper);
        }

        public void detach(long lifetime) {
            synchronized (MetricsMap.this) {
                _object.totalLifetime += lifetime;
                if (--_object.current == 0) {
                    detached(this);
                }
            }
        }

        public void execute(Observer.MetricsUpdate<T> func) {
            synchronized (MetricsMap.this) {
                func.update(_object);
            }
        }

        public MetricsMap<?> getMap() {
            return MetricsMap.this;
        }

        private MetricsFailures getFailures() {
            if (_failures == null) {
                return null;
            }
            var f = new MetricsFailures();
            f.id = _object.id;
            f.failures = new HashMap<>(_failures);
            return f;
        }

        private void attach(MetricsHelper<T> helper) {
            ++_object.total;
            ++_object.current;
            helper.initMetrics(_object);
        }

        private boolean isDetached() {
            return _object.current == 0;
        }

        @Override
        @SuppressWarnings("unchecked")
        public Metrics clone() {
            T metrics = (T) _object.clone();
            if (_subMaps != null) {
                for (SubMap<?> s : _subMaps.values()) {
                    s.addSubMapToMetrics(metrics);
                }
            }
            return metrics;
        }

        private T _object;
        private Map<String, Integer> _failures;
        private Map<String, SubMap<?>> _subMaps;
    }

    static class SubMap<S extends Metrics> {
        public SubMap(MetricsMap<S> map, java.lang.reflect.Field field) {
            _map = map;
            _field = field;
        }

        public MetricsMap<S>.Entry getMatching(MetricsHelper<S> helper) {
            return _map.getMatching(helper, null);
        }

        public void addSubMapToMetrics(Metrics metrics) {
            try {
                _field.set(metrics, _map.getMetrics());
            } catch (Exception ex) {
                assert false;
            }
        }

        private final MetricsMap<S> _map;
        private final java.lang.reflect.Field _field;
    }

    static class SubMapCloneFactory<S extends Metrics> {
        public SubMapCloneFactory(MetricsMap<S> map, java.lang.reflect.Field field) {
            _map = map;
            _field = field;
        }

        public SubMap<S> create() {
            return new SubMap<S>(new MetricsMap<S>(_map), _field);
        }

        private final MetricsMap<S> _map;
        private final java.lang.reflect.Field _field;
    }

    static class SubMapFactory<S extends Metrics> {
        SubMapFactory(Class<S> cl, java.lang.reflect.Field field) {
            _class = cl;
            _field = field;
        }

        SubMapCloneFactory<S> createCloneFactory(String subMapPrefix, Properties properties) {
            return new SubMapCloneFactory<S>(
                    new MetricsMap<S>(subMapPrefix, _class, properties, null), _field);
        }

        private final Class<S> _class;
        private final java.lang.reflect.Field _field;
    }

    MetricsMap(
            String mapPrefix,
            Class<T> cl,
            Properties props,
            Map<String, SubMapFactory<?>> subMaps) {
        MetricsAdminI.validateProperties(mapPrefix, props);
        _properties = props.getPropertiesForPrefix(mapPrefix);

        _retain = props.getPropertyAsIntWithDefault(mapPrefix + "RetainDetached", 10);
        _accept = parseRule(props, mapPrefix + "Accept");
        _reject = parseRule(props, mapPrefix + "Reject");
        _groupByAttributes = new ArrayList<>();
        _groupBySeparators = new ArrayList<>();
        _class = cl;

        String groupBy = props.getPropertyWithDefault(mapPrefix + "GroupBy", "id");
        if (!groupBy.isEmpty()) {
            String v = "";
            boolean attribute =
                    Character.isLetter(groupBy.charAt(0)) || Character.isDigit(groupBy.charAt(0));
            if (!attribute) {
                _groupByAttributes.add("");
            }

            for (char p : groupBy.toCharArray()) {
                boolean isAlphaNum = Character.isLetter(p) || Character.isDigit(p) || p == '.';
                if (attribute && !isAlphaNum) {
                    _groupByAttributes.add(v);
                    v = "" + p;
                    attribute = false;
                } else if (!attribute && isAlphaNum) {
                    _groupBySeparators.add(v);
                    v = "" + p;
                    attribute = true;
                } else {
                    v += p;
                }
            }

            if (attribute) {
                _groupByAttributes.add(v);
            } else {
                _groupBySeparators.add(v);
            }
        }

        if (subMaps != null && !subMaps.isEmpty()) {
            _subMaps = new HashMap<>();

            List<String> subMapNames = new ArrayList<>();
            for (Map.Entry<String, SubMapFactory<?>> e : subMaps.entrySet()) {
                subMapNames.add(e.getKey());
                String subMapsPrefix = mapPrefix + "Map.";
                String subMapPrefix = subMapsPrefix + e.getKey() + '.';
                if (props.getPropertiesForPrefix(subMapPrefix).isEmpty()) {
                    if (props.getPropertiesForPrefix(subMapsPrefix).isEmpty()) {
                        subMapPrefix = mapPrefix;
                    } else {
                        continue; // This sub-map isn't configured.
                    }
                }

                _subMaps.put(e.getKey(), e.getValue().createCloneFactory(subMapPrefix, props));
            }
        } else {
            _subMaps = null;
        }
    }

    MetricsMap(MetricsMap<T> map) {
        _properties = map._properties;
        _groupByAttributes = map._groupByAttributes;
        _groupBySeparators = map._groupBySeparators;
        _retain = map._retain;
        _accept = map._accept;
        _reject = map._reject;
        _class = map._class;
        _subMaps = map._subMaps;
    }

    Map<String, String> getProperties() {
        return _properties;
    }

    synchronized Metrics[] getMetrics() {
        Metrics[] metrics = new Metrics[_objects.size()];
        int i = 0;
        for (Entry e : _objects.values()) {
            metrics[i++] = e.clone();
        }
        return metrics;
    }

    synchronized MetricsFailures[] getFailures() {
        List<MetricsFailures> failures = new ArrayList<>();
        for (Entry e : _objects.values()) {
            MetricsFailures f = e.getFailures();
            if (f != null) {
                failures.add(f);
            }
        }
        return failures.toArray(new MetricsFailures[failures.size()]);
    }

    synchronized MetricsFailures getFailures(String id) {
        Entry e = _objects.get(id);
        if (e != null) {
            return e.getFailures();
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    public <S extends Metrics> SubMap<S> createSubMap(String subMapName, Class<S> cl) {
        if (_subMaps == null) {
            return null;
        }
        SubMapCloneFactory<S> factory = (SubMapCloneFactory<S>) _subMaps.get(subMapName);
        if (factory != null) {
            return factory.create();
        }
        return null;
    }

    public Entry getMatching(MetricsHelper<T> helper, Entry previous) {
        //
        // Check the accept and reject filters.
        //
        for (Map.Entry<String, Pattern> e : _accept.entrySet()) {
            if (!match(e.getKey(), e.getValue(), helper, false)) {
                return null;
            }
        }

        for (Map.Entry<String, Pattern> e : _reject.entrySet()) {
            if (match(e.getKey(), e.getValue(), helper, true)) {
                return null;
            }
        }

        //
        // Compute the key from the GroupBy property.
        //
        String key;
        try {
            if (_groupByAttributes.size() == 1) {
                key = helper.resolve(_groupByAttributes.get(0));
            } else {
                StringBuilder os = new StringBuilder();
                Iterator<String> q = _groupBySeparators.iterator();
                for (String p : _groupByAttributes) {
                    os.append(helper.resolve(p));
                    if (q.hasNext()) {
                        os.append(q.next());
                    }
                }
                key = os.toString();
            }
        } catch (Exception ex) {
            return null;
        }

        //
        // Lookup the metrics object.
        //
        synchronized (this) {
            if (previous != null && previous._object.id.equals(key)) {
                assert (_objects.get(key) == previous);
                return previous;
            }

            Entry e = _objects.get(key);
            if (e == null) {
                try {
                    T t = _class.getDeclaredConstructor().newInstance();
                    t.id = key;
                    e = new Entry(t);
                    _objects.put(key, e);
                } catch (Exception ex) {
                    assert false;
                }
            }
            e.attach(helper);
            return e;
        }
    }

    private void detached(Entry entry) {
        if (_retain == 0) {
            return;
        }

        if (_detachedQueue == null) {
            _detachedQueue = new LinkedList<>();
        }
        assert (_detachedQueue.size() <= _retain);

        // Compress the queue by removing entries which are no longer detached.
        Iterator<Entry> p = _detachedQueue.iterator();
        while (p.hasNext()) {
            Entry e = p.next();
            if (e == entry || !e.isDetached()) {
                p.remove();
            }
        }

        // If there's still no room, remove the oldest entry (at the front).
        if (_detachedQueue.size() == _retain) {
            _objects.remove(_detachedQueue.pollFirst()._object.id);
        }

        // Add the entry at the back of the queue.
        _detachedQueue.add(entry);
    }

    private Map<String, Pattern> parseRule(
            Properties properties, String name) {
        Map<String, Pattern> pats = new HashMap<>();
        Map<String, String> rules = properties.getPropertiesForPrefix(name + '.');
        for (Map.Entry<String, String> e : rules.entrySet()) {
            pats.put(
                    e.getKey().substring(name.length() + 1),
                    Pattern.compile(e.getValue()));
        }
        return pats;
    }

    private boolean match(
            String attribute,
            Pattern regex,
            MetricsHelper<T> helper,
            boolean reject) {
        String value;
        try {
            value = helper.resolve(attribute);
        } catch (Exception ex) {
            return !reject;
        }
        return regex.matcher(value).matches();
    }

    private final Map<String, String> _properties;
    private final List<String> _groupByAttributes;
    private final List<String> _groupBySeparators;
    private final int _retain;
    private final Map<String, Pattern> _accept;
    private final Map<String, Pattern> _reject;
    private final Class<T> _class;

    private final Map<String, Entry> _objects = new HashMap<>();
    private final Map<String, SubMapCloneFactory<?>> _subMaps;
    private Deque<Entry> _detachedQueue;
}
