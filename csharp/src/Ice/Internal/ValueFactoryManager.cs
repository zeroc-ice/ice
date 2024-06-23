// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public sealed class ValueFactoryManagerI : Ice.ValueFactoryManager
{
    public void add(Ice.ValueFactory factory, string id)
    {
        lock (this)
        {
            if (_factoryMap.ContainsKey(id))
            {
                throw new AlreadyRegisteredException("value factory", id);
            }
            _factoryMap[id] = factory;
        }
    }

    public Ice.ValueFactory find(string id)
    {
        lock (this)
        {
            Ice.ValueFactory factory = null;
            _factoryMap.TryGetValue(id, out factory);
            return factory;
        }
    }

    private Dictionary<string, Ice.ValueFactory> _factoryMap = new();
}
