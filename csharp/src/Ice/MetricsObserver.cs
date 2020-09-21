// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using ZeroC.Ice;

namespace ZeroC.IceMX
{
    public partial class Metrics
    {
        internal object Clone() => MemberwiseClone();
    }

    internal class MetricsHelper<T> where T : Metrics
    {
        internal class AttributeResolver
        {
            private readonly Dictionary<string, Func<object, object?>> _attributes =
                new Dictionary<string, Func<object, object?>>();

            public void Add(string name, Func<object, object?> resolver) => _attributes.Add(name, resolver);

            public string Resolve(MetricsHelper<T> helper, string attribute)
            {
                if (!_attributes.TryGetValue(attribute, out Func<object, object?>? resolver))
                {
                    if (attribute == "none")
                    {
                        return "";
                    }
                    return helper.DefaultResolve(attribute) ?? throw new MissingFieldException(attribute);
                }
                return resolver(helper)?.ToString() ?? "";
            }
        }

        private readonly AttributeResolver _attributes;

        public virtual void InitMetrics(T metrics)
        {
            // Override in specialized helpers.
        }

        internal string Resolve(string attribute) => _attributes.Resolve(this, attribute);
        protected MetricsHelper(AttributeResolver attributes) => _attributes = attributes;

        protected virtual string DefaultResolve(string attribute) => throw new MissingFieldException(attribute);
    }

    internal class Observer<T> : Stopwatch, Ice.Instrumentation.IObserver where T : Metrics, new()
    {
        public delegate void MetricsUpdate(T m);

        private List<MetricsMap<T>.Entry>? _entries;
        private long _previousDelay;

        public virtual void Attach() => Start();

        public virtual void Detach()
        {
            Stop();
            long lifetime = _previousDelay + (long)(ElapsedTicks / (Frequency / 1000000.0));
            foreach (MetricsMap<T>.Entry entry in _entries!)
            {
                entry.Detach(lifetime);
            }
        }

        public virtual void Failed(string exceptionName)
        {
            foreach (MetricsMap<T>.Entry e in _entries!)
            {
                e.Failed(exceptionName);
            }
        }

        internal void ForEach(MetricsUpdate u)
        {
            foreach (MetricsMap<T>.Entry e in _entries!)
            {
                e.Execute(u);
            }
        }

        internal MetricsMap<T>.Entry? GetEntry(MetricsMap<T> map) => _entries!.FirstOrDefault(entry => entry.Map == map);

        internal ObserverImpl? GetObserver<S, ObserverImpl>(string mapName, MetricsHelper<S> helper)
            where S : Metrics, new()
            where ObserverImpl : Observer<S>, new()
        {
            List<MetricsMap<S>.Entry>? metricsObjects = null;
            foreach (MetricsMap<T>.Entry entry in _entries!)
            {
                MetricsMap<S>.Entry? e = entry.GetMatching(mapName, helper);
                if (e != null)
                {
                    if (metricsObjects == null)
                    {
                        metricsObjects = new List<MetricsMap<S>.Entry>(_entries.Count);
                    }
                    metricsObjects.Add(e);
                }
            }

            if (metricsObjects == null)
            {
                return null;
            }

            try
            {
                var obsv = new ObserverImpl();
                obsv.Init(metricsObjects, null);
                return obsv;
            }
            catch (Exception ex)
            {
                Debug.Assert(false, ex.ToString());
                return null;
            }
        }

        internal void Init(List<MetricsMap<T>.Entry> entries, Observer<T>? previous)
        {
            _entries = entries;

            if (previous == null)
            {
                return;
            }

            _previousDelay = previous._previousDelay + (long)(previous.ElapsedTicks / (Frequency / 1000000.0));
            foreach (MetricsMap<T>.Entry e in previous._entries!)
            {
                if (!_entries.Contains(e))
                {
                    e.Detach(_previousDelay);
                }
            }
        }
    }

    internal class ObserverFactory<T, O> where T : Metrics, new() where O : Observer<T>, new()
    {
        internal bool IsEnabled => _enabled;
        private volatile bool _enabled;
        private readonly List<MetricsMap<T>> _maps = new List<MetricsMap<T>>();
        private readonly MetricsAdmin _metrics;
        private readonly object _mutex = new object();
        private readonly string _name;
        private Action? _updater;

        internal ObserverFactory(MetricsAdmin metrics, string name)
        {
            _metrics = metrics;
            _name = name;
            _metrics.RegisterMap<T>(name, Update);
        }

        internal O? GetObserver(MetricsHelper<T> helper) => GetObserver(helper, null);

        internal O? GetObserver(MetricsHelper<T> helper, object? observer)
        {
            lock (_mutex)
            {
                List<MetricsMap<T>.Entry>? metricsObjects = null;
                var old = observer as O;

                foreach (MetricsMap<T> m in _maps)
                {
                    MetricsMap<T>.Entry? e = m.GetMatching(helper, old?.GetEntry(m));
                    if (e != null)
                    {
                        metricsObjects ??= new List<MetricsMap<T>.Entry>(_maps.Count);
                        metricsObjects.Add(e);
                    }
                }

                if (metricsObjects == null)
                {
                    old?.Detach();
                    return null;
                }

                try
                {
                    var obsv = new O();
                    obsv.Init(metricsObjects, old);
                    return obsv;
                }
                catch (Exception ex)
                {
                    Debug.Assert(false, ex.ToString());
                    return null;
                }
            }
        }

        internal void RegisterSubMap<S>(string subMap, Action<Metrics, Metrics[]> fieldSetter)
            where S : Metrics, new() => _metrics.RegisterSubMap<S>(_name, subMap, fieldSetter);

        internal void SetUpdater(Action? updater)
        {
            lock (_mutex)
            {
                _updater = updater;
            }
        }

        private void Update()
        {
            Action? updater;
            lock (_mutex)
            {
                _maps.Clear();
                foreach (MetricsMap<T> m in _metrics.GetMaps<T>(_name))
                {
                    _maps.Add(m);
                }
                _enabled = _maps.Count > 0;
                updater = _updater;
            }

            updater?.Invoke();
        }
    }
}
