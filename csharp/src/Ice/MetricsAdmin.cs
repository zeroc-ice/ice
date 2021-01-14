// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.IceMX;

namespace ZeroC.Ice
{
    internal interface IMetricsMap
    {
        MetricsFailures[] GetFailures();
        MetricsFailures? GetFailures(string id);
        Metrics[] GetMetrics();
        IReadOnlyDictionary<string, string> GetProperties();
    }

    internal interface IMetricsMapFactory
    {
        IMetricsMap Create(string mapPrefix, Communicator communicator);
        void RegisterSubMap<S>(string subMap, Action<Metrics, Metrics[]> fieldSetter) where S : Metrics, new();
        void Update();
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

    internal class MetricsMap<T> : IMetricsMap where T : Metrics, new()
    {
        internal class Entry
        {
            internal MetricsMap<T> Map { get; }

            internal MetricsFailures? Failures => _failures == null ?
                (MetricsFailures?)null : new MetricsFailures(_object.Id, new Dictionary<string, int>(_failures));
            internal string Id => _object.Id;
            internal bool IsDetached => _object.Current == 0;

            private Dictionary<string, int>? _failures;
            private readonly object _mutex;
            private readonly T _object;
            private Dictionary<string, ISubMap>? _subMaps;

            internal Entry(object mutex, MetricsMap<T> map, T obj)
            {
                _mutex = mutex;
                Map = map;
                _object = obj;
            }

            internal void Attach(MetricsHelper<T> helper)
            {
                ++_object.Total;
                ++_object.Current;
                helper.InitMetrics(_object);
            }

            internal void Detach(long lifetime)
            {
                lock (_mutex)
                {
                    _object.TotalLifetime += lifetime;
                    if (--_object.Current == 0)
                    {
                        Map.Detached(this);
                    }
                }
            }

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

            internal void Execute(Observer<T>.MetricsUpdate func)
            {
                lock (_mutex)
                {
                    func(_object);
                }
            }

