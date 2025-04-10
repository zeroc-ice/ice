// Copyright (c) ZeroC, Inc.

package test.Ice.defaultServant;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.FacetNotExistException;
import com.zeroc.Ice.ObjectNotExistException;

import test.Ice.defaultServant.Test.MyObject;

public final class MyObjectI implements MyObject {
    @Override
    public void ice_ping(Current current) {
        String name = current.id.name;

        if ("ObjectNotExist".equals(name)) {
            throw new ObjectNotExistException();
        } else if ("FacetNotExist".equals(name)) {
            throw new FacetNotExistException();
        }
    }

    @Override
    public String getName(Current current) {
        String name = current.id.name;

        if ("ObjectNotExist".equals(name)) {
            throw new ObjectNotExistException();
        } else if ("FacetNotExist".equals(name)) {
            throw new FacetNotExistException();
        }

        return name;
    }
}
