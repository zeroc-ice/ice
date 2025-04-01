// Copyright (c) ZeroC, Inc.

package test.Ice.defaultServant;

import test.Ice.defaultServant.Test.*;

public final class MyObjectI implements MyObject {
    @Override
    public void ice_ping(com.zeroc.Ice.Current current) {
        String name = current.id.name;

        if ("ObjectNotExist".equals(name)) {
            throw new com.zeroc.Ice.ObjectNotExistException();
        } else if ("FacetNotExist".equals(name)) {
            throw new com.zeroc.Ice.FacetNotExistException();
        }
    }

    @Override
    public String getName(com.zeroc.Ice.Current current) {
        String name = current.id.name;

        if ("ObjectNotExist".equals(name)) {
            throw new com.zeroc.Ice.ObjectNotExistException();
        } else if ("FacetNotExist".equals(name)) {
            throw new com.zeroc.Ice.FacetNotExistException();
        }

        return name;
    }
}
