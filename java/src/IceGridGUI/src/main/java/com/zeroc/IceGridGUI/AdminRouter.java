//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI;

import com.zeroc.IceGrid.*;

class AdminRouter implements com.zeroc.Ice.Blobject
{
    @Override
    public com.zeroc.Ice.Object.Ice_invokeResult ice_invoke(byte[] inParams, com.zeroc.Ice.Current current)
    {
        if(_admin == null)
        {
            throw new com.zeroc.Ice.ObjectNotExistException(current.id, current.facet, current.operation);
        }
        else if(current.operation.equals("ice_id") ||
                current.operation.equals("ice_ids") ||
                current.operation.equals("ice_isA") ||
                current.operation.equals("ice_ping") ||
                current.operation.equals("getDefaultApplicationDescriptor"))
        {
            return _admin.ice_invoke(current.operation, current.mode, inParams, current.ctx);
        }
        else
        {
            //
            // Routing other operations could be a security risk
            //
            throw new com.zeroc.Ice.OperationNotExistException(current.id, current.facet, current.operation);
        }
    }

    AdminRouter(AdminPrx admin)
    {
        _admin = admin;
    }

    private final AdminPrx _admin;
}
