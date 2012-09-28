// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Text;
    using System.Diagnostics;
    using System.Collections.Generic;
    using System.Text.RegularExpressions;

    internal interface IMetricsMap
    {
        IceMX.Metrics[] getMetrics();
        IceMX.MetricsFailures[] getFailures();
        IceMX.MetricsFailures getFailures(string id);
        Dictionary<string, string> getProperties();
    };

    interface ISubMap
    {
        void addSubMapToMetrics(IceMX.Metrics metrics);
    };

    interface ISubMapCloneFactory
    {
        ISubMap create();
    };

    interface ISubMapFactory
    {
        ISubMapCloneFactory createCloneFactory(string subMapPrefix, Ice.Properties properties);
    };

    internal interface IMetricsMapFactory
    {
        void registerSubMap<S>(string subMap, System.Reflection.FieldInfo field) where S : IceMX.Metrics, new();
        void update();
        IMetricsMap create(string mapPrefix, Ice.Properties properties);
    };
    
    internal class SubMap<S> : ISubMap where S : IceMX.Metrics, new()
    {
        internal SubMap(MetricsMap<S> map, System.Reflection.FieldInfo field)
        {
            _map = map;
            _field = field;
        }

        internal MetricsMap<S>.Entry getMatching(IceMX.MetricsHelper<S> helper)
        {
            return _map.getMatching(helper);
        }
        
        public void addSubMapToMetrics(IceMX.Metrics metrics)
        {
            try
            {
                _field.SetValue(metrics, _map.getMetrics());
            }
            catch(Exception)
            {
                Debug.Assert(false);
            }
        }
            
        readonly private MetricsMap<S> _map;
        readonly private System.Reflection.FieldInfo _field;
    };

    internal class SubMapCloneFactory<S> : ISubMapCloneFactory where S : IceMX.Metrics, new()
    {
        internal SubMapCloneFactory(MetricsMap<S> map, System.Reflection.FieldInfo field)
        {
            _map = map;
            _field = field;
        }
        
        public ISubMap create()
        {
            return new SubMap<S>(new MetricsMap<S>(_map), _field);
        }
        
        readonly private MetricsMap<S> _map;
        readonly private System.Reflection.FieldInfo _field;
    };

    class SubMapFactory<S> : ISubMapFactory where S : IceMX.Metrics, new()
    {
        internal SubMapFactory(System.Reflection.FieldInfo field)
        {
            _field = field;
        }
        
        public ISubMapCloneFactory createCloneFactory(string subMapPrefix, Ice.Properties properties)
        {
            return new SubMapCloneFactory<S>(new MetricsMap<S>(subMapPrefix, properties, null), _field);
        }

        readonly private System.Reflection.FieldInfo _field;
    };

    public class MetricsMap<T> : IMetricsMap where T : IceMX.Metrics, new()
    {
        private string[] mapSuffixes =
            {
                "GroupBy",
                "Accept.*",
                "Reject.*",
                "RetainDetached",
            };
    
        public class Entry : IComparable<Entry>
        {
            internal Entry(MetricsMap<T> map, T obj)
            {
                _map = map;
                _object = obj;
            }

            public void failed(string exceptionName)
            {
                lock(this)
                {
                    ++_object.failures;
                    int count;
                    if(_failures.TryGetValue(exceptionName, out count))
                    {
                        _failures[exceptionName] = count + 1;
                    }
                    else
                    {
                        _failures[exceptionName] = 1;
                    }
                }
            }

            public IceMX.MetricsFailures getFailures()
            {
                lock(this)
                {
                    if(_failures.Count == 0)
                    {
                        return null;
                    }
                    IceMX.MetricsFailures f = new IceMX.MetricsFailures();
                    f.id = _object.id;
                    f.failures = new Dictionary<string, int>(_failures);
                    return f;
                }
            }

            internal MetricsMap<S>.Entry getMatching<S>(string mapName, IceMX.MetricsHelper<S> helper)
                where S : IceMX.Metrics, new()
            {
                ISubMap m;
                lock(this)
                {
                    if(_subMaps == null || !_subMaps.TryGetValue(mapName, out m))
                    {
                        m = _map.createSubMap(mapName);
                        if(m == null)
                        {
                            return null;
                        }
                        if(_subMaps == null)
                        {
                            _subMaps = new Dictionary<string, ISubMap>();
                        }
                        _subMaps.Add(mapName, m);
                    }
                }
                return ((SubMap<S>)m).getMatching(helper);
            }

            public void attach(IceMX.MetricsHelper<T> helper)
            {
                lock(this)
                {
                    ++_object.total;
                    ++_object.current;
                    helper.initMetrics(_object);
                }
            }

            public void detach(long lifetime)
            {
                lock(this)
                {
                    _object.totalLifetime += lifetime;
                    if(--_object.current > 0)
                    {
                        return;
                    }
                }
                _map.detached(this);
            }

            public bool isDetached()
            {
                lock(this)
                {
                    return _object.current == 0;
                }
            }
            
            public void execute(IceMX.Observer<T>.MetricsUpdate func)
            {
                lock(this)
                {
                    func(_object);
                }
            }

            public IceMX.Metrics clone()
            {
                lock(this)
                {
                    T metrics = (T)_object.Clone();
                    if(_subMaps != null)
                    {
                        foreach(ISubMap s in _subMaps.Values)
                        {
                            s.addSubMapToMetrics(metrics);
                        }
                    }
                    return metrics;
                }
            }

            internal string getId()
            {
                return _object.id;
            }

            public int CompareTo(Entry e)
            {
                return _object.id.CompareTo(e._object.id);
            }

            private MetricsMap<T> _map;
            private T _object;
            private Dictionary<string, int> _failures = new Dictionary<string, int>();
            private Dictionary<string, ISubMap> _subMaps;
        };

        internal MetricsMap(string mapPrefix, Ice.Properties props, Dictionary<string, ISubMapFactory> subMaps)
        {
            _properties = props.getPropertiesForPrefix(mapPrefix);
        
            _retain = props.getPropertyAsIntWithDefault(mapPrefix + "RetainDetached", 10);
            _accept = parseRule(props, mapPrefix + "Accept");
            _reject = parseRule(props, mapPrefix + "Reject");
            _groupByAttributes = new List<string>();
            _groupBySeparators = new List<string>();

            string groupBy = props.getPropertyWithDefault(mapPrefix + "GroupBy", "id");
            if(groupBy.Length > 0)
            {
                string v = "";
                bool attribute = Char.IsLetter(groupBy[0]) || Char.IsDigit(groupBy[0]);
                if(!attribute)
                {
                    _groupByAttributes.Add("");
                }
            
                foreach(char p in groupBy)
                {
                    bool isAlphaNum = Char.IsLetter(p) || Char.IsDigit(p) || p == '.';
                    if(attribute && !isAlphaNum)
                    {
                        _groupByAttributes.Add(v);
                        v = "" + p;
                        attribute = false;
                    }
                    else if(!attribute && isAlphaNum)
                    {
                        _groupBySeparators.Add(v);
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
                    _groupByAttributes.Add(v);
                }
                else
                {
                    _groupBySeparators.Add(v);
                }
            }

            if(subMaps != null && subMaps.Count > 0)
            {
                _subMaps = new Dictionary<string, ISubMapCloneFactory>();

                List<string> subMapNames = new List<string>();
                foreach(KeyValuePair<string, ISubMapFactory> e in subMaps)
                {
                    subMapNames.Add(e.Key);
                    string subMapsPrefix = mapPrefix + "Map.";
                    string subMapPrefix = subMapsPrefix + e.Key + '.';
                    if(props.getPropertiesForPrefix(subMapPrefix).Count == 0)
                    {
                        if(props.getPropertiesForPrefix(subMapsPrefix).Count == 0)
                        {
                            subMapPrefix = mapPrefix;
                        }
                        else
                        {
                            continue; // This sub-map isn't configured.
                        }
                    }

                    _subMaps.Add(e.Key, e.Value.createCloneFactory(subMapPrefix, props));
                }
                validateProperties(mapPrefix, props, subMapNames);
            }
            else
            {
                _subMaps = null;
            }
        }
    
        internal MetricsMap(MetricsMap<T> map)
        {
            _properties = map._properties;
            _groupByAttributes = map._groupByAttributes;
            _groupBySeparators = map._groupBySeparators;
            _retain = map._retain;
            _accept = map._accept;
            _reject = map._reject;
            _subMaps = map._subMaps;
        }

        private void validateProperties(string prefix, Ice.Properties props, ICollection<string> subMaps)
        {
            if(subMaps.Count == 0)
            {
                MetricsAdminI.validateProperties(prefix, props, mapSuffixes);
                return;
            }

            List<string> suffixes = new List<string>(mapSuffixes);
            foreach(string s in subMaps)
            {
                string suffix = "Map." + s + ".";
                MetricsAdminI.validateProperties(prefix + suffix, props, mapSuffixes);
                suffixes.Add(suffix + '*');
            }
            MetricsAdminI.validateProperties(prefix, props, suffixes.ToArray());
        }
        
        public Dictionary<string, string> getProperties()
        {
            return _properties;
        }

        public IceMX.Metrics[] getMetrics()
        {
            lock(this)
            {
                IceMX.Metrics[] metrics = new IceMX.Metrics[_objects.Count];
                int i = 0;
                foreach(Entry e in _objects.Values)
                {
                    metrics[i++] = e.clone();
                }
                return metrics;
            }
        }
    
        public IceMX.MetricsFailures[] getFailures()
        {
            lock(this)
            {
                List<IceMX.MetricsFailures> failures = new List<IceMX.MetricsFailures>();
                foreach(Entry e in _objects.Values)
                {
                    IceMX.MetricsFailures f = e.getFailures();
                    if(f != null)
                    {
                        failures.Add(f);
                    }
                }
                return failures.ToArray();
            }
        }
    
        public IceMX.MetricsFailures getFailures(string id)
        {
            lock(this)
            {
                Entry e;
                if(_objects.TryGetValue(id, out e))
                {
                    return e.getFailures();
                }
                return null;
            }
        }

        ISubMap createSubMap(string subMapName)
        {
            if(_subMaps == null)
            {
                return null;
            }
            ISubMapCloneFactory factory;
            if(_subMaps.TryGetValue(subMapName, out factory))
            {
                return factory.create();
            }
            return null;
        }

        public Entry getMatching(IceMX.MetricsHelper<T> helper)
        {
            //
            // Check the accept and reject filters.
            //
            foreach(KeyValuePair<string, Regex> e in _accept)
            {
                if(!match(e.Key, e.Value, helper, false))
                {
                    System.Console.Out.WriteLine("No accept match");
                    return null;
                }
            }
        
            foreach(KeyValuePair<string, Regex> e in _reject)
            {
                if(match(e.Key, e.Value, helper, false))
                {
                    System.Console.Out.WriteLine("No reject match");
                    return null;
                }
            }

            //
            // Compute the key from the GroupBy property.
            //
            string key;
            try
            {
                if(_groupByAttributes.Count == 1)
                {
                    key = helper.resolve(_groupByAttributes[0]);
                }
                else
                {
                    StringBuilder os = new StringBuilder();
                    IEnumerator<string> q = _groupBySeparators.GetEnumerator();
                    foreach(string p in _groupByAttributes)
                    {
                        os.Append(helper.resolve(p));
                        if(q.MoveNext())
                        {
                            os.Append(q.Current);
                        }
                    }
                    key = os.ToString();
                }
            }
            catch(Exception)
            {
                return null;
            }

            //
            // Lookup the metrics object.
            // 
            lock(this)
            {
                Entry e;
                if(!_objects.TryGetValue(key, out e))
                {
                    try
                    {
                        T t = new T();
                        t.id = key;
                        e = new Entry(this, t);
                        _objects.Add(key, e);
                    }
                    catch(Exception)
                    {
                        Debug.Assert(false);
                    }
                }
                return e;
            }
        }
    
        private void detached(Entry entry)
        {
            if(_retain == 0)
            {
                return;
            }

            lock(this)
            {
                if(_detachedQueue == null)
                {
                    _detachedQueue = new LinkedList<Entry>();
                }
                Debug.Assert(_detachedQueue.Count <= _retain);
            
                // Compress the queue by removing entries which are no longer detached.
                LinkedListNode<Entry> p = _detachedQueue.First;
                while(p != null)
                {
                    LinkedListNode<Entry> next = p.Next;
                    if(p.Value == entry || !p.Value.isDetached())
                    {
                        _detachedQueue.Remove(p);
                    }
                    p = next;
                }

                // If there's still no room, remove the oldest entry (at the front).
                if(_detachedQueue.Count == _retain)
                {
                    _objects.Remove(_detachedQueue.First.Value.getId());
                    _detachedQueue.RemoveFirst();
                }
            
                // Add the entry at the back of the queue.
                _detachedQueue.AddLast(entry);
            }
        }
    
        private Dictionary<string, Regex> parseRule(Ice.Properties properties, string name)
        {
            Dictionary<string, Regex> pats = new Dictionary<string, Regex>();
            Dictionary<string, string> rules = properties.getPropertiesForPrefix(name + '.');
            foreach(KeyValuePair<string, string> e in rules)
            {
                pats.Add(e.Key.Substring(name.Length + 1), new Regex(e.Value));
            }
            return pats;
        }

        private bool match(string attribute, Regex regex, IceMX.MetricsHelper<T> helper, bool reject)
        {
            string value;
            try
            {
                value = helper.resolve(attribute);
            }
            catch(Exception)
            {
                return !reject;
            }
            return regex.IsMatch(value);
        }

        readonly private Dictionary<string, string> _properties;
        readonly private List<string> _groupByAttributes;
        readonly private List<string> _groupBySeparators;
        readonly private int _retain;
        readonly private Dictionary<string, Regex> _accept;
        readonly private Dictionary<string, Regex> _reject;

        readonly private Dictionary<string, Entry> _objects = new Dictionary<string, Entry>();
        readonly private Dictionary<string, ISubMapCloneFactory> _subMaps;
        private LinkedList<Entry> _detachedQueue;
    };

    internal class MetricsViewI
    {
        internal MetricsViewI(string name)
        {
            _name = name;
        }

        internal bool addOrUpdateMap(Ice.Properties properties, string mapName, IMetricsMapFactory factory, 
                                   Ice.Logger logger)
        {
            //
            // Add maps to views configured with the given map.
            //
            string viewPrefix = "IceMX.Metrics." + _name + ".";
            string mapsPrefix = viewPrefix + "Map.";
            Dictionary<string, string> mapsProps = properties.getPropertiesForPrefix(mapsPrefix);
            
            string mapPrefix;
            Dictionary<string, string> mapProps = new Dictionary<string, string>();
            if(mapsProps.Count > 0)
            {
                mapPrefix = mapsPrefix + mapName + ".";
                mapProps = properties.getPropertiesForPrefix(mapPrefix);
                if(mapProps.Count == 0)
                {
                    // This map isn't configured for this view.
                    _maps.Remove(mapName);
                    return true;
                }
            }
            else
            {
                mapPrefix = viewPrefix;
                mapProps = properties.getPropertiesForPrefix(mapPrefix);
            }
            
            IMetricsMap m;
            if(_maps.TryGetValue(mapName, out m) && m.getProperties().Equals(mapProps))
            {
                return false; // The map configuration didn't change, no need to re-create.
            }
            
            try
            {
                _maps[mapName] = factory.create(mapPrefix, properties);
            }
            catch(Exception ex)
            {
                logger.warning("unexpected exception while creating metrics map:\n" + ex);
                _maps.Remove(mapName);
            }
            return true;
        }
        
        internal bool removeMap(string mapName)
        {
            bool removed = _maps.ContainsKey(mapName);
            _maps.Remove(mapName);
            return removed;
        }

        internal Dictionary<string, IceMX.Metrics[]> getMetrics()
        {
            Dictionary<string, IceMX.Metrics[]> metrics = new Dictionary<string, IceMX.Metrics[]>();
            foreach(KeyValuePair<string, IMetricsMap> e in _maps)
            {
                IceMX.Metrics[] m = e.Value.getMetrics();
                if(m != null)
                {
                    metrics.Add(e.Key, m);
                }
            }
            return metrics;
        }

        internal IceMX.MetricsFailures[] getFailures(string mapName)
        {
            IMetricsMap m;
            if(_maps.TryGetValue(mapName, out m))
            {
                return m.getFailures();
            }
            return null;
        }

        internal IceMX.MetricsFailures getFailures(string mapName, string id)
        {
            IMetricsMap m;
            if(_maps.TryGetValue(mapName, out m))
            {
                return m.getFailures(id);
            }
            return null;
        }

        internal ICollection<string> getMaps()
        {
            return _maps.Keys;
        }
        
        internal MetricsMap<T> getMap<T>(string mapName) where T : IceMX.Metrics, new()
        {
            IMetricsMap m;
            if(_maps.TryGetValue(mapName, out m))
            {
                return (MetricsMap<T>)m;
            }
            return null;
        }
        
        readonly private string _name;
        readonly private Dictionary<string, IMetricsMap> _maps = new Dictionary<string, IMetricsMap>();
    };

    public class MetricsAdminI : IceMX.MetricsAdminDisp_, Ice.PropertiesAdminUpdateCallback
    {
        readonly static private string[] viewSuffixes =
            {
                "Disabled",
                "GroupBy",
                "Accept.*",
                "Reject.*",
                "RetainDetached",
                "Map.*",
            };

        public static void validateProperties(string prefix, Ice.Properties properties, string[] suffixes)
        {
            Dictionary<string, string> props = properties.getPropertiesForPrefix(prefix);
            List<string> unknownProps = new List<string>();
            foreach(string prop in props.Keys)
            {
                bool valid = false;
                foreach(string suffix in suffixes)
                {
                    if(IceUtilInternal.StringUtil.match(prop, prefix + suffix, false))
                    {
                        valid = true;
                        break;
                    }
                }

                if(!valid)
                {
                    unknownProps.Add(prop);
                }
            }
        
            if(unknownProps.Count != 0 && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
            {
                StringBuilder message = new StringBuilder("found unknown IceMX properties for `");
                message.Append(prefix.Substring(0, prefix.Length - 1));
                message.Append("':");
                foreach(string p in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(p);
                }
                Ice.Util.getProcessLogger().warning(message.ToString());
            }
        }

        class MetricsMapFactory<T> : IMetricsMapFactory where T : IceMX.Metrics, new()
        {
            public MetricsMapFactory(System.Action updater)
            {
                _updater = updater;
            }

            public void update()
            {
                Debug.Assert(_updater != null);
                _updater();
            }

            public IMetricsMap create(string mapPrefix, Ice.Properties properties)
            {
                return new MetricsMap<T>(mapPrefix, properties, _subMaps);
            }

            public void registerSubMap<S>(string subMap, System.Reflection.FieldInfo field) 
                where S : IceMX.Metrics, new()
            {
                _subMaps.Add(subMap, new SubMapFactory<S>(field));
            }

            readonly private System.Action _updater;
            readonly private Dictionary<string, ISubMapFactory> _subMaps = new Dictionary<string, ISubMapFactory>();
        };

        public MetricsAdminI(Ice.Properties properties, Ice.Logger logger)
        {
            _logger = logger;
            _properties = properties;
            updateViews();
        }

        public void updateViews()
        {
            HashSet<IMetricsMapFactory> updatedMaps = new HashSet<IMetricsMapFactory>();
            lock(this)
            {
                string viewsPrefix = "IceMX.Metrics.";
                Dictionary<string, string> viewsProps = _properties.getPropertiesForPrefix(viewsPrefix);
                Dictionary<string, MetricsViewI> views = new Dictionary<string, MetricsViewI>();
                foreach(KeyValuePair<string, string> e in viewsProps)
                {
                    string viewName = e.Key.Substring(viewsPrefix.Length);
                    int dotPos = viewName.IndexOf('.');
                    if(dotPos > 0)
                    {
                        viewName = viewName.Substring(0, dotPos);
                    }
                
                    if(views.ContainsKey(viewName))
                    {
                        continue; // View already configured.
                    }
                
                    validateProperties(viewsPrefix + viewName + ".", _properties, viewSuffixes);
                
                    if(_properties.getPropertyAsIntWithDefault(viewsPrefix + viewName + ".Disabled", 0) > 0)
                    {
                        continue; // The view is disabled
                    }
                
                    //
                    // Create the view or update it.
                    //
                    MetricsViewI v;
                    if(!_views.TryGetValue(viewName, out v))
                    {
                        v = new MetricsViewI(viewName);
                    }
                    views.Add(viewName, v);

                    foreach(KeyValuePair<string, IMetricsMapFactory> f in _factories)
                    {
                        if(v.addOrUpdateMap(_properties, f.Key, f.Value, _logger))
                        {
                            updatedMaps.Add(f.Value);
                        }
                    }
                }
                Dictionary<string, MetricsViewI> tmp = _views;
                _views = views;
                views = tmp;
        
                //
                // Go through removed views to collect maps to update.
                //
                foreach(KeyValuePair<string, MetricsViewI> v in views)
                {
                    if(!_views.ContainsKey(v.Key))
                    {
                        foreach(string n in v.Value.getMaps())
                        {
                            updatedMaps.Add(_factories[n]);
                        }
                    }
                }
            }
        
            //
            // Call the updaters to update the maps.
            //
            foreach(IMetricsMapFactory f in updatedMaps)
            {
                f.update();
            }
        }

        override public string[] getMetricsViewNames(Ice.Current current)
        {
            lock(this)
            {
                return new List<String>(_views.Keys).ToArray();
            }
        }

        override public Dictionary<string, IceMX.Metrics[]> getMetricsView(string viewName, Ice.Current current)
        {
            lock(this)
            {
                MetricsViewI view;
                if(!_views.TryGetValue(viewName, out view))
                {
                    throw new IceMX.UnknownMetricsView();
                }
                return view.getMetrics();
            }
        }

        override public IceMX.MetricsFailures[] getMapMetricsFailures(string viewName, string mapName, Ice.Current c)
        {
            lock(this)
            {
                MetricsViewI view;
                if(!_views.TryGetValue(viewName, out view))
                {
                    throw new IceMX.UnknownMetricsView();
                }
                return view.getFailures(mapName);
            }
        }

        override public IceMX.MetricsFailures getMetricsFailures(string viewName, string mapName, string id, 
                                                                 Ice.Current c)
        {
            lock(this)
            {
                MetricsViewI view;
                if(!_views.TryGetValue(viewName, out view))
                {
                    throw new IceMX.UnknownMetricsView();
                }
                return view.getFailures(mapName, id);
            }
        }

        public void registerMap<T>(string map, System.Action updater)
            where T : IceMX.Metrics, new()
        {
            bool updated;
            MetricsMapFactory<T> factory;
            lock(this)
            {
                factory = new MetricsMapFactory<T>(updater);
                _factories.Add(map, factory);
                updated = addOrUpdateMap(map, factory);
            }
            if(updated)
            {
                factory.update();
            }
        }

        public void registerSubMap<S>(string map, string subMap, System.Reflection.FieldInfo field) 
            where S : IceMX.Metrics, new()
        {
            bool updated;
            IMetricsMapFactory factory;
            lock(this)
            {
                if(!_factories.TryGetValue(map, out factory))
                {
                    return;
                }
                factory.registerSubMap<S>(subMap, field);
                removeMap(map);
                updated = addOrUpdateMap(map, factory);
            }
            if(updated)
            {
                factory.update();
            }
        }

        public void unregisterMap(string mapName)
        {
            bool updated;
            IMetricsMapFactory factory;
            lock(this)
            {
                if(!_factories.TryGetValue(mapName, out factory))
                {
                    return;
                }
                _factories.Remove(mapName);
                updated = removeMap(mapName);
            }
            if(updated)
            {
                factory.update();
            }
        }

        public List<MetricsMap<T>> getMaps<T>(string mapName) where T : IceMX.Metrics, new()
        {
            List<MetricsMap<T>> maps = new List<MetricsMap<T>>();
            foreach(MetricsViewI v in _views.Values)
            {
                MetricsMap<T> map = v.getMap<T>(mapName);
                if(map != null)
                {
                    maps.Add(map);
                }
            }
            return maps;
        }

        public Ice.Logger getLogger()
        {
            return _logger;
        }

        public void setProperties(Ice.Properties properties)
        {
            _properties = properties;
        }
    
        public void updated(Dictionary<string, string> props)
        { 
            foreach(KeyValuePair<string, string> e in props)
            {
                if(e.Key.IndexOf("IceMX.") == 0)
                {
                    // Udpate the metrics views using the new configuration.
                    try
                    {
                        updateViews();
                    }
                    catch(Exception ex)
                    {
                        _logger.warning("unexpected exception while updating metrics view configuration:\n" + 
                                        ex.ToString());
                    }
                    return;
                }
            }
        }
    
        private bool addOrUpdateMap(string mapName, IMetricsMapFactory factory)
        {
            bool updated = false;
            foreach(MetricsViewI v in _views.Values)
            {
                updated |= v.addOrUpdateMap(_properties, mapName, factory, _logger);
            }
            return updated;
        }

        private bool removeMap(string mapName)
        {
            bool updated = false;
            foreach(MetricsViewI v in _views.Values)
            {
                updated |= v.removeMap(mapName);
            }
            return updated;
        }

        private Ice.Properties _properties;
        readonly private Ice.Logger _logger;
        readonly private Dictionary<string, IMetricsMapFactory> _factories = 
            new Dictionary<string, IMetricsMapFactory>();
        private Dictionary<string, MetricsViewI> _views = new Dictionary<string, MetricsViewI>();
    }
}