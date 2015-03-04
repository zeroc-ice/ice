// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    public sealed class ObjectAdapterFactory
    {
        public void shutdown()
        {
            List<Ice.ObjectAdapterI> adapters;
            _m.Lock();
            try
            {
                //
                // Ignore shutdown requests if the object adapter factory has
                // already been shut down.
                //
                if(instance_ == null)
                {
                    return;
                }

                adapters = new List<Ice.ObjectAdapterI>(_adapters);
                
                instance_ = null;
                _communicator = null;
                
                _m.NotifyAll();
            }
            finally
            {
                _m.Unlock();
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
            _m.Lock();
            try
            {
                //
                // First we wait for the shutdown of the factory itself.
                //
                while(instance_ != null)
                {
                    _m.Wait();
                }
                
                adapters = new List<Ice.ObjectAdapterI>(_adapters);
            }
            finally
            {
                _m.Unlock();
            }

            //
            // Now we wait for deactivation of each object adapter.
            //
            foreach(Ice.ObjectAdapter adapter in adapters)
            {
                adapter.waitForDeactivate();
            }
        }

        public bool isShutdown()
        {
            _m.Lock();
            try
            {
                return instance_ == null;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void destroy()
        {
            //
            // First wait for shutdown to finish.
            //
            waitForShutdown();

            List<Ice.ObjectAdapterI> adapters;
            _m.Lock();
            try
            {
                adapters = new List<Ice.ObjectAdapterI>(_adapters);
            }
            finally
            {
                _m.Unlock();
            }

            foreach(Ice.ObjectAdapter adapter in adapters)
            {
                adapter.destroy();
            }

            _m.Lock();
            try
            {
                _adapters.Clear();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void
        updateConnectionObservers()
        {
            List<Ice.ObjectAdapterI> adapters;
            _m.Lock();
            try
            {
                adapters = new List<Ice.ObjectAdapterI>(_adapters);
            }
            finally
            {
                _m.Unlock();
            }
            
            foreach(Ice.ObjectAdapterI adapter in adapters)
            {
                adapter.updateConnectionObservers();
            }
        }
        
        public void
        updateThreadObservers()
        {
            List<Ice.ObjectAdapterI> adapters;
            _m.Lock();
            try
            {
                adapters = new List<Ice.ObjectAdapterI>(_adapters);
            }
            finally
            {
                _m.Unlock();
            }
            
            foreach(Ice.ObjectAdapterI adapter in adapters)
            {
                adapter.updateThreadObservers();
            }
        }
        
        public Ice.ObjectAdapter createObjectAdapter(string name, Ice.RouterPrx router)
        {
            _m.Lock();
            try
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
            finally
            {
                _m.Unlock();
            }
        }
        
        public Ice.ObjectAdapter findObjectAdapter(Ice.ObjectPrx proxy)
        {
            List<Ice.ObjectAdapterI> adapters;
            _m.Lock();
            try
            {
                if(instance_ == null)
                {
                    return null;
                }
                
                adapters = new List<Ice.ObjectAdapterI>(_adapters);
            }
            finally
            {
                _m.Unlock();
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
            _m.Lock();
            try
            {
                if(instance_ == null)
                {
                    return;
                }

                _adapters.Remove(adapter);
                _adapterNamesInUse.Remove(adapter.getName());
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void flushAsyncBatchRequests(CommunicatorBatchOutgoingAsync outAsync)
        {
            List<Ice.ObjectAdapterI> adapters;
            _m.Lock();
            try
            {
                adapters = new List<Ice.ObjectAdapterI>(_adapters);
            }
            finally
            {
                _m.Unlock();
            }

            foreach(Ice.ObjectAdapterI adapter in adapters)
            {
                adapter.flushAsyncBatchRequests(outAsync);
            }
        }
        
        //
        // Only for use by Instance.
        //
        internal ObjectAdapterFactory(Instance instance, Ice.Communicator communicator)
        {
            instance_ = instance;
            _communicator = communicator;
            _adapterNamesInUse = new HashSet<string>();
            _adapters = new List<Ice.ObjectAdapterI>();
        }
        
        private Instance instance_;
        private Ice.Communicator _communicator;
        private HashSet<string> _adapterNamesInUse;
        private List<Ice.ObjectAdapterI> _adapters;

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

}
