// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System.Collections;
    using System.Threading;

    //
    // The base class for all ImplicitContext implementations
    //
    public abstract class ImplicitContextI : LocalObjectImpl, ImplicitContext
    {
	public static ImplicitContextI create(string kind)
	{
	    if(kind.Equals("None") || kind.Equals(""))
	    {
		return null;
	    }
	    else if(kind.Equals("Shared"))
	    {
		return new SharedImplicitContext();
	    }
	    else if(kind.Equals("SharedWithoutLocking"))
	    {
		return new SharedImplicitContextWithoutLocking();
	    }
	    else if(kind.Equals("PerThread"))
	    {
		return new PerThreadImplicitContext();
	    }
	    else
	    {
		throw new Ice.InitializationException(
		    "'" + kind + "' is not a valid value for Ice.ImplicitContext"); 
	    }
	}
	
	public abstract Context getContext();
	public abstract void setContext(Context newContext);
	public abstract  string get(string key);
	public abstract string getWithDefault(string key, string defaultValue);
	public abstract void set(string key, string value);
	public abstract void remove(string key);

	abstract public void write(Context prxContext, IceInternal.BasicStream os);
	abstract internal Context combine(Context prxContext);
    }
	
    internal class SharedImplicitContextWithoutLocking : ImplicitContextI
    {
	public override Context getContext()
	{
	    return (Context)_context.Clone();
	}
	    
	public override void setContext(Context context)
	{
	    if(context != null && context.Count != 0)
	    {
		_context = (Context)context.Clone();
	    }
	    else
	    {
		_context.Clear();
	    }
	}
	    
	public override string get(string key)
	{
	    if(key == null)
	    {
		key = "";
	    }
		
	    string val = _context[key];
	    if(val == null)
	    {
		throw new NotSetException(key);
	    }
	    else
	    {
		return val;
	    }
	}
	    
	public override string getWithDefault(string key, string dflt)
	{
	    if(key == null)
	    {
		key = "";
	    }
	    if(dflt == null)
	    {
		dflt = "";
	    }
		
	    string val = _context[key];
	    return val == null ? dflt : val;
	}
	    
	public override void set(string key, string value)
	{
	    if(key == null)
	    {
		key = "";
	    }
	    if(value == null)
	    {
		value = "";
	    }
		
	    _context[key] = value;
	}
	    
	public override void remove(string key)
	{
	    if(key == null)
	    {
		key = "";
	    }
		
	    if(_context.Contains(key))
	    {
		_context.Remove(key);
	    }
	    else
	    {
		throw new NotSetException(key);
	    }
	}
	    
	public override void write(Context prxContext, IceInternal.BasicStream os)
	{
	    if(prxContext.Count == 0)
	    {
		ContextHelper.write(os, _context);
	    }
	    else if(_context.Count == 0)
	    {
		ContextHelper.write(os, prxContext);
	    }
	    else
	    {
		ContextHelper.write(os, combine(prxContext));
	    }
	}
	    
	internal override Context combine(Context prxContext)
	{
	    Context combined = (Context)prxContext.Clone();
	    combined.AddRange(_context);
	    return combined;
	}
	    
	protected Context _context = new Context();
    }
	
    internal class SharedImplicitContext : SharedImplicitContextWithoutLocking
    {
	public override Context getContext()
	{
	    lock(this)
	    {
		return base.getContext();
	    }
	}
	    
	public override void setContext(Context context)
	{
	    lock(this)
	    {
		base.setContext(context);
	    }
	}
	    
	public override string get(string key)
	{
	    lock(this)
	    {
		return base.get(key);
	    }
	}
	    
	public override string getWithDefault(string key, string dflt)
	{
	    lock(this)
	    {
		return base.getWithDefault(key, dflt);
	    }
	}
	    
	public override void set(string key, string value)
	{
	    lock(this)
	    {
		base.set(key, value);
	    }
	}
	    
	public override void remove(string key)
	{
	    lock(this)
	    {
		base.remove(key);
	    }
	}
	    
	public override void write(Context prxContext, IceInternal.BasicStream os)
	{
	    if(prxContext.Count == 0)
	    {
		lock(this)
		{
		    ContextHelper.write(os, _context);
		}
	    }
	    else 
	    {
		Context ctx = null;
		lock(this)
		{
		    ctx = _context.Count == 0 ? prxContext : base.combine(prxContext); 
		}
		ContextHelper.write(os, ctx);
	    }
	}

	internal override Context combine(Context prxContext)
	{
	    lock(this)
	    {
		return base.combine(prxContext);
	    }
	}
    }

    internal class PerThreadImplicitContext : ImplicitContextI
    {
	public override Context getContext()
	{
	    Context threadContext = null;
	    lock(this)
	    {
		threadContext = (Context)_map[Thread.CurrentThread];
	    }

	    if(threadContext == null)
	    {
		threadContext = new Context();
	    }
	    return threadContext;
	}

	public override void setContext(Context context)
	{
	    if(context == null || context.Count == 0)
	    {
		lock(this)
		{
		    _map.Remove(Thread.CurrentThread);
		}
	    }
	    else
	    {
		Context threadContext = (Context)context.Clone();
		
		lock(this)
		{
		    _map.Add(Thread.CurrentThread, threadContext);
		}
	    }
	}

	public override string get(string key)
	{
	    if(key == null)
	    {
		key = "";
	    }

	    Context threadContext = null;
	    lock(this)
	    {
		threadContext = (Context)_map[Thread.CurrentThread];
	    }

	    if(threadContext == null)
	    {
		throw new NotSetException(key);
	    }
	    string val = threadContext[key];
	    if(val == null)
	    {
		throw new NotSetException(key);
	    }
	    return val;
	}

	public override string getWithDefault(string key, string dflt)
	{
	    if(key == null)
	    {
		key = "";
	    }
	    if(dflt == null)
	    {
		dflt = "";
	    }
    
	    Context threadContext = null;
	    lock(this)
	    {
		threadContext = (Context)_map[Thread.CurrentThread];
	    }

	    if(threadContext == null)
	    {
		return dflt;
	    }
	    string val = threadContext[key];
	    if(val == null)
	    {
		return dflt;
	    }
	    return val;
	}

	public override void set(string key, string value)
	{
	    if(key == null)
	    {
		key = "";
	    }
	    if(value == null)
	    {
		value = "";
	    }

	    Thread currentThread = Thread.CurrentThread;
	    
	    Context threadContext = null;
	    lock(this)
	    {
		threadContext = (Context)_map[currentThread];
	    }
	   
	    if(threadContext == null)
	    {
		threadContext = new Context();
		lock(this)
		{
		    _map.Add(currentThread, threadContext);
		}
	    }
	    
	    threadContext[key] = value;
	}

	public override void remove(string key)
	{
	    if(key == null)
	    {
		key = "";
	    }

	    Context threadContext = null;
	    lock(this)
	    {
		threadContext = (Context)_map[Thread.CurrentThread];
	    }

	    if(threadContext == null || !threadContext.Contains(key))
	    {
		throw new NotSetException(key);
	    }
	    threadContext.Remove(key);
	}

	public override void write(Context prxContext, IceInternal.BasicStream os)
	{
	    Context threadContext = null;
	    lock(this)
	    {
		threadContext = (Context)_map[Thread.CurrentThread];
	    }
	    
	    if(threadContext == null || threadContext.Count == 0)
	    {
		ContextHelper.write(os, prxContext);
	    }
	    else if(prxContext.Count == 0)
	    {
		ContextHelper.write(os, threadContext);
	    }
	    else
	    {
		Context combined = (Context)prxContext.Clone();
		combined.AddRange(threadContext);
		ContextHelper.write(os, combined);
	    }
	}

	internal override Context combine(Context prxContext)
	{
	    Context threadContext = null;
	    lock(this)
	    {
		threadContext = (Context)_map[Thread.CurrentThread];
	    }

	    Context combined = (Context)prxContext.Clone();
	    combined.AddRange(threadContext);
	    return combined;
	}

	//
	//  map Thread -> Context
	//
	private Hashtable _map = new Hashtable();
    } 
}


