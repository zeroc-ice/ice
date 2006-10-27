// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

//
// The base class for all ImplicitContext implementations
//
public abstract class ImplicitContextI extends LocalObjectImpl implements ImplicitContext
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
	else if(kind.equals("SharedWithoutLocking"))
	{
	    return new SharedWithoutLocking();
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


    static class SharedWithoutLocking extends ImplicitContextI
    {
	public java.util.Map getContext()
	{
	    return (java.util.Map)_context.clone();
	}

	public void setContext(java.util.Map context)
	{
	    _context.clear();
	    if(context != null && !context.isEmpty())
	    {
		_context.putAll(context);
	    }
	}

	public String get(String key)
	{
	    if(key == null)
	    {
		key = "";
	    }


	    String val = (String)_context.get(key);
	    if(val == null)
	    {
		throw new Ice.NotSetException(key);
	    }
	    else
	    {
		return val;
	    }
	}

	public String getWithDefault(String key, String dflt)
	{
	    if(key == null)
	    {
		key = "";
	    }
	    if(dflt == null)
	    {
		dflt = "";
	    }

	    String val = (String)_context.get(key);
	    return val == null ? dflt : val;
	}

	public void set(String key, String value)
	{
	    if(key == null)
	    {
		key = "";
	    }
	    if(value == null)
	    {
		value = "";
	    }

	    _context.put(key, value);
	}

	public void remove(String key)
	{
	    if(key == null)
	    {
		key = "";
	    }

	    if(_context.remove(key) == null)
	    {
		throw new Ice.NotSetException(key);
	    }
	}

	public void write(java.util.Map prxContext, IceInternal.BasicStream os)
	{
	    if(prxContext.isEmpty())
	    {
		ContextHelper.write(os, _context);
	    }
	    else if(_context.isEmpty())
	    {
		ContextHelper.write(os, prxContext);
	    }
	    else
	    {
		ContextHelper.write(os, combine(prxContext));
	    }
	}

	java.util.Map combine(java.util.Map prxContext)
	{
	    java.util.Map combined = (java.util.Map)_context.clone();
	    combined.putAll(prxContext);
	    return combined;
	}

	protected java.util.HashMap _context = new java.util.HashMap();
    }

    static class Shared extends SharedWithoutLocking
    {
	public synchronized java.util.Map getContext()
	{
	    return super.getContext();
	}

	public synchronized void setContext(java.util.Map context)
	{
	    super.setContext(context);
	}

	public synchronized String get(String key)
	{
	    return super.get(key);
	}

	public synchronized String getWithDefault(String key, String dflt)
	{
	    return super.getWithDefault(key, dflt);
	}

	public synchronized void set(String key, String value)
	{
	    super.set(key, value);
	}

	public synchronized void remove(String key)
	{
	    super.remove(key);
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
		    ctx = _context.isEmpty() ? prxContext : super.combine(prxContext); 
		}
		ContextHelper.write(os, ctx);
	    }
	}

	synchronized java.util.Map combine(java.util.Map prxContext)
	{
	    return super.combine(prxContext);
	}
	
	//
	// The other write does not need to be further synchronized
	//
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

	public String get(String key)
	{
	    if(key == null)
	    {
		key = "";
	    }

	    java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());

	    if(threadContext == null)
	    {
		throw new Ice.NotSetException(key);
	    }
	    String val = (String)threadContext.get(key);
	    if(val == null)
	    {
		throw new Ice.NotSetException(key);
	    }
	    return val;
	}

	public String getWithDefault(String key, String dflt)
	{
	    if(key == null)
	    {
		key = "";
	    }
	    if(dflt == null)
	    {
		dflt = "";
	    }
    
	    java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());

	    if(threadContext == null)
	    {
		return dflt;
	    }
	    String val = (String)threadContext.get(key);
	    if(val == null)
	    {
		return dflt;
	    }
	    return val;
	}

	public void set(String key, String value)
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
	    
	    threadContext.put(key, value);
	}

	public void remove(String key)
	{
	    if(key == null)
	    {
		key = "";
	    }

	    java.util.HashMap threadContext = (java.util.HashMap)_map.get(Thread.currentThread());

	    if(threadContext == null)
	    {
		throw new Ice.NotSetException(key);
	    }
	    
	    if(threadContext.remove(key) == null)
	    {
		throw new Ice.NotSetException(key);
	    }
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
