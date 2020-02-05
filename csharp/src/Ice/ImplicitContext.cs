//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading;

namespace Ice
{
    //
    // The base class for all ImplicitContext implementations
    //
    public abstract class ImplicitContext : IImplicitContext
    {
        public static ImplicitContext? Create(string? kind)
        {
            if (string.IsNullOrEmpty(kind) || kind == "None")
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
                throw new InitializationException($"'{kind}' is not a valid value for Ice.ImplicitContext");
            }
        }

        public abstract Dictionary<string, string> GetContext();
        public abstract void SetContext(Dictionary<string, string> newContext);
        public abstract bool ContainsKey(string key);
        public abstract string Get(string key);
        public abstract string Put(string key, string value);
        public abstract string Remove(string key);

        public abstract void Write(Dictionary<string, string> prxContext, OutputStream os);
        internal abstract Dictionary<string, string> Combine(Dictionary<string, string> prxContext);
    }

    internal class SharedImplicitContext : ImplicitContext
    {
        public override Dictionary<string, string> GetContext()
        {
            lock (this)
            {
                return new Dictionary<string, string>(_context);
            }
        }

        public override void SetContext(Dictionary<string, string> context)
        {
            lock (this)
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

        public override bool ContainsKey(string key)
        {
            lock (this)
            {
                if (key == null)
                {
                    key = "";
                }

                return _context.ContainsKey(key);
            }
        }

        public override string Get(string key)
        {
            lock (this)
            {
                if (key == null)
                {
                    key = "";
                }

                string val = _context[key];
                if (val == null)
                {
                    val = "";
                }
                return val;
            }
        }

        public override string Put(string key, string value)
        {
            lock (this)
            {
                if (key == null)
                {
                    key = "";
                }
                if (value == null)
                {
                    value = "";
                }

                _context.TryGetValue(key, out string oldVal);
                if (oldVal == null)
                {
                    oldVal = "";
                }
                _context[key] = value;

                return oldVal;
            }
        }

        public override string Remove(string key)
        {
            lock (this)
            {
                if (key == null)
                {
                    key = "";
                }

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

        public override void Write(Dictionary<string, string> prxContext, OutputStream ostr)
        {
            if (prxContext.Count == 0)
            {
                lock (this)
                {
                    Ice.ContextHelper.Write(ostr, _context);
                }
            }
            else
            {
                Dictionary<string, string> ctx;
                lock (this)
                {
                    ctx = _context.Count == 0 ? prxContext : Combine(prxContext);
                }
                Ice.ContextHelper.Write(ostr, ctx);
            }
        }

        internal override Dictionary<string, string> Combine(Dictionary<string, string> prxContext)
        {
            lock (this)
            {
                var combined = new Dictionary<string, string>(prxContext);
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
    }

    internal class PerThreadImplicitContext : ImplicitContext
    {
        public override Dictionary<string, string> GetContext()
        {
            Dictionary<string, string>? threadContext = null;
            Thread currentThread = Thread.CurrentThread;
            lock (this)
            {
                if (_map.ContainsKey(currentThread))
                {
                    threadContext = _map[currentThread];
                }
            }

            if (threadContext == null)
            {
                threadContext = new Dictionary<string, string>();
            }
            return threadContext;
        }

        public override void SetContext(Dictionary<string, string> context)
        {
            if (context == null || context.Count == 0)
            {
                lock (this)
                {
                    _map.Remove(Thread.CurrentThread);
                }
            }
            else
            {
                var threadContext = new Dictionary<string, string>(context);

                lock (this)
                {
                    _map.Add(Thread.CurrentThread, threadContext);
                }
            }
        }

        public override bool ContainsKey(string key)
        {
            if (key == null)
            {
                key = "";
            }

            Dictionary<string, string> threadContext;
            lock (this)
            {
                if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
                {
                    return false;
                }
            }

            return threadContext.ContainsKey(key);
        }

        public override string Get(string key)
        {
            if (key == null)
            {
                key = "";
            }

            Dictionary<string, string> threadContext;
            lock (this)
            {
                if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
                {
                    return "";
                }
            }

            string val = threadContext[key];
            if (val == null)
            {
                val = "";
            }
            return val;
        }

        public override string Put(string key, string value)
        {
            if (key == null)
            {
                key = "";
            }
            if (value == null)
            {
                value = "";
            }

            Dictionary<string, string> threadContext;
            lock (this)
            {
                if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
                {
                    threadContext = new Dictionary<string, string>();
                    _map.Add(Thread.CurrentThread, threadContext);
                }
            }

            if (!threadContext.TryGetValue(key, out string oldVal))
            {
                oldVal = "";
            }

            threadContext[key] = value;
            return oldVal;
        }

        public override string Remove(string key)
        {
            if (key == null)
            {
                key = "";
            }

            Dictionary<string, string> threadContext;
            lock (this)
            {
                if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
                {
                    return "";
                }
            }

            if (!threadContext.TryGetValue(key, out string val))
            {
                val = "";
            }
            else
            {
                threadContext.Remove(key);
            }
            return val;
        }

        public override void Write(Dictionary<string, string> prxContext, OutputStream ostr)
        {
            Dictionary<string, string> threadContext;
            lock (this)
            {
                _map.TryGetValue(Thread.CurrentThread, out threadContext);
            }

            if (threadContext == null || threadContext.Count == 0)
            {
                Ice.ContextHelper.Write(ostr, prxContext);
            }
            else if (prxContext.Count == 0)
            {
                Ice.ContextHelper.Write(ostr, threadContext);
            }
            else
            {
                var combined = new Dictionary<string, string>(prxContext);
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
                Ice.ContextHelper.Write(ostr, combined);
            }
        }

        internal override Dictionary<string, string> Combine(Dictionary<string, string> prxContext)
        {
            Dictionary<string, string> threadContext;
            lock (this)
            {
                if (!_map.TryGetValue(Thread.CurrentThread, out threadContext))
                {
                    return new Dictionary<string, string>(prxContext);
                }
            }

            var combined = new Dictionary<string, string>(prxContext);
            foreach (KeyValuePair<string, string> e in threadContext)
            {
                combined.Add(e.Key, e.Value);
            }
            return combined;
        }

        //
        //  map Thread -> Context
        //
        private readonly Dictionary<Thread, Dictionary<string, string>> _map =
            new Dictionary<Thread, Dictionary<string, string>>();
    }
}
