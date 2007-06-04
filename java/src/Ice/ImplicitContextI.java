// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

//
// The base class for all ImplicitContext implementations
//
public abstract class ImplicitContextI implements ImplicitContext
{
    public static ImplicitContextI create(String kind)
    {
        if(kind.equals("None") || kind.equals(""))
        {
            return null;
        }
        else if(kind.equals("Shared"))
        {
            return new Shared();
        }
        else if(kind.equals("PerThread"))
        {
            return new PerThread();
        }
        else
        {
            throw new Ice.InitializationException(
                "'" + kind + "' is not a valid value for Ice.ImplicitContext"); 
        }
    }

    abstract public void write(java.util.Map prxContext, IceInternal.BasicStream os);
    abstract java.util.Map combine(java.util.Map prxContext);


    static class Shared extends ImplicitContextI
    {
        public synchronized java.util.Map getContext()
        {
            return (java.util.Map)_context.clone();
        }
        
        public synchronized void setContext(java.util.Map context)
        {
            _context.clear();
            if(context != null && !context.isEmpty())
            {
                _context.putAll(context);
            }
        }
        
        public synchronized boolean containsKey(String key)
        {
            if(key == null)
            {
                key = "";
            }

            return _context.containsKey(key);
        }

        public synchronized String get(String key)
        {
            if(key == null)
            {
                key = "";
            }

            String val = (String)_context.get(key);
            if(val == null)
            {
                val = "";
            }
           
            return val;
        }

        public synchronized String put(String key, String value)
        {
            if(key == null)
            {
                key = "";
            }
            if(value == null)
            {
                value = "";
            }

            String oldVal = (String)_context.put(key, value);
            if(oldVal == null)
            {
                oldVal = "";
            }
            return oldVal;
        }

        public synchronized String remove(String key)
        {
            if(key == null)
            {
                key = "";
            }

            String val = (String)_context.remove(key);

            if(val == null)
            {
                val = "";
            }
            return val;
        }

        public void write(java.util.Map prxContext, IceInternal.BasicStream os)
        {
            if(prxContext.isEmpty())
            {
                synchronized(this)
                {
                    ContextHelper.write(os, _context);
                }
            }
            else 
            {
                java.util.Map ctx = null;
                synchronized(this)
                {
                    ctx = _context.isEmpty() ? prxContext : combine(prxContext); 
                }
                ContextHelper.write(os, ctx);
            }
        }

        synchronized java.util.Map combine(java.util.Map prxContext)
        {
            java.util.Map combined = (java.util.Map)_context.clone();
            combined.putAll(prxContext);
            return combined;
        }

        private java.util.HashMap _context = new java.util.HashMap();
    }

    static class PerThread extends ImplicitContextI
    {
        
        public java.util.Map getContext()
        {
            //
            // Note that _map is a *synchronized* map
            //
            java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());
            
            if(threadContext == null)
            {
                threadContext = new java.util.HashMap();
            }
            return threadContext;
        }

        public void setContext(java.util.Map context)
        {
            if(context == null || context.isEmpty())
            {
                _map.remove(Thread.currentThread());
            }
            else
            {
                java.util.HashMap threadContext = new java.util.HashMap(context);
                _map.put(Thread.currentThread(), threadContext);
            }
        }

        public boolean containsKey(String key)
        {
            if(key == null)
            {
                key = "";
            }

            java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());

            if(threadContext == null)
            {
                return false;
            }

            return threadContext.containsKey(key);
        }

        public String get(String key)
        {
            if(key == null)
            {
                key = "";
            }

            java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());

            if(threadContext == null)
            {
                return "";
            }
            String val = (String)threadContext.get(key);
            if(val == null)
            {
                val = "";
            }
            return val;
        }

        public String put(String key, String value)
        {
            if(key == null)
            {
                key = "";
            }
            if(value == null)
            {
                value = "";
            }

            Thread currentThread = Thread.currentThread();
            java.util.HashMap threadContext = (java.util.HashMap)_map.get(currentThread);

            if(threadContext == null)
            {
                threadContext = new java.util.HashMap();
                _map.put(currentThread, threadContext);
            }
            
            String oldVal = (String)threadContext.put(key, value);
            if(oldVal == null)
            {
                oldVal = "";
            }
            return oldVal;
        }

        public String remove(String key)
        {
            if(key == null)
            {
                key = "";
            }

            java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());

            if(threadContext == null)
            {
                return null;
            }

            String val = (String)threadContext.remove(key);

            if(val == null)
            {
                val = "";
            }
            return val;
        }

        public void write(java.util.Map prxContext, IceInternal.BasicStream os)
        {
            java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());
            
            if(threadContext == null || threadContext.isEmpty())
            {
                ContextHelper.write(os, prxContext);
            }
            else if(prxContext.isEmpty())
            {
                ContextHelper.write(os, threadContext);
            }
            else
            {
                java.util.Map combined = (java.util.Map)threadContext.clone();
                combined.putAll(prxContext);
                ContextHelper.write(os, combined);
            }
        }

        java.util.Map combine(java.util.Map prxContext)
        {
            java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());

            java.util.Map combined = (java.util.Map)threadContext.clone();
            combined.putAll(prxContext);
            return combined;
        }

        //
        // Synchronized map Thread -> Context
        //
        private java.util.Map _map = java.util.Collections.synchronizedMap(new java.util.HashMap());

    } 
}