            internal void Failed(string exceptionName)
            {
                lock (_mutex)
                {
                    ++_object.Failures;
                    _failures ??= new Dictionary<string, int>();

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
                lock (_mutex)
                {
                    if (_subMaps == null || !_subMaps.TryGetValue(mapName, out m))
                    {
                        m = Map.CreateSubMap(mapName);
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
        }

        private readonly IReadOnlyDictionary<string, Regex> _accept;
        private LinkedList<Entry>? _detachedQueue;
        private readonly IReadOnlyList<string> _groupByAttributes;
        private readonly IReadOnlyList<string> _groupBySeparators;
        private readonly object _mutex = new object();
        private readonly Dictionary<string, Entry> _entries = new Dictionary<string, Entry>();
        private readonly IReadOnlyDictionary<string, string> _properties;
        private readonly IReadOnlyDictionary<string, Regex> _reject;
        private readonly int _retain;
        private readonly IReadOnlyDictionary<string, ISubMapCloneFactory>? _subMapCloneFactories;

        public MetricsFailures[] GetFailures()
        {
            lock (_mutex)
            {
                return _entries.Values.Where(entry => entry.Failures != null).Select(
                    entry => entry.Failures!.Value).ToArray();
            }
        }

        public Metrics[] GetMetrics()
        {
            lock (_mutex)
            {
                return _entries.Values.Select(entry => entry.Clone()).ToArray();
            }
        }

        public IReadOnlyDictionary<string, string> GetProperties() => _properties;

        internal MetricsMap(string mapPrefix, Communicator communicator, Dictionary<string, ISubMapFactory>? subMaps)
        {
            MetricsAdmin.ValidateProperties(mapPrefix, communicator);
            _properties = communicator.GetProperties(forPrefix: mapPrefix);

            _retain = communicator.GetPropertyAsInt($"{mapPrefix}RetainDetached") ?? 10;
            _accept = ParseRule(communicator, $"{mapPrefix}Accept");
            _reject = ParseRule(communicator, $"{mapPrefix}Reject");
            var groupByAttributes = new List<string>();
            var groupBySeparators = new List<string>();

            string groupBy = communicator.GetProperty($"{mapPrefix}GroupBy") ?? "id";
            if (groupBy.Length > 0)
            {
                string v = "";
                bool attribute = char.IsLetter(groupBy[0]) || char.IsDigit(groupBy[0]);
                if (!attribute)
                {
                    groupByAttributes.Add("");
                }

                foreach (char p in groupBy)
                {
                    bool isAlphaNum = char.IsLetter(p) || char.IsDigit(p) || p == '.';
                    if (attribute && !isAlphaNum)
                    {
                        groupByAttributes.Add(v);
                        v = "" + p;
                        attribute = false;
                    }
                    else if (!attribute && isAlphaNum)
                    {
                        groupBySeparators.Add(v);
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
                    groupByAttributes.Add(v);
                }
                else
                {
                    groupBySeparators.Add(v);
                }
            }
            _groupByAttributes = groupByAttributes.ToImmutableList();
            _groupBySeparators = groupBySeparators.ToImmutableList();

            if (subMaps != null && subMaps.Count > 0)
            {
                _subMapCloneFactories = new Dictionary<string, ISubMapCloneFactory>();

                var subMapNames = new List<string>();
                var subMapCloneFactories = new Dictionary<string, ISubMapCloneFactory>();
                foreach ((string key, ISubMapFactory value) in subMaps)
                {
                    subMapNames.Add(key);
                    string subAllMapsPrefix = $"{mapPrefix}Map.";
                    string subMapPrefix = $"{subAllMapsPrefix}{key}.";
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

                    subMapCloneFactories.Add(key, value.CreateCloneFactory(subMapPrefix, communicator));
                }
                _subMapCloneFactories = subMapCloneFactories.ToImmutableDictionary();
            }
            else
            {
                _subMapCloneFactories = null;
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
            _subMapCloneFactories = map._subMapCloneFactories;
        }

        internal ISubMap? CreateSubMap(string subMapName) =>
            _subMapCloneFactories != null && _subMapCloneFactories.TryGetValue(subMapName, out ISubMapCloneFactory? factory) ?
                factory.Create() : null;

        public MetricsFailures? GetFailures(string id)
        {
            lock (_mutex)
            {
                return _entries.TryGetValue(id, out MetricsMap<T>.Entry? entry) ? entry.Failures : null;
            }
        }

        internal Entry? GetMatching(MetricsHelper<T> helper, Entry? previous)
        {
            // Check the accept filters.
            foreach ((string name, Regex value) in _accept)
            {
                if (!Match(name, value, helper, false))
                {
                    return null;
                }
            }

            // Check the reject filters.
            foreach ((string name, Regex value) in _reject)
            {
                if (Match(name, value, helper, true))
                {
                    return null;
                }
            }

            // Compute the key from the GroupBy property.
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
            catch
            {
                return null;
            }

            // Lookup the metrics object.
            lock (_mutex)
            {
                if (previous != null && previous.Id.Equals(key))
                {
                    Debug.Assert(_entries[key] == previous);
                    return previous;
                }

                if (!_entries.TryGetValue(key, out MetricsMap<T>.Entry? e))
                {
                    var t = new T();
                    t.Id = key;
                    e = new Entry(_mutex, this, t);
                    _entries.Add(key, e);
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
                if (p.Value == entry || !p.Value.IsDetached)
                {
                    _detachedQueue.Remove(p);
                }
                p = next;
            }

            // If there's still no room, remove the oldest entry (at the front).
            if (_detachedQueue.Count == _retain)
            {
                _entries.Remove(_detachedQueue.First!.Value.Id);
                _detachedQueue.RemoveFirst();
            }

            // Add the entry at the back of the queue.
            _detachedQueue.AddLast(entry);
        }

        private static bool Match(string attribute, Regex regex, MetricsHelper<T> helper, bool reject)
        {
            string value;
            try
            {
                value = helper.Resolve(attribute);
            }
            catch
            {
                return !reject;
            }
            return regex.IsMatch(value);
        }

        private static Dictionary<string, Regex> ParseRule(Communicator communicator, string name)
        {
            var rules = new Dictionary<string, Regex>();
            foreach ((string key, string value) in communicator.GetProperties(forPrefix: $"{name}."))
            {
                rules.Add(key[(name.Length + 1)..], new Regex(value));
            }
            return rules;
        }
    }

    internal class MetricsAdmin : IAsyncMetricsAdmin
    {
        internal ILogger Logger { get; }

        private static readonly string[] _suffixes =
            {
                "Disabled",
                "GroupBy",
                "Accept.*",
                "Reject.*",
                "RetainDetached",
                "Map.*",
            };

        private readonly Communicator _communicator;
        private readonly List<string> _disabledViews = new List<string>();
        private readonly Dictionary<string, IMetricsMapFactory> _factories =
            new Dictionary<string, IMetricsMapFactory>();
        private readonly object _mutex = new object();
        private Dictionary<string, MetricsView> _views = new Dictionary<string, MetricsView>();

        public ValueTask DisableMetricsViewAsync(string name, Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                GetMetricsView(name); // Throws if unknown view.
                _communicator.SetProperty($"IceMX.Metrics.{name}.Disabled", "1");
            }
            UpdateViews();
            return default;
        }

        public ValueTask EnableMetricsViewAsync(string name, Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                GetMetricsView(name); // Throws if unknown view.
                _communicator.SetProperty($"IceMX.Metrics.{name}.Disabled", "0");
            }
            UpdateViews();
            return default;
        }

        public ValueTask<IEnumerable<MetricsFailures>> GetMapMetricsFailuresAsync(
            string viewName,
            string mapName,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                return new(GetMetricsView(viewName) is MetricsView view ?
                    view.GetFailures(mapName) : Array.Empty<MetricsFailures>());
            }
        }

        public ValueTask<MetricsFailures> GetMetricsFailuresAsync(
            string viewName,
            string mapName,
            string id,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                return new(GetMetricsView(viewName) is MetricsView view &&
                    view.GetFailures(mapName, id) is MetricsFailures failures ?
                        failures : new MetricsFailures(id, new Dictionary<string, int>()));
            }
        }

        public ValueTask<(IReadOnlyDictionary<string, Metrics?[]>, long)> GetMetricsViewAsync(
            string viewName,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                if (GetMetricsView(viewName) is MetricsView view)
                {
                    return new(
                        (view.GetMetrics() as Dictionary<string, Metrics?[]>, (long)Time.Elapsed.TotalMilliseconds));
                }
                else
                {
                    return new((ImmutableDictionary<string, Metrics?[]>.Empty, (long)Time.Elapsed.TotalMilliseconds));
                }
            }
        }

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> GetMetricsViewNamesAsync(
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                return new((_views.Keys.ToArray(), _disabledViews.ToArray()));
            }
        }

