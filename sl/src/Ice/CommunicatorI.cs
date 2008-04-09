// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
#if DEBUG
            lock(this)
            {
                destroyed_ = true;
            }
#endif
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

        public Dictionary<string, string> getDefaultContext()
        {
            return instance_.getDefaultContext();
        }
        
        public void setDefaultContext(Dictionary<string, string> ctx)
        {
            instance_.setDefaultContext(ctx);
        }

        public ImplicitContext getImplicitContext()
        {
            return instance_.getImplicitContext();
        }

        internal CommunicatorI(InitializationData initData)
        {
            instance_ = new IceInternal.Instance(this, initData);
        }
        
#if DEBUG
        ~CommunicatorI()
        {
            lock(this)
            {
                if(!destroyed_)
                {
                    System.Console.Error.WriteLine("Ice::Communicator::destroy() has not been called");
                }
            }
        }
#endif

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
#if DEBUG
                destroyed_ = true;
#endif
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
#if DEBUG
        private bool destroyed_ = false;
#endif
    }
}
