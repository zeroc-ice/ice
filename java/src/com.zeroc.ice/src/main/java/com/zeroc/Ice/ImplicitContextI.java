// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/** The base class for all ImplicitContext implementations. */
abstract class ImplicitContextI implements ImplicitContext {
    public static ImplicitContextI create(String kind) {
        if (kind.isEmpty() || "None".equals(kind)) {
            return null;
        } else if ("Shared".equals(kind)) {
            return new Shared();
        } else if ("PerThread".equals(kind)) {
            return new PerThread();
        } else {
            throw new InitializationException(
                "'" + kind + "' is not a valid value for ImplicitContext");
        }
    }

    public abstract void write(Map<String, String> prxContext, OutputStream os);

    abstract Map<String, String> combine(Map<String, String> prxContext);

    static class Shared extends ImplicitContextI {
        @Override
        public synchronized Map<String, String> getContext() {
            return new HashMap<>(_context);
        }

        @Override
        public synchronized void setContext(Map<String, String> context) {
            _context.clear();
            if (context != null && !context.isEmpty()) {
                _context.putAll(context);
            }
        }

        @Override
        public synchronized boolean containsKey(String key) {
            if (key == null) {
                key = "";
            }

            return _context.containsKey(key);
        }

        @Override
        public synchronized String get(String key) {
            if (key == null) {
                key = "";
            }

            String val = _context.get(key);
            if (val == null) {
                val = "";
            }

            return val;
        }

        @Override
        public synchronized String put(String key, String value) {
            if (key == null) {
                key = "";
            }
            if (value == null) {
                value = "";
            }

            String oldVal = _context.put(key, value);
            if (oldVal == null) {
                oldVal = "";
            }
            return oldVal;
        }

        @Override
        public synchronized String remove(String key) {
            if (key == null) {
                key = "";
            }

            String val = _context.remove(key);

            if (val == null) {
                val = "";
            }
            return val;
        }

        @Override
        public void write(Map<String, String> prxContext, OutputStream os) {
            if (prxContext.isEmpty()) {
                synchronized (this) {
                    ContextHelper.write(os, _context);
                }
            } else {
                Map<String, String> ctx = null;
                synchronized (this) {
                    ctx = _context.isEmpty() ? prxContext : combine(prxContext);
                }
                ContextHelper.write(os, ctx);
            }
        }

        @Override
        synchronized Map<String, String> combine(
                Map<String, String> prxContext) {
            Map<String, String> combined = new HashMap<>(_context);
            combined.putAll(prxContext);
            return combined;
        }

        private Map<String, String> _context = new HashMap<>();
    }

    static class PerThread extends ImplicitContextI {

        @Override
        public Map<String, String> getContext() {
            //
            // Note that _map is a *synchronized* map
            //
            Map<String, String> threadContext = _map.get(Thread.currentThread());

            if (threadContext == null) {
                threadContext = new HashMap<>();
            }
            return threadContext;
        }

        @Override
        public void setContext(Map<String, String> context) {
            if (context == null || context.isEmpty()) {
                _map.remove(Thread.currentThread());
            } else {
                Map<String, String> threadContext = new HashMap<>(context);
                _map.put(Thread.currentThread(), threadContext);
            }
        }

        @Override
        public boolean containsKey(String key) {
            if (key == null) {
                key = "";
            }

            Map<String, String> threadContext = _map.get(Thread.currentThread());

            if (threadContext == null) {
                return false;
            }

            return threadContext.containsKey(key);
        }

        @Override
        public String get(String key) {
            if (key == null) {
                key = "";
            }

            Map<String, String> threadContext = _map.get(Thread.currentThread());

            if (threadContext == null) {
                return "";
            }
            String val = threadContext.get(key);
            if (val == null) {
                val = "";
            }
            return val;
        }

        @Override
        public String put(String key, String value) {
            if (key == null) {
                key = "";
            }
            if (value == null) {
                value = "";
            }

            Thread currentThread = Thread.currentThread();
            Map<String, String> threadContext = _map.get(currentThread);

            if (threadContext == null) {
                threadContext = new HashMap<>();
                _map.put(currentThread, threadContext);
            }

            String oldVal = threadContext.put(key, value);
            if (oldVal == null) {
                oldVal = "";
            }
            return oldVal;
        }

        @Override
        public String remove(String key) {
            if (key == null) {
                key = "";
            }

            Map<String, String> threadContext = _map.get(Thread.currentThread());

            if (threadContext == null) {
                return null;
            }

            String val = threadContext.remove(key);

            if (val == null) {
                val = "";
            }
            return val;
        }

        @Override
        public void write(Map<String, String> prxContext, OutputStream os) {
            Map<String, String> threadContext = _map.get(Thread.currentThread());

            if (threadContext == null || threadContext.isEmpty()) {
                ContextHelper.write(os, prxContext);
            } else if (prxContext.isEmpty()) {
                ContextHelper.write(os, threadContext);
            } else {
                Map<String, String> combined = new HashMap<>(threadContext);
                combined.putAll(prxContext);
                ContextHelper.write(os, combined);
            }
        }

        @Override
        Map<String, String> combine(Map<String, String> prxContext) {
            Map<String, String> threadContext = _map.get(Thread.currentThread());

            Map<String, String> combined = new HashMap<>(threadContext);
            combined.putAll(prxContext);
            return combined;
        }

        //
        // Synchronized map Thread -> Context
        //
        private Map<Thread, Map<String, String>> _map =
            Collections.synchronizedMap(new HashMap<>());
    }
}
