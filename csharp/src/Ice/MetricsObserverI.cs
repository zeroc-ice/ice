//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using ZeroC.Ice;

namespace ZeroC.IceMX
{
    public partial class Metrics
    {
        internal object Clone() => MemberwiseClone();
    }
    internal class MetricsHelper<T> where T : Metrics
    {
        private readonly AttributeResolver _attributes;

        internal class AttributeResolver
        {
            private abstract class Resolver
            {
                protected string Name { get; }
                protected Resolver(string name) => Name = name;

                protected abstract object? Resolve(object obj);

                public string ResolveImpl(object obj)
                {
                    try
                    {
                        return Resolve(obj)?.ToString() ?? "";
                    }
                    catch (ArgumentOutOfRangeException)
                    {
                        throw;
                    }
                    catch (Exception ex)
                    {
                        throw new ArgumentOutOfRangeException(Name, ex);
                    }
                }
            }

            public string Resolve(MetricsHelper<T> helper, string attribute)
            {
                if (!_attributes.TryGetValue(attribute, out Func<object, object?>? resolver))
                {
                    if (attribute.Equals("none"))
                    {
                        return "";
                    }
                    return helper.DefaultResolve(attribute) ?? throw new ArgumentOutOfRangeException(attribute);
                }
                return resolver(helper)?.ToString() ?? "";
            }

            public void Add(string name, Func<object, object?> resolver) => _attributes.Add(name, resolver);

            private readonly Dictionary<string, Func<object, object?>> _attributes =
                new Dictionary<string, Func<object, object?>>();
        }

        protected MetricsHelper(AttributeResolver attributes) => _attributes = attributes;

        internal string Resolve(string attribute) => _attributes.Resolve(this, attribute);

        public virtual void InitMetrics(T metrics)
        {
            // Override in specialized helpers.
        }

        protected virtual string? DefaultResolve(string attribute) => null;
    }

    public class Observer<T> : Stopwatch, Ice.Instrumentation.IObserver where T : Metrics, new()
    {
        public delegate void MetricsUpdate(T m);

        public virtual void Attach() => Start();

        public virtual void Detach()
        {
            Stop();
            long lifetime = _previousDelay + (long)(ElapsedTicks / (Frequency / 1000000.0));
            foreach (MetricsMap<T>.Entry e in _objects!)
            {
                e.Detach(lifetime);
            }
        }

        public virtual void Failed(string exceptionName)
        {
            foreach (MetricsMap<T>.Entry e in _objects!)
            {
                e.Failed(exceptionName);
            }
        }

        public void ForEach(MetricsUpdate u)
        {
            foreach (MetricsMap<T>.Entry e in _objects!)
            {
                e.Execute(u);
            }
        }

        public void Init(List<MetricsMap<T>.Entry> objects, Observer<T>? previous)
        {
            _objects = objects;

            if (previous == null)
            {
                return;
            }

            _previousDelay = previous._previousDelay + (long)(previous.ElapsedTicks / (Frequency / 1000000.0));
            foreach (MetricsMap<T>.Entry e in previous._objects!)
            {
                if (!_objects.Contains(e))
                {
                    e.Detach(_previousDelay);
                }
            }
        }

        internal ObserverImpl? GetObserver<S, ObserverImpl>(string mapName, MetricsHelper<S> helper)
            where S : Metrics, new()
            where ObserverImpl : Observer<S>, new()
        {
            List<MetricsMap<S>.Entry>? metricsObjects = null;
            foreach (MetricsMap<T>.Entry entry in _objects!)
            {
                MetricsMap<S>.Entry? e = entry.GetMatching(mapName, helper);
                if (e != null)
                {
                    if (metricsObjects == null)
                    {
                        metricsObjects = new List<MetricsMap<S>.Entry>(_objects.Count);
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
            catch (Exception)
            {
                Debug.Assert(false);
                return null;
            }
        }

        public MetricsMap<T>.Entry? GetEntry(MetricsMap<T> map)
        {
            foreach (MetricsMap<T>.Entry e in _objects!)
            {
                if (e.GetMap() == map)
                {
                    return e;
                }
            }
            return null;
        }

        private List<MetricsMap<T>.Entry>? _objects;
        private long _previousDelay = 0;
    }

    internal class ObserverFactory<T, O> where T : Metrics, new() where O : Observer<T>, new()
    {
        internal ObserverFactory(MetricsAdminI metrics, string name)
        {
            _metrics = metrics;
            _name = name;
            _metrics.RegisterMap<T>(name, Update);
        }

        internal ObserverFactory(string name)
        {
            _name = name;
            _metrics = null;
        }

        internal void Destroy()
        {
            if (_metrics != null)
            {
                _metrics.UnregisterMap(_name);
            }
        }

        internal O? GetObserver(MetricsHelper<T> helper) => GetObserver(helper, null);

        internal O? GetObserver(MetricsHelper<T> helper, object? observer)
        {
            lock (this)
            {
                List<MetricsMap<T>.Entry>? metricsObjects = null;
                O? old = null;
                try
                {
                    old = (O?)observer;
                }
                catch (InvalidCastException)
                {
                }
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

                var obsv = new O();
                obsv.Init(metricsObjects, old);
                return obsv;
            }
        }

        internal void RegisterSubMap<S>(string subMap, Action<Metrics, Metrics[]> fieldSetter)
            where S : Metrics, new() => _metrics!.RegisterSubMap<S>(_name, subMap, fieldSetter);

        internal bool IsEnabled => _enabled;

        public void Update()
        {
            Action? updater;
            lock (this)
            {
                _maps.Clear();
                foreach (MetricsMap<T> m in _metrics!.GetMaps<T>(_name))
                {
                    _maps.Add(m);
                }
                _enabled = _maps.Count > 0;
                updater = _updater;
            }

            updater?.Invoke();
        }

        public void SetUpdater(Action? updater)
        {
            lock (this)
            {
                _updater = updater;
            }
        }

        private readonly MetricsAdminI? _metrics;
        private readonly string _name;
        private readonly List<MetricsMap<T>> _maps = new List<MetricsMap<T>>();
        private volatile bool _enabled;
        private Action? _updater;
    }
}
