// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using IceUtilInternal;

    public sealed class ObjectAdapterFactory
    {
        public void shutdown()
        {
            List<Ice.ObjectAdapterI> adapters;
            lock(this)
            {
                //
                // Ignore shutdown requests if the object adapter factory has
                // already been shut down.
                //
                if(instance_ == null)
                {
                    return;
                }

                adapters = _adapters;
                
                instance_ = null;
                _communicator = null;
                
                System.Threading.Monitor.PulseAll(this);
            }

            //
            // Deactivate outside the thread synchronization, to avoid
            // deadlocks.
            //
            foreach(Ice.ObjectAdapter adapter in adapters)
            {
                adapter.deactivate();
            }
        }
        
        public void waitForShutdown()
        {
            List<Ice.ObjectAdapterI> adapters;
            lock(this)
            {
                //
                // First we wait for the shutdown of the factory itself.
                //
                while(instance_ != null)
                {
                    System.Threading.Monitor.Wait(this);
                }
                
                //
                // If some other thread is currently shutting down, we wait
                // until this thread is finished.
                //
                while(_waitForShutdown)
                {
                    System.Threading.Monitor.Wait(this);
                }
                _waitForShutdown = true;
                adapters = _adapters;
            }

            //
            // Now we wait for deactivation of each object adapter.
            //
            if(adapters != null)
            {
                foreach(Ice.ObjectAdapter adapter in adapters)
                {
                    adapter.waitForDeactivate();
                }
            }
            
            lock(this)
            {
                //
                // Signal that waiting is complete.
                //
                _waitForShutdown = false;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public bool isShutdown()
        {
            lock(this)
            {
                return instance_ == null;
            }
        }

        public void destroy()
        {
            //
            // First wait for shutdown to finish.
            //
            waitForShutdown();

            List<Ice.ObjectAdapterI> adapters;
            lock(this)
            {
                adapters = _adapters;

                //
                // We set _adapters to null because our destructor must not
                // invoke methods on member objects.
                //
                _adapters = null;
            }

            if(adapters != null)
            {
                foreach(Ice.ObjectAdapter adapter in adapters)
                {
                    adapter.destroy();
                }
            }
        }
        
        public Ice.ObjectAdapter createObjectAdapter(string name, Ice.RouterPrx router)
        {
            lock(this)
            {
                if(instance_ == null)
                {
                    throw new Ice.ObjectAdapterDeactivatedException();
                }
                
                Ice.ObjectAdapterI adapter = null;
                if(name.Length == 0)
                {
                    string uuid = System.Guid.NewGuid().ToString();
                    adapter = new Ice.ObjectAdapterI(instance_, _communicator, this, uuid, null, true);
                }
                else
                {
                    if(_adapterNamesInUse.Contains(name))
                    {
                        Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                        ex.kindOfObject = "object adapter";
                        ex.id = name;
                        throw ex;
                    }
                    adapter = new Ice.ObjectAdapterI(instance_, _communicator, this, name, router, false);
                    _adapterNamesInUse.Add(name);
                }
                _adapters.Add(adapter);
                return adapter;
            }
        }
        
        public Ice.ObjectAdapter findObjectAdapter(Ice.ObjectPrx proxy)
        {
            List<Ice.ObjectAdapterI> adapters;
            lock(this)
            {
                if(instance_ == null)
                {
                    return null;
                }
                
                adapters = _adapters;
            }
            
            foreach(Ice.ObjectAdapterI adapter in adapters)
            {
                try
                {
                    if(adapter.isLocal(proxy))
                    {
                        return adapter;
                    }
                }
                catch(Ice.ObjectAdapterDeactivatedException)
                {
                    // Ignore.
                }
            }

            return null;
        }

        public void removeObjectAdapter(Ice.ObjectAdapterI adapter)
        {
            lock(this)
            {
                if(instance_ == null)
                {
                    return;
                }

                _adapters.Remove(adapter);
                _adapterNamesInUse.Remove(adapter.getName());
            }
        }
        
        public void flushBatchRequests()
        {
            List<Ice.ObjectAdapterI> adapters;
            lock(this)
            {
                if(_adapters == null)
                {
                    return;
                }
                
                adapters = _adapters;
            }

            foreach(Ice.ObjectAdapterI adapter in adapters)
            {
                adapter.flushBatchRequests();
            }
        }
        
        //
        // Only for use by Instance.
        //
        internal ObjectAdapterFactory(Instance instance, Ice.Communicator communicator)
        {
            instance_ = instance;
            _communicator = communicator;
            _adapterNamesInUse = new Set();
            _adapters = new List<Ice.ObjectAdapterI>();
            _waitForShutdown = false;
        }
        
        private Instance instance_;
        private Ice.Communicator _communicator;
        private Set _adapterNamesInUse;
        private List<Ice.ObjectAdapterI> _adapters;
        private bool _waitForShutdown;
    }

}
