// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.Ice.Blobject;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.OperationNotExistException;
import com.zeroc.IceGrid.*;

class AdminRouter implements Blobject {
    @Override
    public Object.Ice_invokeResult ice_invoke(
            byte[] inParams, Current current) {
        if (_admin == null) {
            throw new ObjectNotExistException();
        } else if ("ice_id".equals(current.operation)
                || "ice_ids".equals(current.operation)
                || "ice_isA".equals(current.operation)
                || "ice_ping".equals(current.operation)
                || "getDefaultApplicationDescriptor".equals(current.operation)) {
            return _admin.ice_invoke(current.operation, current.mode, inParams, current.ctx);
        } else {
            // Routing other operations could be a security risk
            throw new OperationNotExistException();
        }
    }

    AdminRouter(AdminPrx admin) {
        _admin = admin;
    }

    private final AdminPrx _admin;
}
