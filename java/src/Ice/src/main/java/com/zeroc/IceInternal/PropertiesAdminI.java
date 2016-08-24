// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.PropertiesAdminUpdateCallback;

class PropertiesAdminI implements com.zeroc.Ice.PropertiesAdmin, com.zeroc.Ice.NativePropertiesAdmin
{
    public PropertiesAdminI(Instance instance)
    {
        _properties = instance.initializationData().properties;
        _logger = instance.initializationData().logger;
    }

    @Override
    public synchronized String getProperty(String name, com.zeroc.Ice.Current current)
    {
        return _properties.getProperty(name);
    }

    @Override
    public synchronized java.util.TreeMap<String, String>
    getPropertiesForPrefix(String name, com.zeroc.Ice.Current current)
    {
        return new java.util.TreeMap<>(_properties.getPropertiesForPrefix(name));
    }

    @Override
    synchronized public void setProperties(java.util.Map<String, String> props, com.zeroc.Ice.Current current)
    {
        java.util.Map<String, String> old = _properties.getPropertiesForPrefix("");
        final int traceLevel = _properties.getPropertyAsInt("Ice.Trace.Admin.Properties");

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
        java.util.Map<String, String> added = new java.util.HashMap<>();
        java.util.Map<String, String> changed = new java.util.HashMap<>();
        java.util.Map<String, String> removed = new java.util.HashMap<>();
        for(java.util.Map.Entry<String, String> e : props.entrySet())
        {
            final String key = e.getKey();
            final String value = e.getValue();
            if(!old.containsKey(key))
            {
                if(value.length() > 0)
                {
                    //
                    // This property is new.
                    //
                    added.put(key, value);
                }
            }
            else
            {
                if(!value.equals(old.get(key)))
                {
                    if(value.length() == 0)
                    {
                        //
                        // This property was removed.
                        //
                        removed.put(key, value);
                    }
                    else
                    {
                        //
                        // This property has changed.
                        //
                        changed.put(key, value);
                    }
                }

                old.remove(key);
            }
        }

        if(traceLevel > 0 && (!added.isEmpty() || !changed.isEmpty() || !removed.isEmpty()))
        {
            StringBuilder out = new StringBuilder(128);
            out.append("Summary of property changes");

            if(!added.isEmpty())
            {
                out.append("\nNew properties:");
                for(java.util.Map.Entry<String, String> e : added.entrySet())
                {
                    out.append("\n  ");
                    out.append(e.getKey());
                    if(traceLevel > 1)
                    {
                        out.append(" = ");
                        out.append(e.getValue());
                    }
                }
            }

            if(!changed.isEmpty())
            {
                out.append("\nChanged properties:");
                for(java.util.Map.Entry<String, String> e : changed.entrySet())
                {
                    out.append("\n  ");
                    out.append(e.getKey());
                    if(traceLevel > 1)
                    {
                        out.append(" = ");
                        out.append(e.getValue());
                        out.append(" (old value = ");
                        out.append(_properties.getProperty(e.getKey()));
                        out.append(")");
                    }
                }
            }

            if(!removed.isEmpty())
            {
                out.append("\nRemoved properties:");
                for(java.util.Map.Entry<String, String> e : removed.entrySet())
                {
                    out.append("\n  ");
                    out.append(e.getKey());
                }
            }

            _logger.trace(_traceCategory, out.toString());
        }

        //
        // Update the property set.
        //

        for(java.util.Map.Entry<String, String> e : added.entrySet())
        {
            _properties.setProperty(e.getKey(), e.getValue());
        }

        for(java.util.Map.Entry<String, String> e : changed.entrySet())
        {
            _properties.setProperty(e.getKey(), e.getValue());
        }

        for(java.util.Map.Entry<String, String> e : removed.entrySet())
        {
            _properties.setProperty(e.getKey(), "");
        }

        if(!_updateCallbacks.isEmpty())
        {
            java.util.Map<String, String> changes = new java.util.HashMap<>(added);
            changes.putAll(changed);
            changes.putAll(removed);

            //
            // Copy the callbacks to allow callbacks to update the callbacks.
            //
            for(PropertiesAdminUpdateCallback callback : new java.util.ArrayList<>(_updateCallbacks))
            {
                try
                {
                    callback.updated(changes);
                }
                catch(RuntimeException ex)
                {
                    if(_properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
                    {
                        java.io.StringWriter sw = new java.io.StringWriter();
                        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                        ex.printStackTrace(pw);
                        pw.flush();
                        _logger.warning("properties admin update callback raised unexpected exception:\n" +
                                        sw.toString());
                    }
                }
            }
        }
    }

    @Override
    public synchronized void addUpdateCallback(PropertiesAdminUpdateCallback cb)
    {
        _updateCallbacks.add(cb);
    }

    @Override
    public synchronized void removeUpdateCallback(PropertiesAdminUpdateCallback cb)
    {
        _updateCallbacks.remove(cb);
    }

    private final com.zeroc.Ice.Properties _properties;
    private final com.zeroc.Ice.Logger _logger;
    private java.util.List<PropertiesAdminUpdateCallback> _updateCallbacks = new java.util.ArrayList<>();

    static private final String _traceCategory = "Admin.Properties";
}
