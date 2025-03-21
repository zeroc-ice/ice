// Copyright (c) ZeroC, Inc.

using Ice.Internal;
using System.Diagnostics;

namespace IceMX;

public class MetricsHelper<T> where T : Metrics
{
    public class AttributeResolver
    {
        private abstract class Resolver
        {
            protected Resolver(string name)
            {
                _name = name;
            }

            protected abstract object resolve(object obj);

            public string resolveImpl(object obj)
            {
                try
                {
                    object result = resolve(obj);
                    if (result != null)
                    {
                        return result.ToString();
                    }
                    return "";
                }
                catch (ArgumentOutOfRangeException)
                {
                    throw;
                }
                catch (Exception ex)
                {
                    throw new ArgumentOutOfRangeException(_name, ex);
                }
            }

            protected object getField(System.Reflection.FieldInfo field, object obj)
            {
                while (obj != null)
                {
                    try
                    {
                        return field.GetValue(obj);
                    }
                    catch (ArgumentException)
                    {
                        if (obj is Ice.EndpointInfo)
                        {
                            obj = ((Ice.EndpointInfo)obj).underlying;
                        }
                        else if (obj is Ice.ConnectionInfo)
                        {
                            obj = ((Ice.ConnectionInfo)obj).underlying;
                        }
                        else
                        {
                            throw;
                        }
                    }
                }
                return null;
            }

            protected object getProperty(System.Reflection.PropertyInfo property, object obj) =>
                property?.GetValue(obj);

            protected readonly string _name;
        }

        private class FieldResolverI : Resolver
        {
            internal FieldResolverI(string name, System.Reflection.FieldInfo field)
                : base(name)
            {
                Debug.Assert(field != null);
                _field = field;
            }

            protected override object resolve(object obj)
            {
                return getField(_field, obj);
            }

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

            protected override object resolve(object obj)
            {
                return _method.Invoke(obj, null);
            }

            private readonly System.Reflection.MethodInfo _method;
        }

        private class MemberFieldResolverI : Resolver
        {
            internal MemberFieldResolverI(
                string name,
                System.Reflection.MethodInfo method,
                System.Reflection.FieldInfo field)
                : base(name)
            {
                Debug.Assert(method != null && field != null);
                _method = method;
                _field = field;
            }

            protected override object resolve(object obj)
            {
                object o = _method.Invoke(obj, null);
                if (o != null)
                {
                    return getField(_field, o);
                }
                throw new ArgumentOutOfRangeException(_name);
            }

            private readonly System.Reflection.MethodInfo _method;
            private readonly System.Reflection.FieldInfo _field;
        }

        private class MemberPropertyResolverI : Resolver
        {
            private readonly System.Reflection.MethodInfo _method;
            private readonly System.Reflection.PropertyInfo _property;

            internal MemberPropertyResolverI(
                string name,
                System.Reflection.MethodInfo method,
                System.Reflection.PropertyInfo property)
                : base(name)
            {
                Debug.Assert(method is not null && property is not null);
                _method = method;
                _property = property;
            }

            protected override object resolve(object obj) =>
                _method.Invoke(obj, null) is object o ?
                    getProperty(_property, o) : throw new ArgumentOutOfRangeException(_name);
        }

        private class MemberMethodResolverI : Resolver
        {
            internal MemberMethodResolverI(
                string name,
                System.Reflection.MethodInfo method,
                System.Reflection.MethodInfo subMeth)
                : base(name)
            {
                Debug.Assert(method != null && subMeth != null);
                _method = method;
                _subMethod = subMeth;
            }

            protected override object resolve(object obj)
            {
                object o = _method.Invoke(obj, null);
                if (o != null)
                {
                    return _subMethod.Invoke(o, null);
                }
                throw new ArgumentOutOfRangeException(_name);
            }

            private readonly System.Reflection.MethodInfo _method;
            private readonly System.Reflection.MethodInfo _subMethod;
        }

        protected AttributeResolver()
        {
        }

        public string resolve(MetricsHelper<T> helper, string attribute)
        {
            Resolver resolver;
            if (!_attributes.TryGetValue(attribute, out resolver))
            {
                if (attribute == "none")
                {
                    return "";
                }
                string v = helper.defaultResolve(attribute);
                if (v != null)
                {
                    return v;
                }
                throw new ArgumentOutOfRangeException(attribute);
            }
            return resolver.resolveImpl(helper);
        }

        public void
        add(string name, System.Reflection.MethodInfo method)
        {
            _attributes.Add(name, new MethodResolverI(name, method));
        }

        public void
        add(string name, System.Reflection.FieldInfo field)
        {
            _attributes.Add(name, new FieldResolverI(name, field));
        }

        public void
        add(string name, System.Reflection.MethodInfo method, System.Reflection.FieldInfo field)
        {
            _attributes.Add(name, new MemberFieldResolverI(name, method, field));
        }

        public void add(string name, System.Reflection.MethodInfo method, System.Reflection.PropertyInfo property) =>
            _attributes.Add(name, new MemberPropertyResolverI(name, method, property));

        public void
        add(string name, System.Reflection.MethodInfo method, System.Reflection.MethodInfo subMethod)
        {
            _attributes.Add(name, new MemberMethodResolverI(name, method, subMethod));
        }

