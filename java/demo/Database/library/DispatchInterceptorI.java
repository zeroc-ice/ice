// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class DispatchInterceptorI extends Ice.DispatchInterceptor
{
    public Ice.DispatchStatus 
    dispatch(Ice.Request request)
    {
        // Allocate a new SQLRequestContext associated with this
        // request thread.
        SQLRequestContext context = new SQLRequestContext();
        try
        {
            Ice.DispatchStatus status = _servant.ice_dispatch(request);

            // An exception causes the current transaction to rollback.
            context.destroyFromDispatch(status == Ice.DispatchStatus.DispatchOK);

            return status;
        }
        catch(JDBCException ex)
        {
            // Log the error.
            Ice.Current c = request.getCurrent();
            context.error("call of `" + c.operation + "' on id `" + c.id.category + "/" + c.id.name + "' failed", ex);

            // A JDBCException causes the current transaction to
            // rollback.
            context.destroyFromDispatch(false);

            // Translate the exception to UnknownException.
            Ice.UnknownException e = new Ice.UnknownException();
            e.initCause(ex);
            throw e;
        }
        catch(RuntimeException ex)
        {
            // Any other exception causes the transaction to rollback.
            context.destroyFromDispatch(false);
            throw ex;
        }
    }

    DispatchInterceptorI(Ice.Object servant)
    {
        _servant = servant;
    }

    private Ice.Object _servant;
}
