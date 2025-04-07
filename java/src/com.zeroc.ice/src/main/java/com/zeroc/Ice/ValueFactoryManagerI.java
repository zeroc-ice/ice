// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.HashMap;
import java.util.Map;

final class ValueFactoryManagerI implements ValueFactoryManager {
    public synchronized void add(ValueFactory factory, String id) {
        if (_factoryMap.containsKey(id)) {
            throw new AlreadyRegisteredException("value factory", id);
        }
        _factoryMap.put(id, factory);
    }

    public synchronized ValueFactory find(String id) {
        return _factoryMap.get(id);
    }

    private final Map<String, ValueFactory> _factoryMap = new HashMap<>();
}