        public void RegisterMap<T>(string map, Action updater) where T : Metrics, new()
        {
            bool updated;
            MetricsMapFactory<T> factory;
            lock (_mutex)
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

        public void RegisterSubMap<S>(string map, string subMap, Action<Metrics, Metrics[]> fieldSetter)
            where S : Metrics, new()
        {
            bool updated;
            IMetricsMapFactory? factory;
            lock (_mutex)
            {
                if (!_factories.TryGetValue(map, out factory))
                {
                    return;
                }
                factory.RegisterSubMap<S>(subMap, fieldSetter);
                RemoveMap(map);
                updated = AddOrUpdateMap(map, factory);
            }
            if (updated)
            {
                factory.Update();
            }
        }

        internal MetricsAdmin(Communicator communicator, ILogger logger)
        {
            Logger = logger;
            _communicator = communicator;
            UpdateViews();
        }

        internal List<MetricsMap<T>> GetMaps<T>(string mapName) where T : Metrics, new()
        {
            var maps = new List<MetricsMap<T>>();
            foreach (MetricsView v in _views.Values)
            {
                MetricsMap<T>? map = v.GetMap<T>(mapName);
                if (map != null)
                {
                    maps.Add(map);
                }
            }
            return maps;
        }

        internal void Updated(IReadOnlyDictionary<string, string> props)
        {
            if (props.Keys.FirstOrDefault(
                    key => key.IndexOf("IceMX.", StringComparison.InvariantCulture) == 0) != null)
            {
                // Update the metrics views using the new configuration.
                try
                {
                    UpdateViews();
                }
                catch (Exception ex)
                {
                    Logger.Warning($"unexpected exception while updating metrics view configuration:\n{ex}");
                }
            }
        }

        internal static void ValidateProperties(string prefix, Communicator communicator)
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

            if (unknownProps.Count != 0 && communicator.WarnUnknownProperties)
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

        private bool AddOrUpdateMap(string mapName, IMetricsMapFactory factory)
        {
            bool updated = false;
            foreach (MetricsView v in _views.Values)
            {
                updated |= v.AddOrUpdateMap(_communicator, mapName, factory, Logger);
            }
            return updated;
        }

        private MetricsView? GetMetricsView(string name) =>
            _views.TryGetValue(name, out MetricsView? view) ? view :
                (_disabledViews.Contains(name) ? (MetricsView?)null : throw new UnknownMetricsView());

        private bool RemoveMap(string mapName)
        {
            bool updated = false;
            foreach (MetricsView v in _views.Values)
            {
                updated |= v.RemoveMap(mapName);
            }
            return updated;
        }

        private void UpdateViews()
        {
            var updatedMaps = new HashSet<IMetricsMapFactory>();
            lock (_mutex)
            {
                string viewsPrefix = "IceMX.Metrics.";
                Dictionary<string, string> viewsProperties = _communicator.GetProperties(forPrefix: viewsPrefix);
                var views = new Dictionary<string, MetricsView>();
                _disabledViews.Clear();
                foreach (string propery in viewsProperties.Keys)
                {
                    string viewName = propery[viewsPrefix.Length..];
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

                    // Create the view or update it.
                    if (!_views.TryGetValue(viewName, out MetricsView? v))
                    {
                        v = new MetricsView(viewName);
                    }
                    views[viewName] = v;

                    foreach ((string key, IMetricsMapFactory value) in _factories)
                    {
                        if (v.AddOrUpdateMap(_communicator, key, value, Logger))
                        {
                            updatedMaps.Add(value);
                        }
                    }
                }

                // Swap the dictionaries
                (_views, views) = (views, _views);

                // Go through removed views to collect maps to update.
                foreach ((string key, MetricsView value) in views)
                {
                    if (!_views.ContainsKey(key))
                    {
                        foreach (string map in value.Maps)
                        {
                            updatedMaps.Add(_factories[map]);
                        }
                    }
                }
            }

            // Call the updaters to update the maps.
            foreach (IMetricsMapFactory factory in updatedMaps)
            {
                factory.Update();
            }
        }

        private class MetricsMapFactory<T> : IMetricsMapFactory where T : Metrics, new()
        {
            private readonly Action _updater;
            private readonly Dictionary<string, ISubMapFactory> _subMaps = new Dictionary<string, ISubMapFactory>();

            public MetricsMapFactory(Action updater) => _updater = updater;

            public IMetricsMap Create(string mapPrefix, Communicator communicator) =>
                new MetricsMap<T>(mapPrefix, communicator, _subMaps);

            public void RegisterSubMap<S>(string subMap, Action<Metrics, Metrics[]> fieldSetter)
                where S : Metrics, new() => _subMaps.Add(subMap, new SubMapFactory<S>(fieldSetter));

            public void Update() => _updater();
        }
    }

