// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;

    public sealed class ObjectAdapterI : ObjectAdapter
    {
        public string getName()
        {
            //
            // No mutex lock necessary, _name is immutable.
            //
            return _noConfig ? "" : _name;
        }

        public Communicator getCommunicator()
        {
            return _communicator;
        }

        public void activate()
        {
            IceInternal.LocatorInfo locatorInfo = null;
            bool registerProcess = false;
            bool printAdapterReady = false;

            _m.Lock();
            try
            {
                checkForDeactivation();
                
                //
                // If some threads are waiting on waitForHold(), we set this
                // flag to ensure the threads will start again the wait for
                // all the incoming connection factories.
                //
                _waitForHoldRetry = _waitForHold > 0;

                //
                // If the one off initializations of the adapter are already
                // done, we just need to activate the incoming connection
                // factories and we're done.
                //
                if(_activateOneOffDone)
                {
                    foreach(IceInternal.IncomingConnectionFactory icf in _incomingConnectionFactories)
                    {
                        icf.activate();
                    }
                    return;
                }

                //
                // One off initializations of the adapter: update the locator
                // registry and print the "adapter ready" message. We set the
                // _waitForActivate flag to prevent deactivation from other
                // threads while these one off initializations are done.
                //
                _waitForActivate = true;
                
                locatorInfo = _locatorInfo;
                if(!_noConfig)
                {
                    Properties properties = instance_.initializationData().properties;
                    registerProcess = properties.getPropertyAsInt(_name + ".RegisterProcess") > 0;
                    printAdapterReady = properties.getPropertyAsInt("Ice.PrintAdapterReady") > 0;
                }
            }
            finally
            {
                _m.Unlock();
            }
            
            try
            {
                Ice.Identity dummy = new Ice.Identity();
                dummy.name = "dummy";
                updateLocatorRegistry(locatorInfo, createDirectProxy(dummy), registerProcess);
            }
            catch(Ice.LocalException)
            {
                //
                // If we couldn't update the locator registry, we let the
                // exception go through and don't activate the adapter to
                // allow to user code to retry activating the adapter
                // later.
                //
                _m.Lock();
                try
                {
                    _waitForActivate = false;
                    _m.NotifyAll();
                }
                finally
                {
                    _m.Unlock();
                }
                throw;
            }
                
            if(printAdapterReady)
            {
                System.Console.Out.WriteLine(_name + " ready");
            }

            _m.Lock();
            try
            {
                Debug.Assert(!_deactivated); // Not possible if _waitForActivate = true;
            
                //
                // Signal threads waiting for the activation.
                //
                _waitForActivate = false;
                _m.NotifyAll();

                _activateOneOffDone = true;
            
                foreach(IceInternal.IncomingConnectionFactory icf in _incomingConnectionFactories)
                {
                    icf.activate();
                }
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public void hold()
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                
                foreach(IceInternal.IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    factory.hold();
                }
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public void waitForHold()
        {
            while(true)
            {
                List<IceInternal.IncomingConnectionFactory> incomingConnectionFactories;
                _m.Lock();
                try
                {
                    checkForDeactivation();
                    
                    incomingConnectionFactories =
                        new List<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);
                    
                    ++_waitForHold;
                }
                finally
                {
                    _m.Unlock();
                }

                foreach(IceInternal.IncomingConnectionFactory factory in incomingConnectionFactories)
                {
                    factory.waitUntilHolding();
                }
                
                _m.Lock();
                try
                {
                    if(--_waitForHold == 0)
                    {
                        _m.NotifyAll();
                    }
            
                    //
                    // If we don't need to retry, we're done. Otherwise, we wait until 
                    // all the waiters finish waiting on the connections and we try 
                    // again waiting on all the conncetions. This is necessary in the 
                    // case activate() is called by another thread while waitForHold()
                    // waits on the some connection, if we didn't retry, waitForHold() 
                    // could return only after waiting on a subset of the connections.
                    //
                    if(!_waitForHoldRetry)
                    {
                        return;
                    }
                    else 
                    {
                        while(_waitForHold > 0)
                        {
                            checkForDeactivation();
                            _m.Wait();
                        }
                        _waitForHoldRetry = false;
                    }
                }
                finally
                {
                    _m.Unlock();
                }
            }
        }
        
        public void deactivate()
        {
            IceInternal.OutgoingConnectionFactory outgoingConnectionFactory;
            List<IceInternal.IncomingConnectionFactory> incomingConnectionFactories;
            IceInternal.LocatorInfo locatorInfo;

            _m.Lock();
            try
            {
                //
                // Ignore deactivation requests if the object adapter has
                // already been deactivated.
                //
                if(_deactivated)
                {
                    return;
                }

                //
                //
                // Wait for activation to complete. This is necessary to not 
                // get out of order locator updates.
                //
                while(_waitForActivate)
                {
                    _m.Wait();
                }

                if(_routerInfo != null)
                {
                    //
                    // Remove entry from the router manager.
                    //
                    instance_.routerManager().erase(_routerInfo.getRouter());

                    //
                    // Clear this object adapter with the router.
                    //
                    _routerInfo.setAdapter(null);
                }
                
                incomingConnectionFactories =
                    new List<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);
                outgoingConnectionFactory = instance_.outgoingConnectionFactory();
                locatorInfo = _locatorInfo;

                _deactivated = true;
                _m.NotifyAll();
            }
            finally
            {
                _m.Unlock();
            }

            try
            {
                updateLocatorRegistry(locatorInfo, null, false);
            }
            catch(Ice.LocalException)
            {
                //
                // We can't throw exceptions in deactivate so we ignore
                // failures to update the locator registry.
                //
            }

            //
            // Must be called outside the thread synchronization, because
            // Connection::destroy() might block when sending a CloseConnection
            // message.
            //
            foreach(IceInternal.IncomingConnectionFactory factory in incomingConnectionFactories)
            {
                factory.destroy();
            }

            //
            // Must be called outside the thread synchronization, because
            // changing the object adapter might block if there are still
            // requests being dispatched.
            //
            outgoingConnectionFactory.removeAdapter(this);
        }
        
        public void waitForDeactivate()
        {
            IceInternal.IncomingConnectionFactory[] incomingConnectionFactories = null;
            _m.Lock();
            try
            {
                if(_destroyed)
                {
                    return;
                }

                //
                // Wait for deactivation of the adapter itself, and
                // for the return of all direct method calls using this
                // adapter.
                //
                while(!_deactivated || _directCount > 0)
                {
                    _m.Wait();
                }
                
                incomingConnectionFactories = _incomingConnectionFactories.ToArray();
            }
            finally
            {
                _m.Unlock();
            }
            
            //
            // Now we wait for until all incoming connection factories are
            // finished.
            //
            foreach(IceInternal.IncomingConnectionFactory factory in incomingConnectionFactories)
            {
                factory.waitUntilFinished();
            }
        }

        public bool isDeactivated()
        {
            _m.Lock();
            try
            {
                return _deactivated;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void destroy()
        {
            _m.Lock();
            try
            {
                //
                // Another thread is in the process of destroying the object
                // adapter. Wait for it to finish.
                //
                while(_destroying)
                {
                    _m.Wait();
                }

                //
                // Object adpater is already destroyed.
                //
                if(_destroyed)
                {
                    return;
                }

                _destroying = true;
            }
            finally
            {
                _m.Unlock();
            }

            //
            // Deactivate and wait for completion.
            //
            deactivate();
            waitForDeactivate();

            //
            // Now it's also time to clean up our servants and servant
            // locators.
            //
            _servantManager.destroy();
            
            //
            // Destroy the thread pool.
            //
            if(_threadPool != null)
            {
                _threadPool.destroy();
                _threadPool.joinWithAllThreads();
            }

            IceInternal.ObjectAdapterFactory objectAdapterFactory;
            
            _m.Lock();
            try
            {
                //
                // Signal that destroying is complete.
                //
                _destroying = false;
                _destroyed = true;
                _m.NotifyAll();
                
                //
                // We're done, now we can throw away all incoming connection
                // factories.
                //
                _incomingConnectionFactories.Clear();
                
                //
                // Remove object references (some of them cyclic).
                //
                instance_ = null;
                _threadPool = null;
                _routerEndpoints = null;
                _routerInfo = null;
                _publishedEndpoints = null;
                _locatorInfo = null;
                _reference = null;

                objectAdapterFactory = _objectAdapterFactory;
                _objectAdapterFactory = null;
            }
            finally
            {
                _m.Unlock();
            }

            if(objectAdapterFactory != null)
            {
                objectAdapterFactory.removeObjectAdapter(this);
            }
        }

        public ObjectPrx add(Ice.Object obj, Identity ident)
        {
            return addFacet(obj, ident, "");
        }

        public ObjectPrx addFacet(Ice.Object obj, Identity ident, string facet)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);

                //
                // Create a copy of the Identity argument, in case the caller
                // reuses it.
                //
                Identity id = new Identity();
                id.category = ident.category;
                id.name = ident.name;

                _servantManager.addServant(obj, id, facet);

                return newProxy(id, facet);
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public ObjectPrx addWithUUID(Ice.Object obj)
        {
            return addFacetWithUUID(obj, "");
        }

        public ObjectPrx addFacetWithUUID(Ice.Object obj, string facet)
        {
            Identity ident = new Identity();
            ident.category = "";
            ident.name = Guid.NewGuid().ToString();
            
            return addFacet(obj, ident, facet);
        }

        public void addDefaultServant(Ice.Object servant, string category)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();

                _servantManager.addDefaultServant(servant, category);
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public Ice.Object remove(Identity ident)
        {
            return removeFacet(ident, "");
        }

        public Ice.Object removeFacet(Identity ident, string facet)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);
                
                return _servantManager.removeServant(ident, facet);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public Dictionary<string, Ice.Object> removeAllFacets(Identity ident)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);

                return _servantManager.removeAllFacets(ident);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public Ice.Object removeDefaultServant(string category)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();

                return _servantManager.removeDefaultServant(category);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public Ice.Object find(Identity ident)
        {
            return findFacet(ident, "");
        }

        public Ice.Object findFacet(Identity ident, string facet)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);

                return _servantManager.findServant(ident, facet);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public Dictionary<string, Ice.Object> findAllFacets(Identity ident)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);

                return _servantManager.findAllFacets(ident);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public Ice.Object findByProxy(ObjectPrx proxy)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();

                IceInternal.Reference @ref = ((ObjectPrxHelperBase)proxy).reference__();
                return findFacet(@ref.getIdentity(), @ref.getFacet());
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public Ice.Object findDefaultServant(string category)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();

                return _servantManager.findDefaultServant(category);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void addServantLocator(ServantLocator locator, string prefix)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                
                _servantManager.addServantLocator(locator, prefix);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public ServantLocator removeServantLocator(string prefix)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                
                return _servantManager.removeServantLocator(prefix);
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public ServantLocator findServantLocator(string prefix)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                
                return _servantManager.findServantLocator(prefix);
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public ObjectPrx createProxy(Identity ident)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);
                
                return newProxy(ident, "");
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public ObjectPrx createDirectProxy(Identity ident)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);
                
                return newDirectProxy(ident, "");
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public ObjectPrx createIndirectProxy(Identity ident)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                checkIdentity(ident);
                
                return newIndirectProxy(ident, "", _id);
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public void setLocator(LocatorPrx locator)
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                
                _locatorInfo = instance_.locatorManager().get(locator);
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void refreshPublishedEndpoints()
        {
            IceInternal.LocatorInfo locatorInfo = null;
            bool registerProcess = false;
            List<IceInternal.EndpointI> oldPublishedEndpoints;

            _m.Lock();
            try
            {
                checkForDeactivation();

                oldPublishedEndpoints = _publishedEndpoints;
                _publishedEndpoints = parsePublishedEndpoints();

                locatorInfo = _locatorInfo;
                if(!_noConfig)
                {
                    registerProcess =
                        instance_.initializationData().properties.getPropertyAsInt(_name + ".RegisterProcess") > 0;
                }
            }
            finally
            {
                _m.Unlock();
            }

            try
            {
                Ice.Identity dummy = new Ice.Identity();
                dummy.name = "dummy";
                updateLocatorRegistry(locatorInfo, createDirectProxy(dummy), registerProcess);
            }
            catch(Ice.LocalException)
            {
                _m.Lock();
                try
                {
                    //
                    // Restore the old published endpoints.
                    //
                    _publishedEndpoints = oldPublishedEndpoints;
                    throw;
                }
                finally
                {
                    _m.Unlock();
                }
            }
        }

        public Endpoint[] getEndpoints()
        {
            _m.Lock();
            try
            {
                List<Endpoint> endpoints = new List<Endpoint>();
                foreach(IceInternal.IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    endpoints.Add(factory.endpoint());
                }
                return endpoints.ToArray();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public Endpoint[] getPublishedEndpoints()
        {
            _m.Lock();
            try
            {
                return _publishedEndpoints.ToArray();
            }
            finally
            {
                _m.Unlock();
            }
        }

        public bool isLocal(ObjectPrx proxy)
        {
            //
            // NOTE: it's important that isLocal() doesn't perform any blocking operations as 
            // it can be called for AMI invocations if the proxy has no delegate set yet.
            //

            IceInternal.Reference r = ((ObjectPrxHelperBase)proxy).reference__();
            if(r.isWellKnown())
            {
                //
                // Check the active servant map to see if the well-known
                // proxy is for a local object.
                //
                return _servantManager.hasServant(r.getIdentity());
            }
            else if(r.isIndirect())
            {
                //
                // Proxy is local if the reference adapter id matches this
                // adapter id or replica group id.
                //
                return r.getAdapterId().Equals(_id) || r.getAdapterId().Equals(_replicaGroupId);
            }
            else
            {
                IceInternal.EndpointI[] endpoints = r.getEndpoints();
            
                _m.Lock();
                try
                {
                    checkForDeactivation();

                    //
                    // Proxies which have at least one endpoint in common with the
                    // endpoints used by this object adapter's incoming connection
                    // factories are considered local.
                    //
                    for(int i = 0; i < endpoints.Length; ++i)
                    {
                        foreach(IceInternal.EndpointI endpoint in _publishedEndpoints)
                        {
                            if(endpoints[i].equivalent(endpoint))
                            {
                                return true;
                            }
                        }
                        foreach(IceInternal.IncomingConnectionFactory factory in _incomingConnectionFactories)
                        {
                            if(endpoints[i].equivalent(factory.endpoint()))
                            {
                                return true;
                            }
                        }
                    }
                    
                    //
                    // Proxies which have at least one endpoint in common with the
                    // router's server proxy endpoints (if any), are also considered
                    // local.
                    //
                    if(_routerInfo != null && _routerInfo.getRouter().Equals(proxy.ice_getRouter()))
                    {
                        for(int i = 0; i < endpoints.Length; ++i)
                        {
                            foreach(IceInternal.EndpointI endpoint in _routerEndpoints)
                            {
                                if(endpoints[i].equivalent(endpoint))
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    
                    return false;
                }
                finally
                {
                    _m.Unlock();
                }
            }
        }

        public void flushAsyncBatchRequests(IceInternal.CommunicatorBatchOutgoingAsync outAsync)
        {
            List<IceInternal.IncomingConnectionFactory> f;
            _m.Lock();
            try
            {
                f = new List<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);
            }
            finally
            {
                _m.Unlock();
            }

            foreach(IceInternal.IncomingConnectionFactory factory in f)
            {
                factory.flushAsyncBatchRequests(outAsync);
            }
        }

        public void updateConnectionObservers()
        {
            List<IceInternal.IncomingConnectionFactory> f;
            _m.Lock();
            try
            {
                f = new List<IceInternal.IncomingConnectionFactory>(_incomingConnectionFactories);
            }
            finally
            {
                _m.Unlock();
            }

            foreach(IceInternal.IncomingConnectionFactory p in f)
            {
                p.updateConnectionObservers();
            }
        }
    
        public void  updateThreadObservers()
        {
            IceInternal.ThreadPool threadPool = null;
            _m.Lock();
            try
            {
                threadPool = _threadPool;
            }
            finally
            {
                _m.Unlock();
            }

            if(threadPool != null)
            {
                threadPool.updateObservers();
            }
        }

        public void incDirectCount()
        {
            _m.Lock();
            try
            {
                checkForDeactivation();
                
                Debug.Assert(_directCount >= 0);
                ++_directCount;
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public void decDirectCount()
        {
            _m.Lock();
            try
            {
                // Not check for deactivation here!
                
                Debug.Assert(instance_ != null); // Must not be called after destroy().
                
                Debug.Assert(_directCount > 0);
                if(--_directCount == 0)
                {
                    _m.NotifyAll();
                }
            }
            finally
            {
                _m.Unlock();
            }
        }
        
        public IceInternal.ThreadPool getThreadPool()
        {
            // No mutex lock necessary, _threadPool and instance_ are
            // immutable after creation until they are removed in
            // destroy().
            
            // Not check for deactivation here!
            
            Debug.Assert(instance_ != null); // Must not be called after destroy().
            
            if(_threadPool != null)
            {
                return _threadPool;
            }
            else
            {
                return instance_.serverThreadPool(true);
            }
            
        }

        public IceInternal.ServantManager getServantManager()
        {
            //
            // No mutex lock necessary, _servantManager is immutable.
            //
            return _servantManager;
        }

        public int getACM()
        {
            // Not check for deactivation here!
            
            Debug.Assert(instance_ != null); // Must not be called after destroy().
            
            if(_hasAcmTimeout)
            {
                return _acmTimeout;
            }
            else
            {
                return instance_.serverACM();
            }
            
        }

        //
        // Only for use by IceInternal.ObjectAdapterFactory
        //
        public ObjectAdapterI(IceInternal.Instance instance, Communicator communicator,
                              IceInternal.ObjectAdapterFactory objectAdapterFactory, string name, 
                              RouterPrx router, bool noConfig)
        {
            _deactivated = false;
            instance_ = instance;
            _communicator = communicator;
            _objectAdapterFactory = objectAdapterFactory;
            _hasAcmTimeout = false;
            _acmTimeout = 0;
            _servantManager = new IceInternal.ServantManager(instance, name);
            _activateOneOffDone = false;
            _name = name;
            _incomingConnectionFactories = new List<IceInternal.IncomingConnectionFactory>();
            _publishedEndpoints = new List<IceInternal.EndpointI>();
            _routerEndpoints = new List<IceInternal.EndpointI>();
            _routerInfo = null;
            _directCount = 0;
            _waitForActivate = false;
            _waitForHold = 0;
            _waitForHoldRetry = false;
            _noConfig = noConfig;
            _processId = null;
            
            if(_noConfig)
            {
                _id = "";
                _replicaGroupId = "";
                _reference = instance_.referenceFactory().create("dummy -t", "");
                return;
            }

            Properties properties = instance_.initializationData().properties;
            List<string> unknownProps = new List<string>();
            bool noProps = filterProperties(unknownProps);

            //
            // Warn about unknown object adapter properties.
            //
            if(unknownProps.Count != 0 && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0)
            {
                StringBuilder message = new StringBuilder("found unknown properties for object adapter `");
                message.Append(_name);
                message.Append("':");
                foreach(string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                instance_.initializationData().logger.warning(message.ToString());
            }

            //
            // Make sure named adapter has configuration.
            //
            if(router == null && noProps)
            {
                //
                // These need to be set to prevent warnings/asserts in the destructor.
                //
                _deactivated = true;
                instance_ = null;
                _incomingConnectionFactories = null;

                InitializationException ex = new InitializationException();
                ex.reason = "object adapter `" + _name + "' requires configuration";
                throw ex;
            }

            _id = properties.getProperty(_name + ".AdapterId");
            _replicaGroupId = properties.getProperty(_name + ".ReplicaGroupId");

            //
            // Setup a reference to be used to get the default proxy options
            // when creating new proxies. By default, create twoway proxies.
            //
            string proxyOptions = properties.getPropertyWithDefault(_name + ".ProxyOptions", "-t");
            try
            {
                _reference = instance_.referenceFactory().create("dummy " + proxyOptions, "");
            }
            catch(ProxyParseException)
            {
                InitializationException ex = new InitializationException();
                ex.reason = "invalid proxy options `" + proxyOptions + "' for object adapter `" + _name + "'";
                throw ex;
            }

            try
            {
                int threadPoolSize = properties.getPropertyAsInt(_name + ".ThreadPool.Size");
                int threadPoolSizeMax = properties.getPropertyAsInt(_name + ".ThreadPool.SizeMax");
                if(threadPoolSize > 0 || threadPoolSizeMax > 0)
                {
                    _threadPool = new IceInternal.ThreadPool(instance_, _name + ".ThreadPool", 0);
                }

                _hasAcmTimeout = properties.getProperty(_name + ".ACM").Length > 0;
                if(_hasAcmTimeout)
                {
                    _acmTimeout = properties.getPropertyAsInt(_name + ".ACM");
                    instance_.connectionMonitor().checkIntervalForACM(_acmTimeout);
                }

                if(router == null)
                {
                    router = RouterPrxHelper.uncheckedCast(
                        instance_.proxyFactory().propertyToProxy(_name + ".Router"));
                }
                if(router != null)
                {
                    _routerInfo = instance_.routerManager().get(router);
                    if(_routerInfo != null)
                    {
                        //
                        // Make sure this router is not already registered with another adapter.
                        //
                        if(_routerInfo.getAdapter() != null)
                        {
                            Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                            ex.kindOfObject = "object adapter with router";
                            ex.id = instance_.identityToString(router.ice_getIdentity());
                            throw ex;
                        }

                        //
                        // Add the router's server proxy endpoints to this object
                        // adapter.
                        //
                        IceInternal.EndpointI[] endpoints = _routerInfo.getServerEndpoints();
                        for(int i = 0; i < endpoints.Length; ++i)
                        {
                            _routerEndpoints.Add(endpoints[i]);
                        }
                        _routerEndpoints.Sort(); // Must be sorted.

                        //
                        // Remove duplicate endpoints, so we have a list of unique endpoints.
                        //
                        for(int i = 0; i < _routerEndpoints.Count-1;)
                        {
                            IceInternal.EndpointI e1 = _routerEndpoints[i];
                            IceInternal.EndpointI e2 = _routerEndpoints[i + 1];
                            if(e1.Equals(e2))
                            {
                                _routerEndpoints.RemoveAt(i);
                            }
                            else
                            {
                                ++i;
                            }
                        }

                        //
                        // Associate this object adapter with the router. This way,
                        // new outgoing connections to the router's client proxy will
                        // use this object adapter for callbacks.
                        //
                        _routerInfo.setAdapter(this);
                    
                        //
                        // Also modify all existing outgoing connections to the
                        // router's client proxy to use this object adapter for
                        // callbacks.
                        //      
                        instance_.outgoingConnectionFactory().setRouterInfo(_routerInfo);
                    }
                }
                else
                {
                    //
                    // Parse the endpoints, but don't store them in the adapter. The connection
                    // factory might change it, for example, to fill in the real port number.
                    //
                    List<IceInternal.EndpointI> endpoints = 
                        parseEndpoints(properties.getProperty(_name + ".Endpoints"), true);
                    foreach(IceInternal.EndpointI endp in endpoints)
                    {
                        IceInternal.IncomingConnectionFactory factory =
                            new IceInternal.IncomingConnectionFactory(instance, endp, this, _name);
                        _incomingConnectionFactories.Add(factory);
                    }
                    if(endpoints.Count == 0)
                    {
                        IceInternal.TraceLevels tl = instance_.traceLevels();
                        if(tl.network >= 2)
                        {
                            instance_.initializationData().logger.trace(tl.networkCat, "created adapter `" + _name +
                                                                        "' without endpoints");
                        }
                    }
                
                    //
                    // Parse published endpoints.
                    //
                    _publishedEndpoints = parsePublishedEndpoints();
                }

                if(properties.getProperty(_name + ".Locator").Length > 0)
                {
                    setLocator(LocatorPrxHelper.uncheckedCast(
                        instance_.proxyFactory().propertyToProxy(_name + ".Locator")));
                }
                else
                {
                    setLocator(instance_.referenceFactory().getDefaultLocator());
                }
            }
            catch(LocalException)
            {
                destroy();
                throw;
            }
        }
        
        /*
        ~ObjectAdapterI()
        {
            if(!_deactivated)
            {
                string msg = "object adapter `" + getName() + "' has not been deactivated";
                if(!Environment.HasShutdownStarted)
                {
                    instance_.initializationData().logger.warning(msg);
                }
                else
                {
                    Console.Error.WriteLine(msg);
                }
            }
            else if(!_destroyed)
            {
                string msg = "object adapter `" + getName() + "' has not been destroyed";
                if(!Environment.HasShutdownStarted)
                {
                    instance_.initializationData().logger.warning(msg);
                }
                else
                {
                    Console.Error.WriteLine(msg);
                }
            }
        }
        */

        private ObjectPrx newProxy(Identity ident, string facet)
        {
            if(_id.Length == 0)
            {
                return newDirectProxy(ident, facet);
            }
            else if(_replicaGroupId.Length == 0)
            {
                return newIndirectProxy(ident, facet, _id);
            }
            else
            {
                return newIndirectProxy(ident, facet, _replicaGroupId);
            }
        }
        
        private ObjectPrx newDirectProxy(Identity ident, string facet)
        {
            IceInternal.EndpointI[] endpoints;

            // 
            // Use the published endpoints, otherwise use the endpoints from all
            // incoming connection factories.
            //
            int sz = _publishedEndpoints.Count;
            endpoints = new IceInternal.EndpointI[sz + _routerEndpoints.Count];
            for(int i = 0; i < sz; ++i)
            {
                endpoints[i] = _publishedEndpoints[i];
            }

            //
            // Now we also add the endpoints of the router's server proxy, if
            // any. This way, object references created by this object adapter
            // will also point to the router's server proxy endpoints.
            //
            for(int i = 0; i < _routerEndpoints.Count; ++i)
            {
                endpoints[sz + i] = _routerEndpoints[i];
            }
            
            //
            // Create a reference and return a proxy for this reference.
            //
            IceInternal.Reference reference = instance_.referenceFactory().create(ident, facet, _reference, endpoints);
            return instance_.proxyFactory().referenceToProxy(reference);
        }
        
        private ObjectPrx newIndirectProxy(Identity ident, string facet, string id)
        {
            //
            // Create a reference with the adapter id and return a
            // proxy for the reference.
            //
            IceInternal.Reference reference = instance_.referenceFactory().create(ident, facet, _reference, id);
            return instance_.proxyFactory().referenceToProxy(reference);
        }

        private void checkForDeactivation()
        {
            if(_deactivated)
            {
                ObjectAdapterDeactivatedException ex = new ObjectAdapterDeactivatedException();
                ex.name = getName();
                throw ex;
            }
        }
        
        private static void checkIdentity(Identity ident)
        {
            if(ident.name == null || ident.name.Length == 0)
            {
                IllegalIdentityException e = new IllegalIdentityException();
                e.id.name = ident.name;
                e.id.category = ident.category;
                throw e;
            }       
            if(ident.category == null)
            {
                ident.category = "";
            }
        }

        private List<IceInternal.EndpointI> parseEndpoints(string endpts, bool oaEndpoints)
        {
            int beg;
            int end = 0;

            string delim = " \t\n\r";

            List<IceInternal.EndpointI> endpoints = new List<IceInternal.EndpointI>();
            while(end < endpts.Length)
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(endpts, delim, end);
                if(beg == -1)
                {
                    break;
                }

                end = beg;
                while(true)
                {
                    end = endpts.IndexOf((System.Char) ':', end);
                    if(end == -1)
                    {
                        end = endpts.Length;
                        break;
                    }
                    else
                    {
                        bool quoted = false;
                        int quote = beg;
                        while(true)
                        {
                            quote = endpts.IndexOf((System.Char) '\"', quote);
                            if(quote == -1 || end < quote)
                            {
                                break;
                            }
                            else
                            {
                                quote = endpts.IndexOf((System.Char) '\"', ++quote);
                                if(quote == -1)
                                {
                                    break;
                                }
                                else if(end < quote)
                                {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if(!quoted)
                        {
                            break;
                        }
                        ++end;
                    }
                }

                if(end == beg)
                {
                    ++end;
                    continue;
                }

                string s = endpts.Substring(beg, (end) - (beg));
                IceInternal.EndpointI endp = instance_.endpointFactoryManager().create(s, oaEndpoints);
                if(endp == null)
                {
#if COMPACT
                    if(s.StartsWith("ssl", StringComparison.Ordinal))
                    {
                        instance_.initializationData().logger.warning(
                            "SSL endpoint `" + s +
                            "' ignored: IceSSL is not supported with the .NET Compact Framework");
                        ++end;
                        continue;
                    }
#else
                    if(IceInternal.AssemblyUtil.runtime_ == IceInternal.AssemblyUtil.Runtime.Mono &&
                       s.StartsWith("ssl", StringComparison.Ordinal))
                    {
                        instance_.initializationData().logger.warning(
                            "SSL endpoint `" + s + "' ignored: IceSSL is not supported with Mono");
                        ++end;
                        continue;
                    }
#endif
                    Ice.EndpointParseException e2 = new Ice.EndpointParseException();
                    e2.str = "invalid object adapter endpoint `" + s + "'";
                    throw e2;
                }
                endpoints.Add(endp);

                ++end;
            }

            return endpoints;
        }

        private List<IceInternal.EndpointI> parsePublishedEndpoints()
        {
            //
            // Parse published endpoints. If set, these are used in proxies
            // instead of the connection factory endpoints.
            //
            string endpts = instance_.initializationData().properties.getProperty(_name + ".PublishedEndpoints");
            List<IceInternal.EndpointI> endpoints = parseEndpoints(endpts, false);
            if(endpoints.Count == 0)
            {
                //
                // If the PublishedEndpoints property isn't set, we compute the published enpdoints
                // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
                // to include actual addresses in the published endpoints.
                //
                foreach(IceInternal.IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    endpoints.AddRange(factory.endpoint().expand());
                }
            }

            if(instance_.traceLevels().network >= 1)
            {
                 StringBuilder s = new StringBuilder("published endpoints for object adapter `");
                 s.Append(_name);
                 s.Append("':\n");
                 bool first = true;
                 foreach(IceInternal.EndpointI endpoint in endpoints)
                 {
                     if(!first)
                     {
                         s.Append(":");
                     }
                     s.Append(endpoint.ToString());
                     first = false;
                 }
                 instance_.initializationData().logger.trace(instance_.traceLevels().networkCat, s.ToString());
             }
             return endpoints;
        }

        private void updateLocatorRegistry(IceInternal.LocatorInfo locatorInfo, ObjectPrx proxy, bool registerProcess)
        {
            if(!registerProcess && _id.Length == 0)
            {
                return; // Nothing to update.
            }

            //
            // Call on the locator registry outside the synchronization to 
            // blocking other threads that need to lock this OA.
            //
            LocatorRegistryPrx locatorRegistry = locatorInfo != null ? locatorInfo.getLocatorRegistry() : null;
            string serverId = "";
            if(registerProcess)
            {
                Debug.Assert(instance_ != null);
                serverId = instance_.initializationData().properties.getProperty("Ice.ServerId");

                if(locatorRegistry == null)
                {
                    instance_.initializationData().logger.warning(
                        "object adapter `" + getName() + "' cannot register the process without a locator registry");
                }
                else if(serverId.Length == 0)
                {
                    instance_.initializationData().logger.warning(
                        "object adapter `" + getName() + 
                        "' cannot register the process without a value for Ice.ServerId");
                }
            }

            if(locatorRegistry == null)
            {
                return;
            }

            if(_id.Length > 0)
            {
                try
                {
                    if(_replicaGroupId.Length == 0)
                    {
                        locatorRegistry.setAdapterDirectProxy(_id, proxy);
                    }
                    else
                    {
                        locatorRegistry.setReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
                    }
                }
                catch(AdapterNotFoundException)
                {
                    if(instance_.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                        s.Append("the object adapter is not known to the locator registry");
                        instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                    }

                    NotRegisteredException ex1 = new NotRegisteredException();
                    ex1.kindOfObject = "object adapter";
                    ex1.id = _id;
                    throw ex1;
                }
                catch(InvalidReplicaGroupIdException)
                {
                    if(instance_.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                        s.Append("the replica group `" + _replicaGroupId + "' is not known to the locator registry");
                        instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                    }
                    
                    NotRegisteredException ex1 = new NotRegisteredException();
                    ex1.kindOfObject = "replica group";
                    ex1.id = _replicaGroupId;
                    throw ex1;
                }
                catch(AdapterAlreadyActiveException)
                {
                    if(instance_.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                        s.Append("the object adapter endpoints are already set");
                        instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                    }

                    ObjectAdapterIdInUseException ex1 = new ObjectAdapterIdInUseException();
                    ex1.id = _id;
                    throw;
                }
                catch(LocalException e)
                {
                    if(instance_.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't update object adapter `" + _id + "' endpoints with the locator registry:\n");
                        s.Append(e.ToString());
                        instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                    }
                    throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
                }

                if(instance_.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("updated object adapter `" + _id + "' endpoints with the locator registry\n");
                    s.Append("endpoints = ");
                    if(proxy != null)
                    {
                        Ice.Endpoint[] endpoints = proxy.ice_getEndpoints();
                        for(int i = 0; i < endpoints.Length; i++)
                        {
                            s.Append(endpoints[i].ToString());
                            if(i + 1 < endpoints.Length)
                            {
                                s.Append(":");
                            }
                        }
                    }
                    instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                }
            }
        
            if(registerProcess && serverId.Length > 0)
            {
                _m.Lock();
                try
                {
                    if(_processId == null)
                    {
                        Process servant = new IceInternal.ProcessI(_communicator);
                        _processId = addWithUUID(servant).ice_getIdentity();
                    }
                }
                finally
                {
                    _m.Unlock();
                }

                try
                {
                    locatorRegistry.setServerProcessProxy(serverId,
                        ProcessPrxHelper.uncheckedCast(createDirectProxy(_processId)));
                }
                catch(ServerNotFoundException)
                {
                    if(instance_.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n");
                        s.Append("the server is not known to the locator registry");
                        instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                    }

                    NotRegisteredException ex1 = new NotRegisteredException();
                    ex1.id = serverId;
                    ex1.kindOfObject = "server";
                    throw ex1;
                }
                catch(LocalException ex)
                {
                    if(instance_.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("couldn't register server `" + serverId + "' with the locator registry:\n" + ex);
                        instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                    }
                    throw; // TODO: Shall we raise a special exception instead of a non obvious local exception?
                }
            
                if(instance_.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("registered server `" + serverId + "' with the locator registry");
                    instance_.initializationData().logger.trace(instance_.traceLevels().locationCat, s.ToString());
                }
            }
        }

        static private readonly string[] _suffixes = 
        {
            "ACM",
            "AdapterId",
            "Endpoints",
            "Locator",
            "Locator.EncodingVersion",
            "Locator.EndpointSelection",
            "Locator.ConnectionCached",
            "Locator.PreferSecure",
            "Locator.CollocationOptimized",
            "Locator.Router",
            "PublishedEndpoints",
            "RegisterProcess",
            "ReplicaGroupId",
            "Router",
            "Router.EncodingVersion",
            "Router.EndpointSelection",
            "Router.ConnectionCached",
            "Router.PreferSecure",
            "Router.CollocationOptimized",
            "Router.Locator",
            "Router.Locator.EndpointSelection",
            "Router.Locator.ConnectionCached",
            "Router.Locator.PreferSecure",
            "Router.Locator.CollocationOptimized",
            "Router.Locator.LocatorCacheTimeout",
            "Router.LocatorCacheTimeout",
            "ProxyOptions",
            "ThreadPool.Size",
            "ThreadPool.SizeMax",
            "ThreadPool.SizeWarn",
            "ThreadPool.StackSize",
            "ThreadPool.Serialize"
        };
            
        private bool filterProperties(List<string> unknownProps)
        {
            //
            // Do not create unknown properties list if Ice prefix, ie Ice, Glacier2, etc
            //
            bool addUnknown = true;
            String prefix = _name + ".";
            for(int i = 0; IceInternal.PropertyNames.clPropNames[i] != null; ++i)
            {
                if(prefix.StartsWith(IceInternal.PropertyNames.clPropNames[i] + ".", StringComparison.Ordinal))
                {
                    addUnknown = false;
                    break;
                }
            }

            bool noProps = true;
            Dictionary<string, string> props =
                instance_.initializationData().properties.getPropertiesForPrefix(prefix);
            foreach(String prop in props.Keys)
            {
                bool valid = false;
                for(int i = 0; i < _suffixes.Length; ++i)
                {
                    if(prop.Equals(prefix + _suffixes[i]))
                    {
                        noProps = false;
                        valid = true;
                        break;
                    }
                }

                if(!valid && addUnknown)
                {
                    unknownProps.Add(prop);
                }
            }

            return noProps;
        }
        
        private bool _deactivated;
        private IceInternal.Instance instance_;
        private Communicator _communicator;
        private IceInternal.ObjectAdapterFactory _objectAdapterFactory;
        private IceInternal.ThreadPool _threadPool;
        private bool _hasAcmTimeout;
        private int _acmTimeout;
        private IceInternal.ServantManager _servantManager;
        private bool _activateOneOffDone;
        private readonly string _name;
        private readonly string _id;
        private readonly string _replicaGroupId;
        private IceInternal.Reference _reference;
        private List<IceInternal.IncomingConnectionFactory> _incomingConnectionFactories;
        private List<IceInternal.EndpointI> _routerEndpoints;
        private IceInternal.RouterInfo _routerInfo;
        private List<IceInternal.EndpointI> _publishedEndpoints;
        private IceInternal.LocatorInfo _locatorInfo;
        private int _directCount;
        private bool _waitForActivate;
        private int _waitForHold;
        private bool _waitForHoldRetry;
        private bool _destroying;
        private bool _destroyed;
        private bool _noConfig;
        private Identity _processId;

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }
}
