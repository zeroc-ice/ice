//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Text.RegularExpressions;

using ZeroC.IceMX;

namespace ZeroC.Ice
{
    internal interface IMetricsMap
    {
        Metrics[] GetMetrics();
        MetricsFailures[] GetFailures();
        MetricsFailures? GetFailures(string id);
        Dictionary<string, string> GetProperties();
    }

    internal interface ISubMap
    {
        void AddSubMapToMetrics(Metrics metrics);
    }

    internal interface ISubMapCloneFactory
    {
        ISubMap Create();
    }

    internal interface ISubMapFactory
    {
        ISubMapCloneFactory CreateCloneFactory(string subMapPrefix, Communicator communicator);
    }

    internal interface IMetricsMapFactory
    {
        void RegisterSubMap<S>(string subMap, System.Reflection.FieldInfo field) where S : Metrics, new();
        void Update();
        IMetricsMap Create(string mapPrefix, Communicator communicator);
    }

    internal class SubMap<S> : ISubMap where S : Metrics, new()
    {
        internal SubMap(MetricsMap<S> map, System.Reflection.FieldInfo field)
        {
            _map = map;
            _field = field;
        }

        internal MetricsMap<S>.Entry? GetMatching(MetricsHelper<S> helper) => _map.GetMatching(helper, null);

        public void AddSubMapToMetrics(Metrics metrics)
        {
            try
            {
                _field.SetValue(metrics, _map.GetMetrics());
            }
            catch (Exception)
            {
                Debug.Assert(false);
            }
        }

        private readonly MetricsMap<S> _map;
        private readonly System.Reflection.FieldInfo _field;
    }

    internal class SubMapCloneFactory<S> : ISubMapCloneFactory where S : Metrics, new()
    {
        internal SubMapCloneFactory(MetricsMap<S> map, System.Reflection.FieldInfo field)
        {
            _map = map;
            _field = field;
        }

        public ISubMap Create() => new SubMap<S>(new MetricsMap<S>(_map), _field);

        private readonly MetricsMap<S> _map;
        private readonly System.Reflection.FieldInfo _field;
    }

    internal class SubMapFactory<S> : ISubMapFactory where S : Metrics, new()
    {
        internal SubMapFactory(System.Reflection.FieldInfo field) => _field = field;

        public ISubMapCloneFactory CreateCloneFactory(string subMapPrefix, Communicator communicator) =>
            new SubMapCloneFactory<S>(new MetricsMap<S>(subMapPrefix, communicator, null), _field);

        private readonly System.Reflection.FieldInfo _field;
    }

    public class MetricsMap<T> : IMetricsMap where T : Metrics, new()
    {
        public class Entry
        {
            internal Entry(MetricsMap<T> map, T obj)
            {
                _map = map;
                _object = obj;
            }

            public void Failed(string exceptionName)
            {
                lock (_map)
                {
                    ++_object.Failures;
                    if (_failures == null)
                    {
                        _failures = new Dictionary<string, int>();
                    }
                    if (_failures.TryGetValue(exceptionName, out int count))
                    {
                        _failures[exceptionName] = count + 1;
                    }
                    else
                    {
                        _failures[exceptionName] = 1;
                    }
                }
            }

            internal MetricsMap<S>.Entry? GetMatching<S>(string mapName, MetricsHelper<S> helper)
                where S : Metrics, new()
            {
                ISubMap? m;
                lock (_map)
                {
                    if (_subMaps == null || !_subMaps.TryGetValue(mapName, out m))
                    {
                        m = _map.CreateSubMap(mapName);
                        if (m == null)
                        {
                            return null;
                        }
                        if (_subMaps == null)
                        {
                            _subMaps = new Dictionary<string, ISubMap>();
                        }
                        _subMaps.Add(mapName, m);
                    }
                }
                return ((SubMap<S>)m).GetMatching(helper);
            }

            public void Detach(long lifetime)
            {
                lock (_map)
                {
                    _object.TotalLifetime += lifetime;
                    if (--_object.Current == 0)
                    {
                        _map.Detached(this);
                    }
                }
            }

            public void Execute(Observer<T>.MetricsUpdate func)
            {
                lock (_map)
                {
                    func(_object);
                }
            }