    internal class MetricsView
    {
        internal ICollection<string> Maps => _maps.Keys;
        private readonly Dictionary<string, IMetricsMap> _maps = new Dictionary<string, IMetricsMap>();
        private readonly string _name;

        internal MetricsView(string name) => _name = name;

        internal bool AddOrUpdateMap(
            Communicator communicator,
            string mapName,
            IMetricsMapFactory factory,
            ILogger logger)
        {
            // Add maps to views configured with the given map.
            string viewPrefix = $"IceMX.Metrics.{_name}.";
            string allMapsPrefix = $"{viewPrefix}Map.";
            Dictionary<string, string> allMapsProps = communicator.GetProperties(forPrefix: allMapsPrefix);

            string mapPrefix;
            Dictionary<string, string> mapProps;
            if (allMapsProps.Count > 0)
            {
                mapPrefix = $"{allMapsPrefix}{mapName}.";
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
                logger.Warning($"unexpected exception while creating metrics map:\n{ex}");
                _maps.Remove(mapName);
            }
            return true;
        }

        internal MetricsFailures[] GetFailures(string mapName) =>
            _maps.TryGetValue(mapName, out IMetricsMap? map) ? map.GetFailures() : Array.Empty<MetricsFailures>();

        internal MetricsFailures? GetFailures(string mapName, string id) =>
            _maps.TryGetValue(mapName, out IMetricsMap? map) ? map.GetFailures(id) : null;

