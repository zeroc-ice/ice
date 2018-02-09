// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Threading;

using IceInternal;

namespace Ice
{
    sealed class CommunicatorI : Communicator
    {
        public void destroy()
        {
            _instance.destroy();
        }

        public void shutdown()
        {
            try
            {
                _instance.objectAdapterFactory().shutdown();
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                // Ignore
            }
        }

        public void waitForShutdown()
        {
            try
            {
                _instance.objectAdapterFactory().waitForShutdown();
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                // Ignore
            }
        }

        public bool isShutdown()
        {
            try
            {
                return _instance.objectAdapterFactory().isShutdown();
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                return true;
            }
        }

        public ObjectPrx stringToProxy(string s)
        {
            return _instance.proxyFactory().stringToProxy(s);
        }

        public string proxyToString(ObjectPrx proxy)
        {
            return _instance.proxyFactory().proxyToString(proxy);
        }

        public ObjectPrx propertyToProxy(string s)
        {
            return _instance.proxyFactory().propertyToProxy(s);
        }

        public Dictionary<string, string> proxyToProperty(ObjectPrx proxy, string prefix)
        {
            return _instance.proxyFactory().proxyToProperty(proxy, prefix);
        }

        public Identity stringToIdentity(string s)
        {
            return Util.stringToIdentity(s);
        }

        public string identityToString(Identity ident)
        {
            return Util.identityToString(ident, _instance.toStringMode());
        }

        public ObjectAdapter createObjectAdapter(string name)
        {
            return _instance.objectAdapterFactory().createObjectAdapter(name, null);
        }

        public ObjectAdapter createObjectAdapterWithEndpoints(string name, string endpoints)
        {
            if(name.Length == 0)
            {
                name = Guid.NewGuid().ToString();
            }

            getProperties().setProperty(name + ".Endpoints", endpoints);
            return _instance.objectAdapterFactory().createObjectAdapter(name, null);
        }

        public ObjectAdapter createObjectAdapterWithRouter(string name, RouterPrx router)
        {
            if(name.Length == 0)
            {
                name = Guid.NewGuid().ToString();
            }

            //
            // We set the proxy properties here, although we still use the proxy supplied.
            //
            Dictionary<string, string> properties = proxyToProperty(router, name + ".Router");
            foreach(KeyValuePair<string, string> entry in properties)
            {
                getProperties().setProperty(entry.Key, entry.Value);
            }

            return _instance.objectAdapterFactory().createObjectAdapter(name, router);
        }

        public void addObjectFactory(ObjectFactory factory, string id)
        {
            _instance.addObjectFactory(factory, id);
        }

        public ObjectFactory findObjectFactory(string id)
        {
            return _instance.findObjectFactory(id);
        }

        public ValueFactoryManager getValueFactoryManager()
        {
            return _instance.initializationData().valueFactoryManager;
        }

        public Properties getProperties()
        {
            return _instance.initializationData().properties;
        }

        public Logger getLogger()
        {
            return _instance.initializationData().logger;
        }

        public Instrumentation.CommunicatorObserver getObserver()
        {
            return _instance.initializationData().observer;
        }

        public RouterPrx getDefaultRouter()
        {
            return _instance.referenceFactory().getDefaultRouter();
        }

        public void setDefaultRouter(RouterPrx router)
        {
            _instance.setDefaultRouter(router);
        }

        public LocatorPrx getDefaultLocator()
        {
            return _instance.referenceFactory().getDefaultLocator();
        }

        public void setDefaultLocator(LocatorPrx locator)
        {
            _instance.setDefaultLocator(locator);
        }

        public ImplicitContext getImplicitContext()
        {
            return _instance.getImplicitContext();
        }

        public PluginManager getPluginManager()
        {
            return _instance.pluginManager();
        }

