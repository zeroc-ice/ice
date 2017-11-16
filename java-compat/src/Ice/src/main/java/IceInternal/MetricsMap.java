// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class MetricsMap<T extends IceMX.Metrics>
{
    public class Entry
    {
        Entry(T obj)
        {
            _object = obj;
        }

        public void
        failed(String exceptionName)
        {
            synchronized(MetricsMap.this)
            {
                ++_object.failures;
                if(_failures == null)
                {
                    _failures = new java.util.HashMap<String, Integer>();
                }
                Integer count = _failures.get(exceptionName);
                _failures.put(exceptionName, Integer.valueOf(count == null ? 1 : count + 1));
            }
        }

        @SuppressWarnings("unchecked")
        public <S extends IceMX.Metrics> MetricsMap<S>.Entry
        getMatching(String mapName, IceMX.MetricsHelper<S> helper, Class<S> cl)
        {
            SubMap<S> m;
            synchronized(MetricsMap.this)
            {
                m = _subMaps != null ? (SubMap<S>)_subMaps.get(mapName) : null;
                if(m == null)
                {
                    m = createSubMap(mapName, cl);
                    if(m == null)
                    {
                        return null;
                    }
                    if(_subMaps == null)
                    {
                        _subMaps = new java.util.HashMap<String, SubMap<?>>();
                    }
                    _subMaps.put(mapName, m);
                }
            }
            return m.getMatching(helper);
        }

        public void
        detach(long lifetime)
        {
            synchronized(MetricsMap.this)
            {
                _object.totalLifetime += lifetime;
                if(--_object.current == 0)
                {
                    detached(this);
                }
            }
        }

        public void
        execute(IceMX.Observer.MetricsUpdate<T> func)
        {
            synchronized(MetricsMap.this)
            {
                func.update(_object);
            }
        }

        public MetricsMap<?>
        getMap()
        {
            return MetricsMap.this;
        }

        private IceMX.MetricsFailures
        getFailures()
        {
            if(_failures == null)
            {
                return null;
            }
            IceMX.MetricsFailures f = new IceMX.MetricsFailures();
            f.id = _object.id;
            f.failures = new java.util.HashMap<String, Integer>(_failures);
            return f;
        }

        private void
        attach(IceMX.MetricsHelper<T> helper)
        {
            ++_object.total;
            ++_object.current;
            helper.initMetrics(_object);
        }

        private boolean
        isDetached()
        {
            return _object.current == 0;
        }

        @Override
        @SuppressWarnings("unchecked")
        public IceMX.Metrics
        clone()
        {
            T metrics = (T)_object.clone();
            if(_subMaps != null)
            {
                for(SubMap<?> s : _subMaps.values())
                {
                    s.addSubMapToMetrics(metrics);
                }
            }
            return metrics;
        }

        private T _object;
        private java.util.Map<String, Integer> _failures;
        private java.util.Map<String, SubMap<?>> _subMaps;
    }

    static class SubMap<S extends IceMX.Metrics>
    {
        public
        SubMap(MetricsMap<S> map, java.lang.reflect.Field field)
        {
            _map = map;
            _field = field;
        }

        public MetricsMap<S>.Entry
        getMatching(IceMX.MetricsHelper<S> helper)
        {
            return _map.getMatching(helper, null);
        }

        public void
        addSubMapToMetrics(IceMX.Metrics metrics)
        {
            try
            {
                _field.set(metrics, _map.getMetrics());
            }
            catch(Exception ex)
            {
                assert(false);
            }
        }

        final private MetricsMap<S> _map;
        final private java.lang.reflect.Field _field;
    }

    static class SubMapCloneFactory<S extends IceMX.Metrics>
    {
        public SubMapCloneFactory(MetricsMap<S> map, java.lang.reflect.Field field)
        {
            _map = map;
            _field = field;
        }

        public SubMap<S>
        create()
        {
            return new SubMap<S>(new MetricsMap<S>(_map), _field);
        }

        final private MetricsMap<S> _map;
        final private java.lang.reflect.Field _field;
    }

    static class SubMapFactory<S extends IceMX.Metrics>
    {
        SubMapFactory(Class<S> cl, java.lang.reflect.Field field)
        {
            _class = cl;
            _field = field;
        }

        SubMapCloneFactory<S>
        createCloneFactory(String subMapPrefix, Ice.Properties properties)
        {
            return new SubMapCloneFactory<S>(new MetricsMap<S>(subMapPrefix, _class, properties, null), _field);
        }

        final private Class<S> _class;
        final private java.lang.reflect.Field _field;
    }

    MetricsMap(String mapPrefix, Class<T> cl, Ice.Properties props, java.util.Map<String, SubMapFactory<?>> subMaps)
    {
        MetricsAdminI.validateProperties(mapPrefix, props);
        _properties = props.getPropertiesForPrefix(mapPrefix);

        _retain = props.getPropertyAsIntWithDefault(mapPrefix + "RetainDetached", 10);
        _accept = parseRule(props, mapPrefix + "Accept");
        _reject = parseRule(props, mapPrefix + "Reject");
        _groupByAttributes = new java.util.ArrayList<String>();
        _groupBySeparators = new java.util.ArrayList<String>();
        _class = cl;

        String groupBy = props.getPropertyWithDefault(mapPrefix + "GroupBy", "id");
        if(!groupBy.isEmpty())
        {
            String v = "";
            boolean attribute = Character.isLetter(groupBy.charAt(0)) || Character.isDigit(groupBy.charAt(0));
            if(!attribute)
            {
                _groupByAttributes.add("");
            }

            for(char p : groupBy.toCharArray())
            {
                boolean isAlphaNum = Character.isLetter(p) || Character.isDigit(p) || p == '.';
                if(attribute && !isAlphaNum)
                {
                    _groupByAttributes.add(v);
                    v = "" + p;
                    attribute = false;
                }
                else if(!attribute && isAlphaNum)
                {
                    _groupBySeparators.add(v);
                    v = "" + p;
                    attribute = true;
                }
                else
                {
                    v += p;
                }
            }

            if(attribute)
            {
                _groupByAttributes.add(v);
            }
            else
            {
                _groupBySeparators.add(v);
            }
        }

        if(subMaps != null && !subMaps.isEmpty())
        {
            _subMaps = new java.util.HashMap<String, SubMapCloneFactory<?>>();

            java.util.List<String> subMapNames = new java.util.ArrayList<String>();
            for(java.util.Map.Entry<String, SubMapFactory<?>> e : subMaps.entrySet())
            {
                subMapNames.add(e.getKey());
                String subMapsPrefix = mapPrefix + "Map.";
                String subMapPrefix = subMapsPrefix + e.getKey() + '.';
                if(props.getPropertiesForPrefix(subMapPrefix).isEmpty())
                {
                    if(props.getPropertiesForPrefix(subMapsPrefix).isEmpty())
                    {
                        subMapPrefix = mapPrefix;
                    }
                    else
                    {
                        continue; // This sub-map isn't configured.
                    }
                }

                _subMaps.put(e.getKey(), e.getValue().createCloneFactory(subMapPrefix, props));
            }
        }
        else
        {
            _subMaps = null;
        }
    }

    MetricsMap(MetricsMap<T> map)
    {
        _properties = map._properties;
        _groupByAttributes = map._groupByAttributes;
        _groupBySeparators = map._groupBySeparators;
        _retain = map._retain;
        _accept = map._accept;
        _reject = map._reject;
        _class = map._class;
        _subMaps = map._subMaps;
    }

    java.util.Map<String, String>
    getProperties()
    {
        return _properties;
    }

    synchronized IceMX.Metrics[]
    getMetrics()
    {
        IceMX.Metrics[] metrics = new IceMX.Metrics[_objects.size()];
        int i = 0;
        for(Entry e : _objects.values())
        {
            metrics[i++] = e.clone();
        }
        return metrics;
    }

    synchronized IceMX.MetricsFailures[]
    getFailures()
    {
        java.util.List<IceMX.MetricsFailures> failures = new java.util.ArrayList<IceMX.MetricsFailures>();
        for(Entry e : _objects.values())
        {
            IceMX.MetricsFailures f = e.getFailures();
            if(f != null)
            {
                failures.add(f);
            }
        }
        return failures.toArray(new IceMX.MetricsFailures[failures.size()]);
    }

    synchronized IceMX.MetricsFailures
    getFailures(String id)
    {
        Entry e = _objects.get(id);
        if(e != null)
        {
            return e.getFailures();
        }
        return null;
    }

    @SuppressWarnings("unchecked")
    public <S extends IceMX.Metrics> SubMap<S>
    createSubMap(String subMapName, Class<S> cl)
    {
        if(_subMaps == null)
        {
            return null;
        }
        SubMapCloneFactory<S> factory = (SubMapCloneFactory<S>)_subMaps.get(subMapName);
        if(factory != null)
        {
            return factory.create();
        }
        return null;
    }

    public Entry
    getMatching(IceMX.MetricsHelper<T> helper, Entry previous)
    {
        //
        // Check the accept and reject filters.
        //
        for(java.util.Map.Entry<String, java.util.regex.Pattern> e : _accept.entrySet())
        {
            if(!match(e.getKey(), e.getValue(), helper, false))
            {
                return null;
            }
        }

        for(java.util.Map.Entry<String, java.util.regex.Pattern> e : _reject.entrySet())
        {
            if(match(e.getKey(), e.getValue(), helper, true))
            {
                return null;
            }
        }

        //
        // Compute the key from the GroupBy property.
        //
        String key;
        try
        {
            if(_groupByAttributes.size() == 1)
            {
                key = helper.resolve(_groupByAttributes.get(0));
            }
            else
            {
                StringBuilder os = new StringBuilder();
                java.util.Iterator<String> q = _groupBySeparators.iterator();
                for(String p : _groupByAttributes)
                {
                    os.append(helper.resolve(p));
                    if(q.hasNext())
                    {
                        os.append(q.next());
                    }
                }
                key = os.toString();
            }
        }
        catch(Exception ex)
        {
            return null;
        }

        //
        // Lookup the metrics object.
        //
        synchronized(this)
        {
            if(previous != null && previous._object.id.equals(key))
            {
                assert(_objects.get(key) == previous);
                return previous;
            }

            Entry e = _objects.get(key);
            if(e == null)
            {
                try
                {
                    T t = _class.getDeclaredConstructor().newInstance();
                    t.id = key;
                    e = new Entry(t);
                    _objects.put(key, e);
                }
                catch(Exception ex)
                {
                    assert(false);
                }
            }
            e.attach(helper);
            return e;
        }
    }

    private void
    detached(Entry entry)
    {
        if(_retain == 0)
        {
            return;
        }

        if(_detachedQueue == null)
        {
            _detachedQueue = new java.util.LinkedList<Entry>();
        }
        assert(_detachedQueue.size() <= _retain);

        // Compress the queue by removing entries which are no longer detached.
        java.util.Iterator<Entry> p = _detachedQueue.iterator();
        while(p.hasNext())
        {
            Entry e = p.next();
            if(e == entry || !e.isDetached())
            {
                p.remove();
            }
        }

        // If there's still no room, remove the oldest entry (at the front).
        if(_detachedQueue.size() == _retain)
        {
            _objects.remove(_detachedQueue.pollFirst()._object.id);
        }

        // Add the entry at the back of the queue.
        _detachedQueue.add(entry);
    }

    private java.util.Map<String, java.util.regex.Pattern>
    parseRule(Ice.Properties properties, String name)
    {
        java.util.Map<String, java.util.regex.Pattern> pats = new java.util.HashMap<String, java.util.regex.Pattern>();
        java.util.Map<String, String> rules = properties.getPropertiesForPrefix(name + '.');
        for(java.util.Map.Entry<String,String> e : rules.entrySet())
        {
            pats.put(e.getKey().substring(name.length() + 1), java.util.regex.Pattern.compile(e.getValue()));
        }
        return pats;
    }

    private boolean
    match(String attribute, java.util.regex.Pattern regex, IceMX.MetricsHelper<T> helper, boolean reject)
    {
        String value;
        try
        {
            value = helper.resolve(attribute);
        }
        catch(Exception ex)
        {
            return !reject;
        }
        return regex.matcher(value).matches();
    }

    final private java.util.Map<String, String> _properties;
    final private java.util.List<String> _groupByAttributes;
    final private java.util.List<String> _groupBySeparators;
    final private int _retain;
    final private java.util.Map<String, java.util.regex.Pattern> _accept;
    final private java.util.Map<String, java.util.regex.Pattern> _reject;
    final private Class<T> _class;

    final private java.util.Map<String, Entry> _objects = new java.util.HashMap<String, Entry>();
    final private java.util.Map<String, SubMapCloneFactory<?>> _subMaps;
    private java.util.Deque<Entry> _detachedQueue;
}