            public MetricsMap<T> GetMap() => _map;

            internal MetricsFailures? GetFailures()
            {
                if (_failures == null)
                {
                    return null;
                }
                var f = new MetricsFailures();
                f.Id = _object.Id;
                f.Failures = new Dictionary<string, int>(_failures);
                return f;
            }

            internal void Attach(MetricsHelper<T> helper)
            {
                ++_object.Total;
                ++_object.Current;
                helper.InitMetrics(_object);
            }

            internal bool IsDetached() => _object.Current == 0;

            internal Metrics Clone()
            {
                var metrics = (T)_object.Clone();
                if (_subMaps != null)
                {
                    foreach (ISubMap s in _subMaps.Values)
                    {
                        s.AddSubMapToMetrics(metrics);
                    }
                }
                return metrics;
            }

            internal string GetId() => _object.Id;

            private readonly MetricsMap<T> _map;
            private readonly T _object;
            private Dictionary<string, int>? _failures;
            private Dictionary<string, ISubMap>? _subMaps;
        }

        internal MetricsMap(string mapPrefix, Communicator communicator, Dictionary<string, ISubMapFactory>? subMaps)
        {
            MetricsAdminI.ValidateProperties(mapPrefix, communicator);
            _properties = communicator.GetProperties(forPrefix: mapPrefix);

            _retain = communicator.GetPropertyAsInt($"{mapPrefix}RetainDetached") ?? 10;
            _accept = ParseRule(communicator, $"{mapPrefix}Accept");
            _reject = ParseRule(communicator, $"{mapPrefix}Reject");
            _groupByAttributes = new List<string>();
            _groupBySeparators = new List<string>();

            string groupBy = communicator.GetProperty($"{mapPrefix}GroupBy") ?? "id";
            if (groupBy.Length > 0)
            {
                string v = "";
                bool attribute = char.IsLetter(groupBy[0]) || char.IsDigit(groupBy[0]);
                if (!attribute)
                {
                    _groupByAttributes.Add("");
                }

                foreach (char p in groupBy)
                {
                    bool isAlphaNum = char.IsLetter(p) || char.IsDigit(p) || p == '.';
                    if (attribute && !isAlphaNum)
                    {
                        _groupByAttributes.Add(v);
                        v = "" + p;
                        attribute = false;
                    }
                    else if (!attribute && isAlphaNum)
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

                if (attribute)
                {
                    _groupByAttributes.Add(v);
                }
                else
                {
                    _groupBySeparators.Add(v);
                }
            }

            if (subMaps != null && subMaps.Count > 0)
            {
                _subMaps = new Dictionary<string, ISubMapCloneFactory>();

                var subMapNames = new List<string>();
                foreach (KeyValuePair<string, ISubMapFactory> e in subMaps)
                {
                    subMapNames.Add(e.Key);
                    string subAllMapsPrefix = mapPrefix + "Map.";
                    string subMapPrefix = subAllMapsPrefix + e.Key + '.';
                    if (communicator.GetProperties(forPrefix: subMapPrefix).Count == 0)
                    {
                        if (communicator.GetProperties(forPrefix: subAllMapsPrefix).Count == 0)
                        {
                            subMapPrefix = mapPrefix;
                        }
                        else
                        {
                            continue; // This sub-map isn't configured.
                        }
                    }

                    _subMaps.Add(e.Key, e.Value.CreateCloneFactory(subMapPrefix, communicator));
                }
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

        public Dictionary<string, string> GetProperties() => _properties;

        public Metrics[] GetMetrics()
        {
            lock (this)
            {
                var metrics = new Metrics[_objects.Count];
                int i = 0;
                foreach (Entry e in _objects.Values)
                {
                    metrics[i++] = e.Clone();
                }
                return metrics;
            }
        }

        public MetricsFailures[] GetFailures()
        {
            lock (this)
            {
                var failures = new List<MetricsFailures>();
                foreach (Entry e in _objects.Values)
                {
                    if (e.GetFailures() is MetricsFailures f)
                    {
                        failures.Add(f);
                    }
                }
                return failures.ToArray();
            }
        }

        public MetricsFailures? GetFailures(string id)
        {
            lock (this)
            {
                if (_objects.TryGetValue(id, out MetricsMap<T>.Entry? e))
                {
                    return e.GetFailures();
                }
                return null;
            }
        }

        internal ISubMap? CreateSubMap(string subMapName)
        {
            if (_subMaps == null)
            {
                return null;
            }
            if (_subMaps.TryGetValue(subMapName, out ISubMapCloneFactory? factory))
            {
                return factory.Create();
            }
            return null;
        }

        public Entry? GetMatching(MetricsHelper<T> helper, Entry? previous)
        {
            //
            // Check the accept and reject filters.
            //
            foreach (KeyValuePair<string, Regex> e in _accept)
            {
                if (!Match(e.Key, e.Value, helper, false))
                {
                    return null;
                }
            }

            foreach (KeyValuePair<string, Regex> e in _reject)
            {
                if (Match(e.Key, e.Value, helper, true))
                {
                    return null;
                }
            }

            //
            // Compute the key from the GroupBy property.
            //
            string key;
            try
            {
                if (_groupByAttributes.Count == 1)
                {
                    key = helper.Resolve(_groupByAttributes[0]);
                }
                else
                {
                    var os = new StringBuilder();
                    IEnumerator<string> q = _groupBySeparators.GetEnumerator();
                    foreach (string p in _groupByAttributes)
                    {
                        os.Append(helper.Resolve(p));
                        if (q.MoveNext())
                        {
                            os.Append(q.Current);
                        }
                    }
                    key = os.ToString();
                }
            }
            catch (Exception)
            {
                return null;
            }

            //
            // Lookup the metrics object.
            //
            lock (this)
            {
                if (previous != null && previous.GetId().Equals(key))
                {
                    Debug.Assert(_objects[key] == previous);
                    return previous;
                }

                if (!_objects.TryGetValue(key, out MetricsMap<T>.Entry? e))
                {
                    try
                    {
                        var t = new T();
                        t.Id = key;
                        e = new Entry(this, t);
                        _objects.Add(key, e);
                    }
                    catch (Exception)
                    {
                        Debug.Assert(false);
                    }
                }
                e.Attach(helper);
                return e;
            }
        }

        private void Detached(Entry entry)
        {
            if (_retain == 0)
            {
                return;
            }

            if (_detachedQueue == null)
            {
                _detachedQueue = new LinkedList<Entry>();
            }
            Debug.Assert(_detachedQueue.Count <= _retain);

            // Compress the queue by removing entries which are no longer detached.
            LinkedListNode<Entry>? p = _detachedQueue.First;
            while (p != null)
            {
                LinkedListNode<Entry>? next = p.Next;
                if (p.Value == entry || !p.Value.IsDetached())
                {
                    _detachedQueue.Remove(p);
                }
                p = next;
            }

            // If there's still no room, remove the oldest entry (at the front).
            if (_detachedQueue.Count == _retain)
            {
                _objects.Remove(_detachedQueue.First!.Value.GetId());
                _detachedQueue.RemoveFirst();
            }

            // Add the entry at the back of the queue.
            _detachedQueue.AddLast(entry);
        }

        private Dictionary<string, Regex> ParseRule(Communicator communicator, string name)
        {
            var pats = new Dictionary<string, Regex>();
            Dictionary<string, string> rules = communicator.GetProperties(forPrefix: $"{name}.");
            foreach (KeyValuePair<string, string> e in rules)
            {
                pats.Add(e.Key.Substring(name.Length + 1), new Regex(e.Value));
            }
            return pats;
        }

        private bool Match(string attribute, Regex regex, MetricsHelper<T> helper, bool reject)
        {
            string value;
            try
            {
                value = helper.Resolve(attribute);
            }
            catch (Exception)
            {
                return !reject;
            }
            return regex.IsMatch(value);
        }

        private readonly Dictionary<string, string> _properties;
        private readonly List<string> _groupByAttributes;
        private readonly List<string> _groupBySeparators;
        private readonly int _retain;
        private readonly Dictionary<string, Regex> _accept;
        private readonly Dictionary<string, Regex> _reject;

        private readonly Dictionary<string, Entry> _objects = new Dictionary<string, Entry>();
        private readonly Dictionary<string, ISubMapCloneFactory>? _subMaps;
        private LinkedList<Entry>? _detachedQueue;
    }

    internal class MetricsViewI
    {
        internal MetricsViewI(string name) => _name = name;

        internal bool AddOrUpdateMap(Communicator communicator, string mapName, IMetricsMapFactory factory,
                                     ILogger logger)
        {
            //
            // Add maps to views configured with the given map.
            //
            string viewPrefix = "IceMX.Metrics." + _name + ".";
            string allMapsPrefix = viewPrefix + "Map.";
            Dictionary<string, string> allMapsProps = communicator.GetProperties(forPrefix: allMapsPrefix);

            string mapPrefix;
            Dictionary<string, string> mapProps;
            if (allMapsProps.Count > 0)
            {
                mapPrefix = allMapsPrefix + mapName + ".";
                mapProps = communicator.GetProperties(forPrefix: mapPrefix);
                if (mapProps.Count == 0)
                {
                    // This map isn't configured for this view.
                    return _maps.Remove(mapName);
                }
            }
            else
            {
                mapPrefix = viewPrefix;
                mapProps = communicator.GetProperties(forPrefix: mapPrefix);
            }

            if (communicator.GetPropertyAsBool($"{mapPrefix}Disabled") ?? false)
            {
                // This map is disabled for this view.
                return _maps.Remove(mapName);
            }

            if (_maps.TryGetValue(mapName, out IMetricsMap? m) && m.GetProperties().DictionaryEqual(mapProps))
            {
                return false; // The map configuration didn't change, no need to re-create.
            }

            try
            {
                _maps[mapName] = factory.Create(mapPrefix, communicator);
            }
            catch (Exception ex)
            {
                logger.Warning("unexpected exception while creating metrics map:\n" + ex);
                _maps.Remove(mapName);
            }
            return true;
        }

        internal bool RemoveMap(string mapName) => _maps.Remove(mapName);

        internal Dictionary<string, Metrics[]> GetMetrics()
        {
            var metrics = new Dictionary<string, Metrics[]>();
            foreach (KeyValuePair<string, IMetricsMap> e in _maps)
            {
                Metrics[] m = e.Value.GetMetrics();
                if (m != null)
                {
                    metrics.Add(e.Key, m);
                }
            }
            return metrics;
        }

        internal MetricsFailures[] GetFailures(string mapName)
        {
            if (_maps.TryGetValue(mapName, out IMetricsMap? m))
            {
                return m.GetFailures();
            }
            return Array.Empty<MetricsFailures>();
        }

        internal MetricsFailures? GetFailures(string mapName, string id)
        {
            if (_maps.TryGetValue(mapName, out IMetricsMap? m))
            {
                return m.GetFailures(id);
            }
            return null;
        }

        internal ICollection<string> GetMaps() => _maps.Keys;

        internal MetricsMap<T>? GetMap<T>(string mapName) where T : Metrics, new()
        {
            if (_maps.TryGetValue(mapName, out IMetricsMap? m))
            {
                return (MetricsMap<T>)m;
            }
            return null;
        }

        private readonly string _name;
        private readonly Dictionary<string, IMetricsMap> _maps = new Dictionary<string, IMetricsMap>();
    }

    public class MetricsAdminI : IMetricsAdmin
    {
        private static readonly string[] _suffixes =
            {
                "Disabled",
                "GroupBy",
                "Accept.*",
                "Reject.*",
                "RetainDetached",
                "Map.*",
            };

        public static void ValidateProperties(string prefix, Communicator communicator)
        {
            Dictionary<string, string> props = communicator.GetProperties(forPrefix: prefix);
            var unknownProps = new List<string>();
            foreach (string prop in props.Keys)
            {
                bool valid = false;
                foreach (string suffix in _suffixes)
                {
                    if (StringUtil.Match(prop, prefix + suffix, false))
                    {
                        valid = true;
                        break;
                    }
                }

                if (!valid)
                {
                    unknownProps.Add(prop);
                }
            }

            if (unknownProps.Count != 0 && (communicator.GetPropertyAsBool("Ice.Warn.UnknownProperties") ?? true))
            {
                var message = new StringBuilder("found unknown IceMX properties for `");
                message.Append(prefix[0..^1]);
                message.Append("':");
                foreach (string p in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(p);
                }
                Runtime.Logger.Warning(message.ToString());
            }
        }

        private class MetricsMapFactory<T> : IMetricsMapFactory where T : Metrics, new()
        {
            public MetricsMapFactory(Action updater) => _updater = updater;

            public void Update()
            {
                Debug.Assert(_updater != null);
                _updater();
            }

            public IMetricsMap Create(string mapPrefix, Communicator communicator) =>
                new MetricsMap<T>(mapPrefix, communicator, _subMaps);

            public void RegisterSubMap<S>(string subMap, System.Reflection.FieldInfo field)
                where S : Metrics, new() => _subMaps.Add(subMap, new SubMapFactory<S>(field));

            private readonly Action _updater;
            private readonly Dictionary<string, ISubMapFactory> _subMaps = new Dictionary<string, ISubMapFactory>();
        }

        public MetricsAdminI(Communicator communicator, ILogger logger)
        {
            _logger = logger;
            _communicator = communicator;
            UpdateViews();
        }

        public void UpdateViews()
        {
            var updatedMaps = new HashSet<IMetricsMapFactory>();
            lock (this)
            {
                string viewsPrefix = "IceMX.Metrics.";
                Dictionary<string, string> viewsProps = _communicator.GetProperties(forPrefix: viewsPrefix);
                var views = new Dictionary<string, MetricsViewI>();
                _disabledViews.Clear();
                foreach (KeyValuePair<string, string> e in viewsProps)
                {
                    string viewName = e.Key.Substring(viewsPrefix.Length);
                    int dotPos = viewName.IndexOf('.');
                    if (dotPos > 0)
                    {
                        viewName = viewName.Substring(0, dotPos);
                    }

                    if (views.ContainsKey(viewName) || _disabledViews.Contains(viewName))
                    {
                        continue; // View already configured.
                    }

                    ValidateProperties($"{viewsPrefix}{viewName}.", _communicator);

                    if (_communicator.GetPropertyAsBool($"{viewsPrefix}{viewName}.Disabled") ?? false)
                    {
                        _disabledViews.Add(viewName);
                        continue; // The view is disabled
                    }

                    //
                    // Create the view or update it.
                    //
                    if (!_views.TryGetValue(viewName, out MetricsViewI? v))
                    {
                        v = new MetricsViewI(viewName);
                    }
                    views[viewName] = v;

                    foreach (KeyValuePair<string, IMetricsMapFactory> f in _factories)
                    {
                        if (v.AddOrUpdateMap(_communicator, f.Key, f.Value, _logger))
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
                foreach (KeyValuePair<string, MetricsViewI> v in views)
                {
                    if (!_views.ContainsKey(v.Key))
                    {
                        foreach (string n in v.Value.GetMaps())
                        {
                            updatedMaps.Add(_factories[n]);
                        }
                    }
                }
            }

            //
            // Call the updaters to update the maps.
            //
            foreach (IMetricsMapFactory f in updatedMaps)
            {
                f.Update();
            }
        }

        public (IEnumerable<string>, IEnumerable<string>) GetMetricsViewNames(Current current)
        {
            lock (this)
            {
                return (new List<string>(_views.Keys).ToArray(), _disabledViews.ToArray());
            }
        }

        public void EnableMetricsView(string name, Current current)
        {
            lock (this)
            {
                GetMetricsView(name); // Throws if unknown view.
                _communicator.SetProperty($"IceMX.Metrics.{name}.Disabled", "0");
            }
            UpdateViews();
        }

        public void DisableMetricsView(string name, Current current)
        {
            lock (this)
            {
                GetMetricsView(name); // Throws if unknown view.
                _communicator.SetProperty($"IceMX.Metrics.{name}.Disabled", "1");
            }
            UpdateViews();
        }

        public (IReadOnlyDictionary<string, Metrics?[]>, long) GetMetricsView(string viewName, Current current)
        {
            lock (this)
            {
                MetricsViewI? view = GetMetricsView(viewName);
                return (view == null ? new Dictionary<string, Metrics?[]>()
                    : view.GetMetrics() as Dictionary<string, Metrics?[]>, Time.CurrentMonotonicTimeMillis());
            }
        }

        public IEnumerable<MetricsFailures> GetMapMetricsFailures(string viewName, string mapName, Current c)
        {
            lock (this)
            {
                MetricsViewI? view = GetMetricsView(viewName);
                if (view != null)
                {
                    return view.GetFailures(mapName);
                }
                return Array.Empty<MetricsFailures>();
            }
        }

        public MetricsFailures GetMetricsFailures(string viewName, string mapName, string id, Current c)
        {
            lock (this)
            {
                MetricsViewI? view = GetMetricsView(viewName);
                if (view != null && view.GetFailures(mapName, id) is MetricsFailures f)
                {
                    return f;
                }
                return new MetricsFailures();
            }
        }

        public void RegisterMap<T>(string map, Action updater)
            where T : Metrics, new()
        {
            bool updated;
            MetricsMapFactory<T> factory;
            lock (this)
            {
                factory = new MetricsMapFactory<T>(updater);
                _factories.Add(map, factory);
                updated = AddOrUpdateMap(map, factory);
            }
            if (updated)
            {
                factory.Update();
            }
        }

        public void RegisterSubMap<S>(string map, string subMap, System.Reflection.FieldInfo field)
            where S : Metrics, new()
        {
            bool updated;
            IMetricsMapFactory? factory;
            lock (this)
            {
                if (!_factories.TryGetValue(map, out factory))
                {
                    return;
                }
                factory.RegisterSubMap<S>(subMap, field);
                RemoveMap(map);
                updated = AddOrUpdateMap(map, factory);
            }
            if (updated)
            {
                factory.Update();
            }
        }

        public void UnregisterMap(string mapName)
        {
            bool updated;
            IMetricsMapFactory? factory;
            lock (this)
            {
                if (!_factories.TryGetValue(mapName, out factory))
                {
                    return;
                }
                _factories.Remove(mapName);
                updated = RemoveMap(mapName);
            }
            if (updated)
            {
                factory.Update();
            }
        }

        public List<MetricsMap<T>> GetMaps<T>(string mapName) where T : Metrics, new()
        {
            var maps = new List<MetricsMap<T>>();
            foreach (MetricsViewI v in _views.Values)
            {
                MetricsMap<T>? map = v.GetMap<T>(mapName);
                if (map != null)
                {
                    maps.Add(map);
                }
            }
            return maps;
        }

        public ILogger GetLogger() => _logger;

        public void Updated(IReadOnlyDictionary<string, string> props)
        {
            foreach (KeyValuePair<string, string> e in props)
            {
                if (e.Key.IndexOf("IceMX.") == 0)
                {
                    // Update the metrics views using the new configuration.
                    try
                    {
                        UpdateViews();
                    }
                    catch (Exception ex)
                    {
                        _logger.Warning("unexpected exception while updating metrics view configuration:\n" +
                                        ex.ToString());
                    }
                    return;
                }
            }
        }

        private MetricsViewI? GetMetricsView(string name)
        {
            if (!_views.TryGetValue(name, out MetricsViewI? view))
            {
                if (!_disabledViews.Contains(name))
                {
                    throw new UnknownMetricsView();
                }
                return null;
            }
            return view;
        }

        private bool AddOrUpdateMap(string mapName, IMetricsMapFactory factory)
        {
            bool updated = false;
            foreach (MetricsViewI v in _views.Values)
            {
                updated |= v.AddOrUpdateMap(_communicator, mapName, factory, _logger);
            }
            return updated;
        }

        private bool RemoveMap(string mapName)
        {
            bool updated = false;
            foreach (MetricsViewI v in _views.Values)
            {
                updated |= v.RemoveMap(mapName);
            }
            return updated;
        }

        private readonly Communicator _communicator;
        private readonly ILogger _logger;
        private readonly Dictionary<string, IMetricsMapFactory> _factories =
            new Dictionary<string, IMetricsMapFactory>();
        private Dictionary<string, MetricsViewI> _views = new Dictionary<string, MetricsViewI>();
        private readonly List<string> _disabledViews = new List<string>();
    }
}
