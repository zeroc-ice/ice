// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

import test.Ice.classLoader.Test.ConcreteClass;
import test.Ice.classLoader.Test.E;
import test.Ice.classLoader.Test.Initial;

public final class InitialI implements Initial
{
    public InitialI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    @Override
    public ConcreteClass getConcreteClass(com.zeroc.Ice.Current current)
    {
        return new ConcreteClass();
    }

    @Override
    public void throwException(com.zeroc.Ice.Current current)
        throws E
    {
        throw new E();
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private com.zeroc.Ice.ObjectAdapter _adapter;
}
