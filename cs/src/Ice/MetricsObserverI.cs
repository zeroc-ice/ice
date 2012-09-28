// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceMX
{
    using IceInternal;
    using System;
    using System.Diagnostics;
    using System.Collections.Generic;

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
                        if(result != null)
                        {
                            return result.ToString();
                        }
                        return "";
                    }
                    catch(ArgumentOutOfRangeException ex)
                    {
                        throw ex;
                    }
                    catch(Exception ex)
                    {
                        throw new ArgumentOutOfRangeException(_name, ex);
                    }
                }
                
                readonly protected string _name;
            };

            class FieldResolverI : Resolver
            {
                internal FieldResolverI(string name, System.Reflection.FieldInfo field) : base(name)
                {
                    Debug.Assert(field != null);
                    _field = field;
                }
                
                override protected object resolve(object obj)
                {
                    return _field.GetValue(obj);
                }
                
                readonly private System.Reflection.FieldInfo _field;
            };
 
            class MethodResolverI : Resolver
            {
                internal MethodResolverI(string name, System.Reflection.MethodInfo method) : base(name)
                {
                    Debug.Assert(method != null);
                    _method = method;
                }
                
                override protected object resolve(object obj)
                {
                    return _method.Invoke(obj, null);
                }
                
                readonly private System.Reflection.MethodInfo _method;
            };

            class MemberFieldResolverI : Resolver
            {
                internal MemberFieldResolverI(string name, System.Reflection.MethodInfo method,
                                              System.Reflection.FieldInfo field)
                    : base(name)
                {
                    Debug.Assert(method != null && field != null);
                    _method = method;
                    _field = field;
                }
                
                override protected object resolve(object obj)
                {
                    object o = _method.Invoke(obj, null);
                    if(o != null)
                    {
                        return _field.GetValue(o);
                    }
                    throw new ArgumentOutOfRangeException(_name);
                }
                
                readonly private System.Reflection.MethodInfo _method;
                readonly private System.Reflection.FieldInfo _field;
            };
 
            class MemberMethodResolverI : Resolver
            {
                internal MemberMethodResolverI(string name, System.Reflection.MethodInfo method, 
                                               System.Reflection.MethodInfo subMeth)
                    : base(name)
                {
                    Debug.Assert(method != null && subMeth != null);
                    _method = method;
                    _subMethod = subMeth;
                }
                
                override protected object resolve(object obj)
                {
                    object o = _method.Invoke(obj, null);
                    if(o != null)
                    {
                        return _subMethod.Invoke(o, null);
                    }
                    throw new ArgumentOutOfRangeException(_name);
                }
                
                readonly private System.Reflection.MethodInfo _method;
                readonly private System.Reflection.MethodInfo _subMethod;
            };

            protected AttributeResolver()
            {
            }

            public string resolve(MetricsHelper<T> helper, string attribute)
            {
                Resolver resolver;
                if(!_attributes.TryGetValue(attribute, out resolver))
                {
                    if(attribute.Equals("none"))
                    {
                        return "";
                    }
                    string v = helper.defaultResolve(attribute);
                    if(v != null)
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
            
            public void 
            add(string name, System.Reflection.MethodInfo method, System.Reflection.MethodInfo subMethod)
            {
                _attributes.Add(name, new MemberMethodResolverI(name, method, subMethod));
            }

            private Dictionary<string, Resolver> _attributes = new Dictionary<string, Resolver>();
        };

        protected MetricsHelper(AttributeResolver attributes)
        {
            _attributes = attributes;
        }

        public string resolve(string attribute)
        {
            return _attributes.resolve(this, attribute);
        }

        virtual public void initMetrics(T metrics)
        {
            // Override in specialized helpers.
        }

        virtual protected string defaultResolve(string attribute)
        {
            return null;
        }

        private AttributeResolver _attributes;
    };

    public class Observer<T> : System.Diagnostics.Stopwatch, Ice.Instrumentation.Observer where T : Metrics, new()
    {
        public delegate void MetricsUpdate(T m);
    
        public void attach()
        {
            Start();
        }

        public void detach()
        {
            Stop();
            long lifetime = (long)(ElapsedTicks / (Frequency / 1000000.0));
            foreach(MetricsMap<T>.Entry e in _objects)
            {
                e.detach(lifetime);
            }
        }

        public void failed(string exceptionName)
        {
            foreach(MetricsMap<T>.Entry e in _objects)
            {
                e.failed(exceptionName);
            }
        }
    
        public void forEach(MetricsUpdate u)
        {
            foreach(MetricsMap<T>.Entry e in _objects)
            {
                e.execute(u);
            }
        }

        public void init(MetricsHelper<T> helper, List<MetricsMap<T>.Entry> objects)
        {
            Debug.Assert(_objects == null);
            _objects = objects;
            _objects.Sort();
            foreach(MetricsMap<T>.Entry e in _objects)
            {
                e.attach(helper);
            }
        }

        public void update(MetricsHelper<T> helper, List<MetricsMap<T>.Entry> objects)
        {
            objects.Sort();
            int p = objects.Count == 0 ? -1 : 0;
            int q = _objects.Count == 0 ? -1 : 0;
            long delay = (long)(ElapsedTicks / (Frequency / 1000000.0));
            while(p >= 0)
            {
                if(q >= 0 || objects[p].CompareTo(objects[q]) < 0) // New metrics object
                {
                    _objects.Insert(q, objects[p]);
                    objects[p].attach(helper);
                    ++p;
                    ++q;
                }
                else if(objects[p] == objects[q]) // Same metrics object
                {
                    ++p;
                    ++q;
                }
                else // Removed metrics object
                {
                    objects[q].detach(delay);
                    _objects.RemoveAt(q);
                }
                p = p < objects.Count ? p : -1;
                q = q < _objects.Count ? q : -1;
            }
            p = q;
            while(q < _objects.Count)
            {
                _objects[q++].detach(delay);
            }
            _objects.RemoveRange(p, _objects.Count - p);
        }

        public ObserverImpl getObserver<S, ObserverImpl>(string mapName, MetricsHelper<S> helper)
            where S : Metrics, new()
            where ObserverImpl : Observer<S>, new()
        {
            List<MetricsMap<S>.Entry> metricsObjects = new List<MetricsMap<S>.Entry>();
            foreach(MetricsMap<T>.Entry entry in _objects)
            {
                MetricsMap<S>.Entry e = entry.getMatching(mapName, helper);
                if(e != null)
                {
                    metricsObjects.Add(e);
                }
            }

            if(metricsObjects.Count == 0)
            {
                return null;
            }

            try
            {
                ObserverImpl obsv = new ObserverImpl();
                obsv.init(helper, metricsObjects);
                return obsv;
            }
            catch(Exception)
            {
                Debug.Assert(false);
                return null;
            }
        }
    
        private List<MetricsMap<T>.Entry> _objects;
    };

    public class ObserverFactory<T, O> where T : Metrics, new() where O : Observer<T>, new()
    {
        public ObserverFactory(IceInternal.MetricsAdminI metrics, string name)
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
            if(_metrics != null)
            {
                _metrics.unregisterMap(_name);
            }
        }

        public O getObserver(MetricsHelper<T> helper)
        {
            lock(this)
            {
                return getObserver(helper, null);
            }
        }

        public O getObserver(MetricsHelper<T> helper, object observer)
        {
            List<MetricsMap<T>.Entry> metricsObjects = new List<MetricsMap<T>.Entry>();
            foreach(MetricsMap<T> m in _maps)
            {
                MetricsMap<T>.Entry e = m.getMatching(helper);
                if(e != null)
                {
                    metricsObjects.Add(e);
                }
            }

            if(metricsObjects.Count == 0)
            {
                return null;
            }

            O obsv;
            if(observer == null)
            {
                try
                {
                    obsv = new O();
                }
                catch(Exception)
                {
                    Debug.Assert(false);
                    return null;
                }
                obsv.init(helper, metricsObjects);
            }
            else
            {
                obsv = (O)observer;
                obsv.update(helper, metricsObjects);
            }
            return obsv;
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
            System.Action updater;
            lock(this)
            {
                _maps.Clear();
                foreach(MetricsMap<T> m in _metrics.getMaps<T>(_name))
                {
                    _maps.Add(m);
                }
                _enabled = _maps.Count > 0;
                updater = _updater;
            }

            if(updater != null)
            {
                updater();
            }
        }

        public void setUpdater(System.Action updater)
        {
            lock(this)
            {
                _updater = updater;
            }
        }
    
        private readonly IceInternal.MetricsAdminI _metrics;
        private readonly string _name;
        private List<MetricsMap<T>> _maps = new List<MetricsMap<T>>();
        private volatile bool _enabled;
        private System.Action _updater;
    };
}