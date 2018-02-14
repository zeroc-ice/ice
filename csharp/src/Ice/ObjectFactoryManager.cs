// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;

    public sealed class ObjectFactoryManager
    {
        public void add(Ice.ObjectFactory factory, string id)
        {
            lock(this)
            {
                if(_factoryMap.ContainsKey(id))
                {
                    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                    ex.id = id;
                    ex.kindOfObject = "object factory";
                    throw ex;
                }
                _factoryMap[id] = factory;
            }
        }
        
        public void remove(string id)
        {
            object o = null;
            lock(this)
            {
                if(!_factoryMap.ContainsKey(id))
                {
                    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                    ex.id = id;
                    ex.kindOfObject = "object factory";
                    throw ex;
                }
                _factoryMap.Remove(id);
            }
            ((Ice.ObjectFactory)o).destroy();
        }
        
        public Ice.ObjectFactory find(string id)
        {
            lock(this)
            {
                Ice.ObjectFactory factory = null;
                _factoryMap.TryGetValue(id, out factory);
                return factory;
            }
        }
        
        //
        // Only for use by Instance
        //
        internal ObjectFactoryManager()
        {
            _factoryMap = new Dictionary<string, Ice.ObjectFactory>();
        }
        
        internal void destroy()
        {
            Dictionary<string, Ice.ObjectFactory> oldMap = null;

            lock(this)
            {
                oldMap = _factoryMap;
                _factoryMap = new Dictionary<string, Ice.ObjectFactory>();
            }

            foreach(Ice.ObjectFactory factory in oldMap.Values)
            {
                factory.destroy();
            }
        }
        
        private Dictionary<string, Ice.ObjectFactory> _factoryMap;
    }

}
