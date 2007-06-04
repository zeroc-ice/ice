// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    public abstract class ImplicitContextI : ImplicitContext
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
        public abstract bool containsKey(string key);    
        public abstract string get(string key);
        public abstract string put(string key, string value);
        public abstract string remove(string key);

        abstract public void write(Context prxContext, IceInternal.BasicStream os);
        abstract internal Context combine(Context prxContext);
    }
        
        
    internal class SharedImplicitContext : ImplicitContextI
    {
        public override Context getContext()
        {
            lock(this)
            {
                return (Context)_context.Clone();
            }
        }
            
        public override void setContext(Context context)
        {
            lock(this)
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
        }
            
        public override bool containsKey(string key)
        {
            lock(this)
            {
                if(key == null)
                {
                    key = "";
                }
                
                return _context.Contains(key);
            }
        }

        public override string get(string key)
        {
            lock(this)
            {
                if(key == null)
                {
                    key = "";
                }
                
                string val = _context[key];
                if(val == null)
                {
                    val = "";
                }
                return val;
            }
        }
            
            
        public override string put(string key, string value)
        {
            lock(this)
            {
                if(key == null)
                {
                    key = "";
                }
                if(value == null)
                {
                    value = "";
                }

                string oldVal = _context[key];
                if(oldVal == null)
                {
                    oldVal = "";
                }
                _context[key] = value;
                
                return oldVal;
            }
        }
            
        public override string remove(string key)
        {
            lock(this)
            {
                if(key == null)
                {
                    key = "";
                }

                string val = _context[key];

                if(val == null)
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
                    ctx = _context.Count == 0 ? prxContext :combine(prxContext); 
                }
                ContextHelper.write(os, ctx);
            }
        }

        internal override Context combine(Context prxContext)
        {
            lock(this)
            {
                Context combined = (Context)prxContext.Clone();
                combined.AddRange(_context);
                return combined;
            }
        }

        private Context _context = new Context();
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

        public override bool containsKey(string key)
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
                return false;
            }

            return threadContext.Contains(key);
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
                return "";
            }
            string val = threadContext[key];
            if(val == null)
            {
                val = "";
            }
            return val;
        }

        public override string put(string key, string value)
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
            
            string oldVal = threadContext[key];
            if(oldVal == null)
            {
                oldVal = "";
            }

            threadContext[key] = value;
            return oldVal;
        }

        public override string remove(string key)
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
                return "";
            }

            string val = threadContext[key];

            if(val == null)
            {
                val = "";
            }
            else
            {
                threadContext.Remove(key);
            }
            return val;
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


