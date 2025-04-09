// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.function.Consumer;

class PropertiesAdminI implements PropertiesAdmin, NativePropertiesAdmin {
    public PropertiesAdminI(Instance instance) {
        _properties = instance.initializationData().properties;
        _logger = instance.initializationData().logger;
    }

    @Override
    public synchronized String getProperty(String name, Current current) {
        return _properties.getProperty(name);
    }

    @Override
    public synchronized TreeMap<String, String> getPropertiesForPrefix(
            String name, Current current) {
        return new TreeMap<>(_properties.getPropertiesForPrefix(name));
    }

    @Override
    public synchronized void setProperties(Map<String, String> props, Current current) {
        Map<String, String> old = _properties.getPropertiesForPrefix("");
        final int traceLevel = _properties.getIcePropertyAsInt("Ice.Trace.Admin.Properties");

        //
        // Compute the difference between the new property set and the existing property set:
        //
        // 1) Any properties in the new set that were not defined in the existing set.
        //
        // 2) Any properties that appear in both sets but with different values.
        //
        // 3) Any properties not present in the new set but present in the existing set.
        //    In other words, the property has been removed.
        //
        Map<String, String> added = new HashMap<>();
        Map<String, String> changed = new HashMap<>();
        Map<String, String> removed = new HashMap<>();
        for (Map.Entry<String, String> e : props.entrySet()) {
            final String key = e.getKey();
            final String value = e.getValue();
            if (!old.containsKey(key)) {
                if (!value.isEmpty()) {
                    //
                    // This property is new.
                    //
                    added.put(key, value);
                }
            } else {
                if (!value.equals(old.get(key))) {
                    if (value.isEmpty()) {
                        //
                        // This property was removed.
                        //
                        removed.put(key, value);
                    } else {
                        //
                        // This property has changed.
                        //
                        changed.put(key, value);
                    }
                }

                old.remove(key);
            }
        }

        if (traceLevel > 0 && (!added.isEmpty() || !changed.isEmpty() || !removed.isEmpty())) {
            StringBuilder out = new StringBuilder(128);
            out.append("Summary of property changes");

            if (!added.isEmpty()) {
                out.append("\nNew properties:");
                for (Map.Entry<String, String> e : added.entrySet()) {
                    out.append("\n  ");
                    out.append(e.getKey());
                    if (traceLevel > 1) {
                        out.append(" = ");
                        out.append(e.getValue());
                    }
                }
            }

            if (!changed.isEmpty()) {
                out.append("\nChanged properties:");
                for (Map.Entry<String, String> e : changed.entrySet()) {
                    out.append("\n  ");
                    out.append(e.getKey());
                    if (traceLevel > 1) {
                        out.append(" = ");
                        out.append(e.getValue());
                        out.append(" (old value = ");
                        out.append(_properties.getProperty(e.getKey()));
                        out.append(')');
                    }
                }
            }

            if (!removed.isEmpty()) {
                out.append("\nRemoved properties:");
                for (Map.Entry<String, String> e : removed.entrySet()) {
                    out.append("\n  ");
                    out.append(e.getKey());
                }
            }

            _logger.trace(_traceCategory, out.toString());
        }

        //
        // Update the property set.
        //

        for (Map.Entry<String, String> e : added.entrySet()) {
            _properties.setProperty(e.getKey(), e.getValue());
        }

        for (Map.Entry<String, String> e : changed.entrySet()) {
            _properties.setProperty(e.getKey(), e.getValue());
        }

        for (Map.Entry<String, String> e : removed.entrySet()) {
            _properties.setProperty(e.getKey(), "");
        }

        if (!_updateCallbacks.isEmpty()) {
            Map<String, String> changes = new HashMap<>(added);
            changes.putAll(changed);
            changes.putAll(removed);

            //
            // Copy the callbacks to allow callbacks to update the callbacks.
            //
            for (Consumer<Map<String, String>> callback :
                    new ArrayList<>(_updateCallbacks)) {
                callback.accept(changes);
            }
        }
    }

    @Override
    public synchronized void addUpdateCallback(
            Consumer<Map<String, String>> cb) {
        _updateCallbacks.add(cb);
    }

    @Override
    public synchronized void removeUpdateCallback(
            Consumer<Map<String, String>> cb) {
        _updateCallbacks.remove(cb);
    }

    private final Properties _properties;
    private final Logger _logger;
    private final List<Consumer<Map<String, String>>>
        _updateCallbacks = new ArrayList<>();

    private static final String _traceCategory = "Admin.Properties";
}