        public void flushBatchRequests(Ice.CompressBatch compressBatch)
        {
            try
            {
                var completed = new FlushBatchTaskCompletionCallback();
                var outgoing = new CommunicatorFlushBatchAsync(_instance, completed);
                outgoing.invoke(_flushBatchRequests_name, compressBatch, true);
                completed.Task.Wait();
            }
            catch(AggregateException ex)
            {
                throw ex.InnerException;
            }
        }

        public Task flushBatchRequestsAsync(Ice.CompressBatch compressBatch,
                                            IProgress<bool> progress = null,
                                            CancellationToken cancel = new CancellationToken())
        {
            var completed = new FlushBatchTaskCompletionCallback(progress, cancel);
            var outgoing = new CommunicatorFlushBatchAsync(_instance, completed);
            outgoing.invoke(_flushBatchRequests_name, compressBatch, false);
            return completed.Task;
        }

        public AsyncResult begin_flushBatchRequests(Ice.CompressBatch compressBatch)
        {
            return begin_flushBatchRequests(compressBatch, null, null);
        }

        private const string _flushBatchRequests_name = "flushBatchRequests";

        private class CommunicatorFlushBatchCompletionCallback : AsyncResultCompletionCallback
        {
            public CommunicatorFlushBatchCompletionCallback(Communicator communicator,
                                                            Instance instance,
                                                            string op,
                                                            object cookie,
                                                            AsyncCallback callback)
                : base(communicator, instance, op, cookie, callback)
            {
            }

            protected override AsyncCallback getCompletedCallback()
            {
                return (AsyncResult result) =>
                {
                    try
                    {
                        result.throwLocalException();
                    }
                    catch(Exception ex)
                    {
                        if(exceptionCallback_ != null)
                        {
                            exceptionCallback_.Invoke(ex);
                        }
                    }
                };
            }
        };

        public AsyncResult begin_flushBatchRequests(Ice.CompressBatch compressBatch, AsyncCallback cb, object cookie)
        {
            var result = new CommunicatorFlushBatchCompletionCallback(this,
                                                                      _instance,
                                                                      _flushBatchRequests_name,
                                                                      cookie,
                                                                      cb);
            var outgoing = new CommunicatorFlushBatchAsync(_instance, result);
            outgoing.invoke(_flushBatchRequests_name, compressBatch, false);
            return result;
        }

        public void end_flushBatchRequests(AsyncResult result)
        {
            if(result != null && result.getCommunicator() != this)
            {
                const string msg = "Communicator for call to end_" + _flushBatchRequests_name +
                                   " does not match communicator that was used to call corresponding begin_" +
                                   _flushBatchRequests_name + " method";
                throw new ArgumentException(msg);
            }
            AsyncResultI.check(result, _flushBatchRequests_name).wait();
        }

        public ObjectPrx createAdmin(ObjectAdapter adminAdapter, Identity adminIdentity)
        {
            return _instance.createAdmin(adminAdapter, adminIdentity);
        }

        public ObjectPrx getAdmin()
        {
            return _instance.getAdmin();
        }

        public void addAdminFacet(Object servant, string facet)
        {
            _instance.addAdminFacet(servant, facet);
        }

        public Object removeAdminFacet(string facet)
        {
            return _instance.removeAdminFacet(facet);
        }

        public Object findAdminFacet(string facet)
        {
            return _instance.findAdminFacet(facet);
        }

        public Dictionary<string, Object> findAllAdminFacets()
        {
            return _instance.findAllAdminFacets();
        }

        public void Dispose()
        {
            destroy();
        }

        internal CommunicatorI(InitializationData initData)
        {
            _instance = new Instance(this, initData);
        }

        /*
        ~CommunicatorI()
        {
            if(!destroyed_)
            {
                if(!System.Environment.HasShutdownStarted)
                {
                    _instance.initializationData().logger.warning(
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
                _instance.finishSetup(ref args, this);
            }
            catch(System.Exception)
            {
                _instance.destroy();
                throw;
            }
        }

        //
        // For use by Util.getInstance()
        //
        internal Instance getInstance()
        {
            return _instance;
        }

        private Instance _instance;
    }
}