        internal MetricsMap<T>? GetMap<T>(string mapName) where T : Metrics, new() =>
            _maps.TryGetValue(mapName, out IMetricsMap? map) ? (MetricsMap<T>)map : null;

        internal Dictionary<string, Metrics[]> GetMetrics() =>
            _maps.ToDictionary(entry => entry.Key, entry => entry.Value.GetMetrics());

        internal bool RemoveMap(string mapName) => _maps.Remove(mapName);
    }

    internal class SubMap<S> : ISubMap where S : Metrics, new()
    {
        private readonly MetricsMap<S> _map;
        private readonly Action<Metrics, Metrics[]> _fieldSetter;

        public void AddSubMapToMetrics(Metrics metrics) => _fieldSetter(metrics, _map.GetMetrics());

        internal SubMap(MetricsMap<S> map, Action<Metrics, Metrics[]> fieldSetter)
        {
            _map = map;
            _fieldSetter = fieldSetter;
        }

        internal MetricsMap<S>.Entry? GetMatching(MetricsHelper<S> helper) => _map.GetMatching(helper, null);
    }

    internal class SubMapCloneFactory<S> : ISubMapCloneFactory where S : Metrics, new()
    {
        private readonly MetricsMap<S> _map;
        private readonly Action<Metrics, Metrics[]> _fieldSetter;

        internal SubMapCloneFactory(MetricsMap<S> map, Action<Metrics, Metrics[]> fieldSetter)
        {
            _map = map;
            _fieldSetter = fieldSetter;
        }

        public ISubMap Create() => new SubMap<S>(new MetricsMap<S>(_map), _fieldSetter);
    }

    internal class SubMapFactory<S> : ISubMapFactory where S : Metrics, new()
    {
        private readonly Action<Metrics, Metrics[]> _fieldSetter;

        internal SubMapFactory(Action<Metrics, Metrics[]> fieldSetter) => _fieldSetter = fieldSetter;

        public ISubMapCloneFactory CreateCloneFactory(string subMapPrefix, Communicator communicator) =>
            new SubMapCloneFactory<S>(new MetricsMap<S>(subMapPrefix, communicator, null), _fieldSetter);
    }
}
