// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;

namespace Ice
{
        
    sealed class CommunicatorI : Communicator
    {
        public void destroy()
        {
            instance_.destroy();
        }

        public void shutdown()
        {
            instance_.objectAdapterFactory().shutdown();
        }
        
        public void waitForShutdown()
        {
            instance_.objectAdapterFactory().waitForShutdown();
        }

        public bool isShutdown()
        {
            return instance_.objectAdapterFactory().isShutdown();
        }
        
        public Ice.ObjectPrx stringToProxy(string s)
        {
            return instance_.proxyFactory().stringToProxy(s);
        }
        
        public string proxyToString(Ice.ObjectPrx proxy)
        {
            return instance_.proxyFactory().proxyToString(proxy);
        }

        public Ice.ObjectPrx propertyToProxy(string s)
        {
            return instance_.proxyFactory().propertyToProxy(s);
        }
        
        public Ice.Identity stringToIdentity(string s)
        {
            return instance_.stringToIdentity(s);
        }

        public string identityToString(Ice.Identity ident)
        {
            return instance_.identityToString(ident);
        }

        public ObjectAdapter createObjectAdapter(string name)
        {
            return instance_.objectAdapterFactory().createObjectAdapter(name, null);
        }
        
        public ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints)
        {
            if(name.Length == 0)
            {
                Ice.InitializationException ex = new Ice.InitializationException();
                ex.reason = "Cannot configure endpoints with nameless object adapter";
                throw ex;
            }

            getProperties().setProperty(name + ".Endpoints", endpoints);
            return instance_.objectAdapterFactory().createObjectAdapter(name, null);
        }
        
        public ObjectAdapter createObjectAdapterWithRouter(string name, RouterPrx router)
        {
            if(name.Length == 0)
            {
                Ice.InitializationException ex = new Ice.InitializationException();
                ex.reason = "Cannot configure router with nameless object adapter";
                throw ex;
            }

            //
            // We set the proxy properties here, although we still use the proxy supplied.
            //
            getProperties().setProperty(name + ".Router", proxyToString(router));
            if(router.ice_getLocator() != null)
            {
                ObjectPrx locator = router.ice_getLocator();
                getProperties().setProperty(name + ".Router.Locator", proxyToString(locator));
            }
            getProperties().setProperty(name + ".Router.CollocationOptimized",
                                        router.ice_isCollocationOptimized() ? "0" : "1");
            getProperties().setProperty(name + ".Router.ConnectionCached", router.ice_isConnectionCached() ? "0" : "1");
            getProperties().setProperty(name + ".Router.PreferSecure", router.ice_isPreferSecure() ? "0" : "1");
            getProperties().setProperty(name + ".Router.EndpointSelection",
                        router.ice_getEndpointSelection() == EndpointSelectionType.Random ? "Random" : "Ordered");
            getProperties().setProperty(name + ".Router.LocatorCacheTimeout", "" + router.ice_getLocatorCacheTimeout());

            return instance_.objectAdapterFactory().createObjectAdapter(name, router);
        }
        
        public void addObjectFactory(ObjectFactory factory, string id)
        {
            instance_.servantFactoryManager().add(factory, id);
        }
        
        public ObjectFactory findObjectFactory(string id)
        {
            return instance_.servantFactoryManager().find(id);
        }
        
        public Properties getProperties()
        {
            return instance_.initializationData().properties;
        }
        
        public Logger getLogger()
        {
            return instance_.initializationData().logger;
        }

        public Stats getStats()
        {
            return instance_.initializationData().stats;
        }

        public RouterPrx getDefaultRouter()
        {
            return instance_.referenceFactory().getDefaultRouter();
        }

        public void setDefaultRouter(RouterPrx router)
        {
            instance_.setDefaultRouter(router);
        }

        public LocatorPrx getDefaultLocator()
        {
            return instance_.referenceFactory().getDefaultLocator();
        }

        public void setDefaultLocator(LocatorPrx locator)
        {
            instance_.setDefaultLocator(locator);
        }
        
        public ImplicitContext getImplicitContext()
        {
            return instance_.getImplicitContext();
        }

        public PluginManager getPluginManager()
        {
            return instance_.pluginManager();
        }

        public void flushBatchRequests()
        {
            instance_.flushBatchRequests();
        }
        
        public Ice.ObjectPrx 
        getAdmin()
        {
            return instance_.getAdmin();
        }
        
        public void 
        addAdminFacet(Ice.Object servant, string facet)
        {
            instance_.addAdminFacet(servant, facet);
        }
        
        public Ice.Object 
        removeAdminFacet(string facet)
        {
            return instance_.removeAdminFacet(facet);
        }


        internal CommunicatorI(InitializationData initData)
        {
            instance_ = new IceInternal.Instance(this, initData);
        }
        
        /*
        ~CommunicatorI()
        {
            if(!destroyed_)
            {
                if(!System.Environment.HasShutdownStarted)
                {
                    instance_.initializationData().logger.warning(
                            "Ice::Communicator::destroy() has not been called");
                }
                else
                {
                    System.Console.Error.WriteLine("Ice::Communicator::destroy() has not been called");
                }
            }
        }
        */

        //
        // Certain initialization tasks need to be completed after the
        // constructor.
        //
        internal void finishSetup(ref string[] args)
        {
            try
            {
                instance_.finishSetup(ref args);
            }
            catch(System.Exception)
            {
                instance_.destroy();
                throw;
            }
        }
        
        //
        // For use by Util.getInstance()
        //
        internal IceInternal.Instance getInstance()
        {
            return instance_;
        }
        
        private IceInternal.Instance instance_;
    }

}
