// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;

    public sealed class ObjectFactoryManager
    {
        public void add(Ice.ObjectFactory factory, string id)
        {
            lock(this)
            {
                object o = _factoryMap[id];
                if(o != null)
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
                o = _factoryMap[id];
                if(o == null)
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
                return (Ice.ObjectFactory)_factoryMap[id];
            }
        }
        
        //
        // Only for use by Instance
        //
        internal ObjectFactoryManager()
        {
            _factoryMap = new Hashtable();
        }
        
        internal void destroy()
        {
            Hashtable oldMap = null;

            lock(this)
            {
                oldMap = _factoryMap;
                _factoryMap = new Hashtable();
            }

            foreach(Ice.ObjectFactory factory in oldMap.Values)
            {
                factory.destroy();
            }
        }
        
        private Hashtable _factoryMap;
    }

}
