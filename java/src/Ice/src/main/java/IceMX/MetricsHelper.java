// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

public class MetricsHelper<T>
{
    public static class AttributeResolver
    {
        private abstract class Resolver
        {
            abstract Object resolve(Object obj) throws Exception;

            String resolveImpl(Object obj)
            {
                try
                {
                    Object result = resolve(obj);
                    if(result != null)
                    {
                        return result.toString();
                    }
                    return "";
                }
                catch(IllegalArgumentException ex)
                {
                    throw ex;
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                    assert(false);
                    return null;
                }
            }
        }

        protected
        AttributeResolver()
        {
        }

        public String
        resolve(MetricsHelper<?> helper, String attribute)
        {
            Resolver resolver = _attributes.get(attribute);
            if(resolver == null)
            {
                if(attribute.equals("none"))
                {
                    return "";
                }
                String v = helper.defaultResolve(attribute);
                if(v != null)
                {
                    return v;
                }
                throw new IllegalArgumentException(attribute);
            }
            return resolver.resolveImpl(helper);
        }

        public void
        add(String name, final java.lang.reflect.Method method)
        {
            _attributes.put(name, new Resolver()
                {
                    @Override
                    public Object
                    resolve(Object obj) throws Exception
                    {
                        return method.invoke(obj);
                    }
                });
        }

        public void
        add(String name, final java.lang.reflect.Field field)
        {
            _attributes.put(name, new Resolver()
                {
                    @Override
                    public Object
                    resolve(Object obj) throws Exception
                    {
                        return field.get(obj);
                    }
                });
        }

        public void
        add(final String name, final java.lang.reflect.Method method, final java.lang.reflect.Field field)
        {
            _attributes.put(name, new Resolver()
                {
                    @Override
                    public Object
                    resolve(Object obj) throws Exception
                    {
                        Object o = method.invoke(obj);
                        if(o != null)
                        {
                            return field.get(o);
                        }
                        throw new IllegalArgumentException(name);
                    }
                });
        }

        public void
        add(final String name, final java.lang.reflect.Method method, final java.lang.reflect.Method subMethod)
        {
            _attributes.put(name, new Resolver()
                {
                    @Override
                    public Object
                    resolve(Object obj) throws Exception
                    {
                        Object o = method.invoke(obj);
                        if(o != null)
                        {
                            return subMethod.invoke(o);
                        }
                        throw new IllegalArgumentException(name);
                    }
                });
        }

        private java.util.Map<String, Resolver> _attributes = new java.util.HashMap<String, Resolver>();
    }

    protected
    MetricsHelper(AttributeResolver attributes)
    {
        _attributes = attributes;
    }

    public String
    resolve(String attribute)
    {
        return _attributes.resolve(this, attribute);
    }

    public void
    initMetrics(T metrics)
    {
        // Override in specialized helpers.
    }

    protected String
    defaultResolve(String attribute)
    {
        return null;
    }

    private AttributeResolver _attributes;
}