        private readonly Dictionary<string, Resolver> _attributes = new Dictionary<string, Resolver>();
    }

    protected MetricsHelper(AttributeResolver attributes)
    {
        _attributes = attributes;
    }

    public string resolve(string attribute)
    {
        return _attributes.resolve(this, attribute);
    }

    public virtual void initMetrics(T metrics)
    {
        // Override in specialized helpers.
    }

    protected virtual string defaultResolve(string attribute)
    {
        return null;
    }

    private readonly AttributeResolver _attributes;
}

public class Observer<T> : Stopwatch, Ice.Instrumentation.Observer where T : Metrics, new()
{
    public delegate void MetricsUpdate(T m);

    public virtual void attach()
    {
        Start();
    }

    public virtual void detach()
    {
        Stop();
        long lifetime = _previousDelay + (long)(ElapsedTicks / (Frequency / 1000000.0));
        foreach (MetricsMap<T>.Entry e in _objects)
        {
            e.detach(lifetime);
        }
    }

    public virtual void failed(string exceptionName)
    {
        foreach (MetricsMap<T>.Entry e in _objects)
        {
            e.failed(exceptionName);
        }
    }

    public void forEach(MetricsUpdate u)
    {
        foreach (MetricsMap<T>.Entry e in _objects)
        {
            e.execute(u);
        }
    }

    public void init(MetricsHelper<T> helper, List<MetricsMap<T>.Entry> objects, Observer<T> previous)
    {
        _objects = objects;

        if (previous == null)
        {
            return;
        }

        _previousDelay = previous._previousDelay + (long)(previous.ElapsedTicks / (Frequency / 1000000.0));
        foreach (MetricsMap<T>.Entry e in previous._objects)
        {
            if (!_objects.Contains(e))
            {
                e.detach(_previousDelay);
            }
        }
    }

    public ObserverImpl getObserver<S, ObserverImpl>(string mapName, MetricsHelper<S> helper)
        where S : Metrics, new()
        where ObserverImpl : Observer<S>, new()
    {
        List<MetricsMap<S>.Entry> metricsObjects = null;
        foreach (MetricsMap<T>.Entry entry in _objects)
        {
            MetricsMap<S>.Entry e = entry.getMatching(mapName, helper);
            if (e != null)
            {
                metricsObjects ??= new List<MetricsMap<S>.Entry>(_objects.Count);
                metricsObjects.Add(e);
            }
        }

        if (metricsObjects == null)
        {
            return null;
        }

        try
        {
            ObserverImpl obsv = new ObserverImpl();
            obsv.init(helper, metricsObjects, null);
            return obsv;
        }
        catch (Exception)
        {
            Debug.Assert(false);
            return null;
        }
    }

    public MetricsMap<T>.Entry getEntry(MetricsMap<T> map)
    {
        foreach (MetricsMap<T>.Entry e in _objects)
        {
            if (e.getMap() == map)
            {
                return e;
            }
        }
        return null;
    }

    private List<MetricsMap<T>.Entry> _objects;
    private long _previousDelay;
}

public class ObserverFactory<T, O> where T : Metrics, new() where O : Observer<T>, new()
{
    public ObserverFactory(Ice.Internal.MetricsAdminI metrics, string name)
    {
        _metrics = metrics;
        _name = name;
        _metrics.registerMap<T>(name, this.update);
    }

    public ObserverFactory(string name)
    {
        _name = name;
        _metrics = null;
    }

    public void destroy()
    {
        _metrics?.unregisterMap(_name);
    }

    public O getObserver(MetricsHelper<T> helper)
    {
        return getObserver(helper, null);
    }

    public O getObserver(MetricsHelper<T> helper, object observer)
    {
        lock (_mutex)
        {
            List<MetricsMap<T>.Entry> metricsObjects = null;
            O old = null;
            try
            {
                old = (O)observer;
            }
            catch (InvalidCastException)
            {
            }
            foreach (MetricsMap<T> m in _maps)
            {
                MetricsMap<T>.Entry e = m.getMatching(helper, old?.getEntry(m));
                if (e != null)
                {
                    metricsObjects ??= new List<MetricsMap<T>.Entry>(_maps.Count);
                    metricsObjects.Add(e);
                }
            }

            if (metricsObjects == null)
            {
                old?.detach();
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
            obsv.init(helper, metricsObjects, old);
            return obsv;
        }
    }

    public void registerSubMap<S>(string subMap, System.Reflection.FieldInfo field)
        where S : Metrics, new()
    {
        _metrics.registerSubMap<S>(_name, subMap, field);
    }

    public bool isEnabled()
    {
        return _enabled;
    }

    public void update()
    {
        Action updater;
        lock (_mutex)
        {
            _maps.Clear();
            foreach (MetricsMap<T> m in _metrics.getMaps<T>(_name))
            {
                _maps.Add(m);
            }
            _enabled = _maps.Count > 0;
            updater = _updater;
        }

        if (updater != null)
        {
            updater();
        }
    }

    public void setUpdater(Action updater)
    {
        lock (_mutex)
        {
            _updater = updater;
        }
    }

    private readonly MetricsAdminI _metrics;
    private readonly string _name;
    private readonly List<MetricsMap<T>> _maps = new();
    private volatile bool _enabled;
    private Action _updater;
    private readonly object _mutex = new();
}
