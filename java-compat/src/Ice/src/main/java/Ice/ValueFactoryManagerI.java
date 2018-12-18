// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class ValueFactoryManagerI implements ValueFactoryManager
{
    public synchronized void add(ValueFactory factory, String id)
    {
        if(_factoryMap.containsKey(id))
        {
            throw new AlreadyRegisteredException("value factory", id);
        }

        _factoryMap.put(id, factory);
    }

    public synchronized ValueFactory find(String id)
    {
        return _factoryMap.get(id);
    }

    private java.util.HashMap<String, ValueFactory> _factoryMap = new java.util.HashMap<String, ValueFactory>();
}
