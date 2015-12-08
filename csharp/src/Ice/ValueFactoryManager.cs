// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;

    public sealed class ValueFactoryManager
    {
        public void add(Ice.ValueFactory factory, string id)
        {
            lock(this)
            {
                if(_factoryMap.ContainsKey(id))
                {
                    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                    ex.id = id;
                    ex.kindOfObject = "value factory";
                    throw ex;
                }
                _factoryMap[id] = factory;
            }
        }

        public void add(Ice.ObjectFactory factory, string id)
        {
            lock(this)
            {
                if(_factoryMap.ContainsKey(id))
                {
                    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                    ex.id = id;
                    ex.kindOfObject = "value factory";
                    throw ex;
                }

                Ice.ValueFactory valueFactory = s => { return factory.create(s); };
                _factoryMap[id] = valueFactory;
                _objectFactoryMap[id] = factory;
            }
        }

        public Ice.ValueFactory find(string id)
        {
            lock(this)
            {
                Ice.ValueFactory factory = null;
                _factoryMap.TryGetValue(id, out factory);
                return factory;
            }
        }

        public Ice.ObjectFactory findObjectFactory(string id)
        {
            lock(this)
            {
                Ice.ObjectFactory factory = null;
                _objectFactoryMap.TryGetValue(id, out factory);
                return factory;
            }
        }

        //
        // Only for use by Instance
        //
        internal ValueFactoryManager()
        {
            _factoryMap = new Dictionary<string, Ice.ValueFactory>();
            _objectFactoryMap = new Dictionary<string, Ice.ObjectFactory>();
        }

        internal void destroy()
        {
            Dictionary<string, Ice.ObjectFactory> oldMap = null;

            lock(this)
            {
                _factoryMap = new Dictionary<string, Ice.ValueFactory>();

                oldMap = _objectFactoryMap;
                _objectFactoryMap = new Dictionary<string, Ice.ObjectFactory>();
            }

            foreach(Ice.ObjectFactory factory in oldMap.Values)
            {
                factory.destroy();
            }
        }

        private Dictionary<string, Ice.ValueFactory> _factoryMap;
        private Dictionary<string, Ice.ObjectFactory> _objectFactoryMap;
    }

}
