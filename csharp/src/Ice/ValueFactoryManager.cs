// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections.Generic;

    public sealed class ValueFactoryManagerI : Ice.ValueFactoryManager
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

        public Ice.ValueFactory find(string id)
        {
            lock(this)
            {
                Ice.ValueFactory factory = null;
                _factoryMap.TryGetValue(id, out factory);
                return factory;
            }
        }

        private Dictionary<string, Ice.ValueFactory> _factoryMap = new Dictionary<string, Ice.ValueFactory>();
    }

}
