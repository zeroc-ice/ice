// Copyright (c) ZeroC, Inc.

package test.Ice.classLoader;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.classLoader.Test.ConcreteClass;
import test.Ice.classLoader.Test.E;
import test.Ice.classLoader.Test.Initial;

public final class InitialI implements Initial {
    public InitialI(ObjectAdapter adapter) {
        _adapter = adapter;
    }

    @Override
    public ConcreteClass getConcreteClass(Current current) {
        return new ConcreteClass();
    }

    @Override
    public void throwException(Current current) throws E {
        throw new E();
    }

    @Override
    public void shutdown(Current current) {
        _adapter.getCommunicator().shutdown();
    }

    private final ObjectAdapter _adapter;
}
