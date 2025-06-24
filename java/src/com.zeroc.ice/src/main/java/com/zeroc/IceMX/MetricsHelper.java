// Copyright (c) ZeroC, Inc.

package com.zeroc.IceMX;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.EndpointInfo;

import java.util.HashMap;
import java.util.Map;

/**
 * Helper class for metrics operations.
 *
 * @param <T> the metrics type
 */
public class MetricsHelper<T> {
    /**
     * Resolves attribute values from metrics objects.
     */
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

        /**
         * Constructs an AttributeResolver.
         */
        protected AttributeResolver() {}

        /**
         * Resolves an attribute value using the specified helper.
         *
         * @param helper the metrics helper
         * @param attribute the attribute name to resolve
         * @return the resolved attribute value
         * @throws IllegalArgumentException if the attribute is unknown
         */
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

        /**
         * Adds an attribute resolver using a method.
         *
         * @param name the attribute name
         * @param method the method to invoke for resolution
         */
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

        /**
         * Adds an attribute resolver using a field.
         *
         * @param name the attribute name
         * @param field the field to access for resolution
         */
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

        /**
         * Adds an attribute resolver using a method and field combination.
         *
         * @param name the attribute name
         * @param method the method to invoke first
         * @param field the field to access on the method result
         */
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

        /**
         * Adds an attribute resolver using two chained methods.
         *
         * @param name the attribute name
         * @param method the first method to invoke
         * @param subMethod the second method to invoke on the first method's result
         */
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

    /**
     * Constructs a MetricsHelper with the specified attribute resolver.
     *
     * @param attributes the attribute resolver to use
     */
    protected MetricsHelper(AttributeResolver attributes) {
        _attributes = attributes;
    }

    /**
     * Resolves an attribute value.
     *
     * @param attribute the attribute name to resolve
     * @return the resolved attribute value
     * @throws IllegalArgumentException if the attribute is unknown
     */
    public String resolve(String attribute) {
        return _attributes.resolve(this, attribute);
    }

    /**
     * Initializes the metrics object.
     *
     * @param metrics the metrics object to initialize
     */
    public void initMetrics(T metrics) {
        // Override in specialized helpers.
    }

    /**
     * Provides default resolution for unknown attributes.
     *
     * @param attribute the attribute name
     * @return the default value, or null if no default is available
     */
    protected String defaultResolve(String attribute) {
        return null;
    }

    private final AttributeResolver _attributes;
}
