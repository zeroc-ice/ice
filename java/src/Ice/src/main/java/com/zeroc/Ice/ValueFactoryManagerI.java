//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

final class ValueFactoryManagerI implements ValueFactoryManager {
    public synchronized void add(ValueFactory factory, String id) {
        Object o = _factoryMap.get(id);
        if (o != null) {
            throw new AlreadyRegisteredException("value factory", id);
        }
        _factoryMap.put(id, factory);
    }

    public synchronized ValueFactory find(String id) {
        return _factoryMap.get(id);
    }

    private final java.util.Map<String, ValueFactory> _factoryMap = new java.util.HashMap<>();
}
