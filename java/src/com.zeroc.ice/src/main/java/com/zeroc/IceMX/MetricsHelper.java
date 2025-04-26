// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.EndpointInfo;

import java.util.HashMap;
import java.util.Map;

public class MetricsHelper<T> {
    public static class AttributeResolver {
        private abstract class Resolver {
            abstract Object resolve(Object obj) throws Exception;

            String resolveImpl(Object obj) {
                try {
                    Object result = resolve(obj);
                    if (result != null) {
                        return result.toString();
                    }
                    return "";
                } catch (IllegalArgumentException ex) {
                    throw ex;
                } catch (Exception ex) {
                    ex.printStackTrace();
                    assert false;
                    return null;
                }
            }
        }

        protected AttributeResolver() {}

        public String resolve(MetricsHelper<?> helper, String attribute) {
            Resolver resolver = _attributes.get(attribute);
            if (resolver == null) {
                if ("none".equals(attribute)) {
                    return "";
                }
                String v = helper.defaultResolve(attribute);
                if (v != null) {
                    return v;
                }
                throw new IllegalArgumentException(attribute);
            }
            return resolver.resolveImpl(helper);
        }

        public void add(final String name, final java.lang.reflect.Method method) {
            _attributes.put(
                name,
                new Resolver() {
                    @Override
                    public Object resolve(Object obj) throws Exception {
                        return method.invoke(obj);
                    }
                });
        }

        public void add(final String name, final java.lang.reflect.Field field) {
            _attributes.put(
                name,
                new Resolver() {
                    @Override
                    public Object resolve(Object obj) throws Exception {
                        return getField(name, field, obj);
                    }
                });
        }

        public void add(
                final String name,
                final java.lang.reflect.Method method,
                final java.lang.reflect.Field field) {
            _attributes.put(
                name,
                new Resolver() {
                    @Override
                    public Object resolve(Object obj) throws Exception {
                        return getField(name, field, method.invoke(obj));
                    }
                });
        }

        public void add(
                final String name,
                final java.lang.reflect.Method method,
                final java.lang.reflect.Method subMethod) {
            _attributes.put(
                name,
                new Resolver() {
                    @Override
                    public Object resolve(Object obj) throws Exception {
                        Object o = method.invoke(obj);
                        if (o != null) {
                            return subMethod.invoke(o);
                        }
                        throw new IllegalArgumentException(name);
                    }
                });
        }

        private Object getField(String name, java.lang.reflect.Field field, Object o)
            throws IllegalArgumentException, IllegalAccessException {
            while (o != null) {
                try {
                    return field.get(o);
                } catch (IllegalArgumentException ex) {
                    // If we're dealing with an endpoint/connection information class,
                    // check if the field is from the underlying info objects.
                    if (o instanceof EndpointInfo) {
                        o = ((EndpointInfo) o).underlying;
                    } else if (o instanceof ConnectionInfo) {
                        o = ((ConnectionInfo) o).underlying;
                    } else {
                        throw ex;
                    }
                }
            }
            throw new IllegalArgumentException(name);
        }

        private Map<String, Resolver> _attributes = new HashMap<>();
    }

    protected MetricsHelper(AttributeResolver attributes) {
        _attributes = attributes;
    }

    public String resolve(String attribute) {
        return _attributes.resolve(this, attribute);
    }

    public void initMetrics(T metrics) {
        // Override in specialized helpers.
    }

    protected String defaultResolve(String attribute) {
        return null;
    }

    private final AttributeResolver _attributes;
}
