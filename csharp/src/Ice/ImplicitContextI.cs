// Copyright (c) ZeroC, Inc.

namespace Ice;

// The base class for all ImplicitContext implementations
public abstract class ImplicitContextI : ImplicitContext
{
    public static ImplicitContextI create(string kind)
    {
        if (kind == "None" || kind.Length == 0)
        {
            return null;
        }
        else if (kind == "Shared")
        {
            return new SharedImplicitContext();
        }
        else if (kind == "PerThread")
        {
            return new PerThreadImplicitContext();
        }
        else
        {
            throw new InitializationException("'" + kind + "' is not a valid value for Ice.ImplicitContext");
        }
    }

    public abstract Dictionary<string, string> getContext();

    public abstract void setContext(Dictionary<string, string> newContext);

    public abstract bool containsKey(string key);

    public abstract string get(string key);

    public abstract string put(string key, string value);

    public abstract string remove(string key);

    public abstract void write(Dictionary<string, string> prxContext, OutputStream os);

    internal abstract Dictionary<string, string> combine(Dictionary<string, string> prxContext);
}

internal class SharedImplicitContext : ImplicitContextI
{
    public override Dictionary<string, string> getContext()
    {
        lock (_mutex)
        {
            return new Dictionary<string, string>(_context);
        }
    }

    public override void setContext(Dictionary<string, string> context)
    {
        lock (_mutex)
        {
            if (context != null && context.Count != 0)
            {
                _context = new Dictionary<string, string>(context);
            }
            else
            {
                _context.Clear();
            }
        }
    }

    public override bool containsKey(string key)
    {
        lock (_mutex)
        {
            key ??= "";

            return _context.ContainsKey(key);
        }
    }

    public override string get(string key)
    {
        lock (_mutex)
        {
            key ??= "";

            string val = _context[key] ?? "";
            return val;
        }
    }

    public override string put(string key, string value)
    {
        lock (_mutex)
        {
            key ??= "";
            value ??= "";

            string oldVal;
            _context.TryGetValue(key, out oldVal);
            oldVal ??= "";
            _context[key] = value;

            return oldVal;
        }
    }

    public override string remove(string key)
    {
        lock (_mutex)
        {
            key ??= "";

            string val = _context[key];

            if (val == null)
            {
                val = "";
            }
            else
            {
                _context.Remove(key);
            }

            return val;
        }
    }

    public override void write(Dictionary<string, string> prxContext, OutputStream os)
    {
        if (prxContext.Count == 0)
        {
            lock (_mutex)
            {
                ContextHelper.write(os, _context);
            }
        }
        else
        {
            Dictionary<string, string> ctx = null;
            lock (_mutex)
            {
                ctx = _context.Count == 0 ? prxContext : combine(prxContext);
            }
            ContextHelper.write(os, ctx);
        }
    }

    internal override Dictionary<string, string> combine(Dictionary<string, string> prxContext)
    {
        lock (_mutex)
        {
            Dictionary<string, string> combined = new Dictionary<string, string>(prxContext);
            foreach (KeyValuePair<string, string> e in _context)
            {
                try
                {
                    combined.Add(e.Key, e.Value);
                }
                catch (System.ArgumentException)
                {
                    // Ignore.
                }
            }
            return combined;
        }
    }

    private Dictionary<string, string> _context = new Dictionary<string, string>();
    private readonly object _mutex = new();
}

internal class PerThreadImplicitContext : ImplicitContextI
{
    public override Dictionary<string, string> getContext()
    {
        Dictionary<string, string> threadContext = null;
        Thread currentThread = Thread.CurrentThread;
        lock (_mutex)
        {
            if (_map.TryGetValue(currentThread, out Dictionary<string, string> value))
            {
                threadContext = value;
            }
        }

        threadContext ??= new Dictionary<string, string>();
        return threadContext;
    }

    public override void setContext(Dictionary<string, string> context)
    {
        if (context == null || context.Count == 0)
        {
            lock (_mutex)
            {
                _map.Remove(Thread.CurrentThread);
            }
        }
        else
        {
            Dictionary<string, string> threadContext = new Dictionary<string, string>(context);

            lock (_mutex)
            {
                _map.Add(Thread.CurrentThread, threadContext);
            }
        }
    }

    public override bool containsKey(string key)
    {
        key ??= "";

        Dictionary<string, string> threadContext = null;
        lock (_mutex)
        {
            if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
            {
                return false;
            }
        }

        return threadContext.ContainsKey(key);
    }

    public override string get(string key)
    {
        key ??= "";

        Dictionary<string, string> threadContext = null;
        lock (_mutex)
        {
            if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
            {
                return "";
            }
        }

        string val = threadContext[key] ?? "";
        return val;
    }

    public override string put(string key, string value)
    {
        key ??= "";
        value ??= "";

        Dictionary<string, string> threadContext = null;
        lock (_mutex)
        {
            if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
            {
                threadContext = new Dictionary<string, string>();
                _map.Add(Thread.CurrentThread, threadContext);
            }
        }

        string oldVal;
        if (!threadContext.TryGetValue(key, out oldVal))
        {
            oldVal = "";
        }

        threadContext[key] = value;
        return oldVal;
    }

    public override string remove(string key)
    {
        key ??= "";

        Dictionary<string, string> threadContext = null;
        lock (_mutex)
        {
            if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
            {
                return "";
            }
        }

        string val = null;
        if (!threadContext.TryGetValue(key, out val))
        {
            val = "";
        }
        else
        {
            threadContext.Remove(key);
        }
        return val;
    }

    public override void write(Dictionary<string, string> prxContext, OutputStream os)
    {
        Dictionary<string, string> threadContext = null;
        lock (_mutex)
        {
            _map.TryGetValue(Thread.CurrentThread, out threadContext);
        }

        if (threadContext == null || threadContext.Count == 0)
        {
            ContextHelper.write(os, prxContext);
        }
        else if (prxContext.Count == 0)
        {
            ContextHelper.write(os, threadContext);
        }
        else
        {
            Dictionary<string, string> combined = new Dictionary<string, string>(prxContext);
            foreach (KeyValuePair<string, string> e in threadContext)
            {
                try
                {
                    combined.Add(e.Key, e.Value);
                }
                catch (System.ArgumentException)
                {
                    // Ignore.
                }
            }
            ContextHelper.write(os, combined);
        }
    }

    internal override Dictionary<string, string> combine(Dictionary<string, string> prxContext)
    {
        Dictionary<string, string> threadContext = null;
        lock (_mutex)
        {
            if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
            {
                return new Dictionary<string, string>(prxContext);
            }
        }

        Dictionary<string, string> combined = new Dictionary<string, string>(prxContext);
        foreach (KeyValuePair<string, string> e in threadContext)
        {
            combined.Add(e.Key, e.Value);
        }
        return combined;
    }

    //
    //  map Thread -> Context
    //
    private Dictionary<Thread, Dictionary<string, string>> _map = new();
    private readonly object _mutex = new();
}
