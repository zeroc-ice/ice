// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI;

import com.zeroc.IceGrid.*;

class AdminRouter implements com.zeroc.Ice.Blobject {
    @Override
    public com.zeroc.Ice.Object.Ice_invokeResult ice_invoke(
            byte[] inParams, com.zeroc.Ice.Current current) {
        if (_admin == null) {
            throw new com.zeroc.Ice.ObjectNotExistException();
        } else if ("ice_id".equals(current.operation) ||
                "ice_ids".equals(current.operation) ||
                "ice_isA".equals(current.operation) ||
                "ice_ping".equals(current.operation) ||
                "getDefaultApplicationDescriptor".equals(current.operation)) {
            return _admin.ice_invoke(current.operation, current.mode, inParams, current.ctx);
        } else {
            // Routing other operations could be a security risk
            throw new com.zeroc.Ice.OperationNotExistException();
        }
    }

    AdminRouter(AdminPrx admin) {
        _admin = admin;
    }

    private final AdminPrx _admin;
}
