//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using IceInternal;

namespace IceMX
{
    public partial class Metrics
    {
        internal object Clone() => MemberwiseClone();
    }
    public class MetricsHelper<T> where T : Metrics
    {
        public class AttributeResolver
        {
            private abstract class Resolver
            {
                protected Resolver(string name) => Name = name;

                protected abstract object? Resolve(object obj);

                public string ResolveImpl(object obj)
                {
                    try
                    {
                        object? result = Resolve(obj);
                        if (result != null)
                        {
                            return result.ToString()!;
                        }
                        return "";
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

                protected object? GetField(System.Reflection.FieldInfo field, object? obj)
                {
                    while (obj != null)
                    {
                        try
                        {
                            return field.GetValue(obj);
                        }
                        catch (ArgumentException)
                        {
                            if (obj is Ice.Endpoint endpoint)
                            {
                                obj = endpoint.Underlying;
                            }
                            else if (obj is Ice.ConnectionInfo connectionInfo)
                            {
                                obj = connectionInfo.Underlying;
                            }
                            else
                            {
                                throw;
                            }
                        }
                    }
                    return null;
                }

                protected readonly string Name;
            }

            private class FieldResolverI : Resolver
            {
                internal FieldResolverI(string name, System.Reflection.FieldInfo field)
                    : base(name)
                {
                    Debug.Assert(field != null);
                    _field = field;
                }

                protected override object? Resolve(object obj) => GetField(_field, obj);

                private readonly System.Reflection.FieldInfo _field;
            }

            private class MethodResolverI : Resolver
            {
                internal MethodResolverI(string name, System.Reflection.MethodInfo method)
                    : base(name)
                {
                    Debug.Assert(method != null);
                    _method = method;
                }

                protected override object? Resolve(object obj) => _method.Invoke(obj, null);

                private readonly System.Reflection.MethodInfo _method;
            }

            private class MemberFieldResolverI : Resolver
            {
                internal MemberFieldResolverI(string name, System.Reflection.MethodInfo method,
                                              System.Reflection.FieldInfo field)
                    : base(name)
                {
                    Debug.Assert(method != null && field != null);
                    _method = method;
                    _field = field;
                }

                protected override object? Resolve(object obj)
                {
                    object? o = _method.Invoke(obj, null);
                    if (o != null)
                    {
                        return GetField(_field, o);
                    }
                    throw new ArgumentOutOfRangeException(Name);
                }

                private readonly System.Reflection.MethodInfo _method;
                private readonly System.Reflection.FieldInfo _field;
            }

            private class MemberMethodResolverI : Resolver
            {
                internal MemberMethodResolverI(string name, System.Reflection.MethodInfo method,
                                               System.Reflection.MethodInfo subMeth)
                    : base(name)
                {
                    Debug.Assert(method != null && subMeth != null);
                    _method = method;
                    _subMethod = subMeth;
                }

                protected override object? Resolve(object obj)
                {
                    object? o = _method.Invoke(obj, null);
                    if (o != null)
                    {
                        return _subMethod.Invoke(o, null);
                    }
                    throw new ArgumentOutOfRangeException(Name);
                }

                private readonly System.Reflection.MethodInfo _method;
                private readonly System.Reflection.MethodInfo _subMethod;
            }

            private class MemberPropertyResolverI : Resolver
            {
                internal MemberPropertyResolverI(string name, System.Reflection.MethodInfo method,
                                                 System.Reflection.PropertyInfo property)
                    : base(name)
                {
                    Debug.Assert(method != null && property != null);
                    _method = method;
                    _property = property;
                }

                protected override object? Resolve(object obj)
                {
                    object? o = _method.Invoke(obj, null);
                    if (o != null)
                    {
                        return _property.GetValue(o, null);
                    }
                    throw new ArgumentOutOfRangeException(Name);
                }

                private readonly System.Reflection.MethodInfo _method;
                private readonly System.Reflection.PropertyInfo _property;
            }

            protected AttributeResolver()
            {
            }

            public string Resolve(MetricsHelper<T> helper, string attribute)
            {
                if (!_attributes.TryGetValue(attribute, out MetricsHelper<T>.AttributeResolver.Resolver? resolver))
                {
                    if (attribute.Equals("none"))
                    {
                        return "";
                    }
                    string? v = helper.DefaultResolve(attribute);
                    if (v != null)
                    {
                        return v;
                    }
                    throw new ArgumentOutOfRangeException(attribute);
                }
                return resolver.ResolveImpl(helper);
            }

            public void
            Add(string name, System.Reflection.MethodInfo method) =>
                _attributes.Add(name, new MethodResolverI(name, method));

            public void
            Add(string name, System.Reflection.FieldInfo field) =>
                _attributes.Add(name, new FieldResolverI(name, field));

            public void
            Add(string name, System.Reflection.MethodInfo method, System.Reflection.FieldInfo field) =>
                _attributes.Add(name, new MemberFieldResolverI(name, method, field));

            public void
            Add(string name, System.Reflection.MethodInfo method, System.Reflection.MethodInfo subMethod) =>
                _attributes.Add(name, new MemberMethodResolverI(name, method, subMethod));

            public void
            Add(string name, System.Reflection.MethodInfo method, System.Reflection.PropertyInfo property) =>
                _attributes.Add(name, new MemberPropertyResolverI(name, method, property));

            private readonly Dictionary<string, Resolver> _attributes = new Dictionary<string, Resolver>();
        }

        protected MetricsHelper(AttributeResolver attributes) => _attributes = attributes;

        public string Resolve(string attribute) => _attributes.Resolve(this, attribute);

        public virtual void InitMetrics(T metrics)
        {
            // Override in specialized helpers.
        }

        protected virtual string? DefaultResolve(string attribute) => null;

        private readonly AttributeResolver _attributes;
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

        public ObserverImpl? GetObserver<S, ObserverImpl>(string mapName, MetricsHelper<S> helper)
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

    public class ObserverFactory<T, O> where T : Metrics, new() where O : Observer<T>, new()
    {
        public ObserverFactory(MetricsAdminI metrics, string name)
        {
            _metrics = metrics;
            _name = name;
            _metrics.RegisterMap<T>(name, Update);
        }

        public ObserverFactory(string name)
        {
            _name = name;
            _metrics = null;
        }

        public void Destroy()
        {
            if (_metrics != null)
            {
                _metrics.UnregisterMap(_name);
            }
        }

        public O? GetObserver(MetricsHelper<T> helper) => GetObserver(helper, null);

        public O? GetObserver(MetricsHelper<T> helper, object? observer)
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
                        if (metricsObjects == null)
                        {
                            metricsObjects = new List<MetricsMap<T>.Entry>(_maps.Count);
                        }
                        metricsObjects.Add(e);
                    }
                }

                if (metricsObjects == null)
                {
                    if (old != null)
                    {
                        old.Detach();
                    }
                    return null;
                }

                O obsv;
                try
                {
                    obsv = new O();
                }
                catch (Exception)
                {
                    Debug.Assert(false);
                    return null;
                }
                obsv.Init(metricsObjects, old);
                return obsv;
            }
        }

        public void RegisterSubMap<S>(string subMap, System.Reflection.FieldInfo field)
            where S : Metrics, new() => _metrics!.RegisterSubMap<S>(_name, subMap, field);

        public bool IsEnabled() => _enabled;

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
